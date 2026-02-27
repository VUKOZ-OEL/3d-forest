/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#include "permafrost.h"

#include "watercycle.h"
#include "climate.h"
#include "resourceunit.h"
#include "soil.h"
#include "modelcontroller.h"


// #include <QMessageBox>

namespace Water {

// static variables
Permafrost::SParam Permafrost::par;
Permafrost::SMossParam Permafrost::mosspar;

// this global static declaration requires an exit-time destructor (clang warning)
// this is not optimal; consider moving the object somewhere else
static PermafrostLayers permafrostLayers;

Permafrost::Permafrost()
{
    mWC = nullptr;
}

Permafrost::~Permafrost()
{
    permafrostLayers.clearGrid(); // reset
}

void Permafrost::setup(WaterCycle *wc)
{
    mWC = wc;
    const XmlHelper &xml=GlobalSettings::instance()->settings();

    par.init();
    par.groundBaseDepth = xml.valueDouble("model.settings.permafrost.groundBaseDepth", 5.);
    par.lambdaSnow = xml.valueDouble("model.settings.permafrost.lambdaSnow", 0.3);
    par.lambdaOrganicLayer = xml.valueDouble("model.settings.permafrost.lambdaOrganicLayer", 0.);
    par.organicLayerDensity = xml.valueDouble("model.settings.permafrost.organicLayerDensity", 50.);

    par.maxFreezeThawPerDay = xml.valueDouble("model.settings.permafrost.maxFreezeThawPerDay", 10.);
    if (par.lambdaSnow * par.lambdaOrganicLayer == 0.)
        throw IException("Setp Permafrost: lambdaSnow or lambdaOrganicLayer is invalid (0).");

    par.onlySimulate = xml.valueBool("model.settings.permafrost.onlySimulate", false);

    mGroundBaseTemperature= xml.valueDouble("model.settings.permafrost.initialGroundTemperature", 0.);

    double ifd = xml.valueDouble("model.settings.permafrost.initialDepthFrozen", 1.);
    if (ifd < par.maxPermafrostDepth) {
        // seasonal permafrost
        mBottom = ifd;
    } else {
        mBottom = par.maxPermafrostDepth; // permanent permafrost
    }
    mTop = 0.; // we assume that the top of the soil is frozen at the beginning of the sim (1st of January)
    mTopFrozen = true;
    mFreezeBack = 0.; // we are not in "freezeback mode" (autumn)

    mSOLDepth = 0.;
    par.SOLDefaultDepth = xml.valueDouble("model.settings.permafrost.organicLayerDefaultDepth", 0.1);
    if (!mWC->mRU->soil()) {
        mSOLDepth = par.SOLDefaultDepth;
        qWarning() << "Permafrost is enabled, but soil carbon cycle is not. Running Permafrost with constant soil organic layer (permafrost.organicLayerDefaultDepth)= " << mSOLDepth;
    }

    mSoilDepth = mWC->soilDepth() / 1000.; // max soil depth (m)
    mPWP = mWC->mPermanentWiltingPoint;
    mFC = mWC->mFieldCapacity;

    mCurrentSoilFrozen = std::min(ifd, mSoilDepth);
    double fraction_frozen = mCurrentSoilFrozen / mSoilDepth;
    mCurrentWaterFrozen = mWC->currentContent() * fraction_frozen;

    if (!par.onlySimulate) {
        mWC->mContent -= mCurrentWaterFrozen;
        mWC->mSoilDepth -= mCurrentSoilFrozen * 1000.;
        mWC->mFieldCapacity = mFC * (1. - fraction_frozen);
        mWC->mPermanentWiltingPoint = mPWP * (1. - fraction_frozen);
    }

    setupThermalConductivity();

    setupMossLayer();

    // setup visual data layers
    if (!permafrostLayers.isValid()) {
        permafrostLayers.setGrid( GlobalSettings::instance()->model()->RUgrid() );
        GlobalSettings::instance()->controller()->addLayers(&permafrostLayers, "permafrost");
    }

    stats.reset();

}

void Permafrost::setFromSnapshot(double moss_biomass, double soil_temp, double depth_frozen, double water_frozen)
{
    mMossBiomass = moss_biomass;
    mGroundBaseTemperature = soil_temp;
    mCurrentSoilFrozen = depth_frozen;
    mCurrentWaterFrozen = water_frozen;
}

void Permafrost::newYear()
{
    // reset stats
    stats.reset();

    // calculate the depth of the organic layer
    Soil *s = mWC->mRU->soil();

    calculateMoss();

    if (s) {
        // the fuel layer is the sum of yL (leaves, needles, and twigs) and yR (coarse downed woody debris) pools (t / ha)
        double totalbiomass = s->youngLabile().biomass() * s->youngLabileAbovegroundFraction() + s->youngRefractory().biomass() * s->youngRefractoryAbovegroundFraction();
        // biomass t/ha = 10*kg/m2 / rho [kg/m3] = m
        mSOLDepth = totalbiomass * 0.1 / par.organicLayerDensity;
        // add moss layer
        mSOLDepth += mossLayerThickness();
    }
    // adapt temperature of deep soil
    // 10 year running average
    mGroundBaseTemperature = 0.9 * mGroundBaseTemperature + 0.1 * mWC->mRU->climate()->meanAnnualTemperature();

}

void Permafrost::run(const ClimateDay *clim_day)
{

    Permafrost::FTResult delta, delta_ground;
    if (clim_day->mean_temp() > 0.) {
        if (mFreezeBack > 0. ) {
            // first thaw the top layer that may be again frozen temporarily
            delta = calcFreezeThaw(mFreezeBack, clim_day->mean_temp(), true, true);
            mFreezeBack = delta.new_depth;
        } else {
            // thawing from above (soil above mTop is thawed)
            delta = calcFreezeThaw(mTop, clim_day->mean_temp(), false, true);
            mTop = delta.new_depth;
            if (mTop > 0.)
                mTopFrozen = false;
            if (mTop >= mBottom) {
                // the soil is fully thawed
                mBottom = 0.;
                mTop = 0.;
                mFreezeBack = 0.;
            }
        }
    }
    // do nothing if temp == 0
    if (clim_day->mean_temp() < 0.) {
        // freezing
        if (mTopFrozen) {
            // energy flows from above through the frozen soil
            delta = calcFreezeThaw(mBottom, clim_day->mean_temp(), true, true);
            mBottom = delta.new_depth;
        } else {
            // freeze back
            delta = calcFreezeThaw(mFreezeBack, clim_day->mean_temp(), true, true);
            mFreezeBack = delta.new_depth;
            if (mFreezeBack >= mTop) {
                // freeze back completed; the soil now is frozen
                // from the top down to "bottom"
                mTopFrozen = true;
                mBottom = std::max(mTop, mBottom);
                mTop = 0.;
                mFreezeBack = 0.;
            }
        }

        if (clim_day->dayOfMonth == 1 && clim_day->month==3) {
            // test for special cases
            if ( mFreezeBack < mTop && mFreezeBack > 0. ) {
                // freezeback not completed, nonetheless we reset
                mTopFrozen = true;
                mBottom = std::max(mTop, mBottom);
                mFreezeBack = 0.;
                mTop = 0.;

            }
        }
    }
    // effect of ground temperature
    if (mGroundBaseTemperature < 0.) {
        delta_ground = calcFreezeThaw(mTop, mGroundBaseTemperature, false, false);
        mTop = delta_ground.new_depth;
    }
    if (mGroundBaseTemperature > 0.) {
        delta_ground = calcFreezeThaw(mBottom, mGroundBaseTemperature, true, false);
        mBottom = delta_ground.new_depth;
    }


    // keep some variables (debug outputs)
    mResult.delta_mm = delta.delta_mm + delta_ground.delta_mm;
    mResult.delta_soil = delta.delta_soil + delta_ground.delta_soil;

    // effect of freezing/thawing on the water storage of the iLand water bucket
    if (mResult.delta_mm != 0. && mResult.delta_soil != 0. && !par.onlySimulate) {

        mWC->mContent = std::max(mWC->mContent + mResult.delta_mm, 0.);
        mCurrentWaterFrozen = std::min( std::max( mCurrentWaterFrozen - mResult.delta_mm, 0.), mFC);

        mWC->mSoilDepth = std::max( mWC->mSoilDepth + mResult.delta_soil * 1000., 0.); // change in mm
        mCurrentSoilFrozen = std::min( std::max( mCurrentSoilFrozen - mResult.delta_soil, 0.), mSoilDepth);

        double unfrozen = 1. - mCurrentSoilFrozen / mSoilDepth;
        mWC->mPermanentWiltingPoint = std::max( mPWP * unfrozen, 0.);
        mWC->mFieldCapacity = mFC * unfrozen;
        if (mWC->mFieldCapacity < 0.000001) {
            mWC->mFieldCapacity = 0.;
            mWC->mPermanentWiltingPoint = 0.;
        }
        if( mWC->mContent < 0.000001)
            mWC->mContent = 0.;
    }


    // stats (annual)
    stats.maxThawDepth = std::max(stats.maxThawDepth, mBottom==0. ? par.maxPermafrostDepth : mTop);
    stats.maxFreezeDepth = std::max(stats.maxFreezeDepth, mBottom);
    stats.maxSnowDepth = std::max(stats.maxSnowDepth, mWC->mSnowPack.snowDepth());

//    if (clim_day->month == 7 && mWC->mFieldCapacity == 0.)
//        QMessageBox::warning(0, "Permaforst havoc!", "debug", QMessageBox::Ok, QMessageBox::Cancel);
}

void Permafrost::debugData(DebugList &out)
{
    // permafrost
    out << mTop << mBottom << mFreezeBack << mResult.delta_mm << mResult.delta_soil
        <<  thermalConductivity(false) << mCurrentSoilFrozen << mCurrentWaterFrozen << mWC->mFieldCapacity;
    // moss
    out << stats.mossFLight << stats.mossFDecid;
}


void Permafrost::setupMossLayer()
{
    const XmlHelper &xml=GlobalSettings::instance()->settings();

    // state variables per RU
    mMossBiomass = xml.valueDouble("model.settings.permafrost.moss.biomass", 0.05); // kg/m2
    // paramters
    mosspar.light_k = xml.valueDouble("model.settings.permafrost.moss.light_k", 0.7);
    mosspar.light_comp = xml.valueDouble("model.settings.permafrost.moss.light_comp", 0.01);
    mosspar.light_sat = xml.valueDouble("model.settings.permafrost.moss.light_sat", 0.05);
    mosspar.respiration_q = xml.valueDouble("model.settings.permafrost.moss.respiration_q", 0.12);
    mosspar.respiration_b = xml.valueDouble("model.settings.permafrost.moss.respiration_b", 0.136);
    mosspar.CNRatio = xml.valueDouble("model.settings.permafrost.moss.CNRatio", 30.);
    mosspar.bulk_density = xml.valueDouble("model.settings.permafrost.moss.bulk_density", 50);
    mosspar.r_decomp = xml.valueDouble("model.settings.permafrost.moss.r_decomp", 0.14);
    mosspar.r_deciduous_inhibition = xml.valueDouble("model.settings.permafrost.moss.r_deciduous_inhibition", 0.45);

}

void Permafrost::calculateMoss()
{
    // See supplementary material S1 for details
    //if (mWC->mRU->id() == 58664)
    //    qDebug() << " debug debug debuk";

    // 1) Available Light

    // get leaf area index for canopy and moss layer
    double LAI_canopy = mWC->mRU->leafAreaIndex();
    double LAI_moss = mMossBiomass * mosspar.SLA;

    double light_below = exp(-mosspar.light_k * (LAI_canopy + LAI_moss));

    // f_light is a linear function with 0 at the light compensation point, and 1 at the light saturation level
    double f_light = (light_below - mosspar.light_comp) / (mosspar.light_sat - mosspar.light_comp);
    f_light = limit(f_light, 0., 1.); // clamp to interval [0,1]

    // 2) dessication (=dryout) of moss if the canopy is too open (lack of stomatal control of moss plants)
    // removed the dessication effect for now. Doesn't work as expected, and dessication could be part of a (future)
    // rework of the water cycle
    //double al = exp(-0.25 * LAI_canopy);
    //double f_dryout = 1.;

    //if (al > 0.5)
    //    f_dryout = 1.25 - al*al;

    // (2.3) Effect of deciduous litter
    // get fresh deciduous litter (t/ha)
    double fresh_dec_litter = 0.;
    if (mWC->mRU->snag())
        fresh_dec_litter = mWC->mRU->snag()->freshDeciduousFoliage() / 1000.; // from kg/ha -> t/ha

    double f_deciduous = exp(-mosspar.r_deciduous_inhibition * fresh_dec_litter);


    // (3) Total productivity
    // Assimilation (kg/m2): modifiers reduce the potential productivity of 0.3 kg/m2/yr
    double moss_assimilation = mosspar.AMax * f_light * f_deciduous; // (note: dessication was here: * f_dryout )

    // producitvity [kg / kg biomass/yr]
    double effective_assimilation = mosspar.SLA * moss_assimilation;
    // annual respiration loss (kg/m2/yr) (flux to atmosphere)
    double moss_rt = mMossBiomass * mosspar.respiration_q;
    // annual turnover (biomass to replace) (kg/m2/yr) (flux to litter)
    double moss_turnover = mMossBiomass * mosspar.respiration_b;

    // net productivty (kg/m2/yr): assimilation - respiration - turnover
    double moss_prod = effective_assimilation * mMossBiomass - moss_rt - moss_turnover;

    // (4) update moss pool and add produced biomass
    mMossBiomass += moss_prod;
    // avoid values below 0; assume a minimum biomass always remains
    mMossBiomass = std::max(mMossBiomass, cMinMossBiomass);

    // dead moss is transferred to the forest floor fine litter pool in iLand
    if (mWC->mRU->snag() && moss_turnover > 0.) {
        // scale up from m2 to stockable area
        double stockable_area = mWC->mRU->stockableArea();
        CNPool litter_input(stockable_area * moss_turnover * biomassCFraction,
                            stockable_area * moss_turnover * biomassCFraction / mosspar.CNRatio,
                            mosspar.r_decomp);
        mWC->mRU->snag()->addBiomassToSoil(CNPool(), litter_input);
    }

     // save some stats for moss
     stats.mossFLight = f_light;
     stats.mossFDecid = f_deciduous;
     // stats.mossFCanopy = f_dryout;
}

void Permafrost::setupThermalConductivity()
{
    // Calcluation of thermal conductivity based on the approach
    // of Farouki 1981 (as described in Bonan 2019)
    const XmlHelper &xml=GlobalSettings::instance()->settings();

    double pct_sand = xml.valueDouble("model.site.pctSand");
    double pct_clay = xml.valueDouble("model.site.pctClay");

    mSoilIsCoarse = pct_sand >= 50; // fine-texture soil: < 50% sand

    // (relative) volumetric water content at saturation = porosity
    double VWCsat = mWC->mTheta_sat;
    double rho_soil = 2700. * (1. - VWCsat);

    // Eq 5.27
    mKdry = (0.135 * rho_soil + 64.7) / (2700. - 0.947 * rho_soil);

    // constants for water and ice (Bonan)
    const double k_water = 0.57; // W/m/K water
    const double k_ice = 2.29; // W/m/K ice

    // Conductivity of solids (Ksol): use an equation from CLM3 (https://opensky.ucar.edu/islandora/object/technotes:393)
    // scale between 8.8 (quartz) and 2.92 (clay), Eq 10
    double k_sol = ( 8.8*pct_sand+ 2.92*pct_clay ) / (pct_sand + pct_clay);

    // Eq 8/9
    mKsat = pow( k_sol, (1.-VWCsat)) * pow( k_water, VWCsat);
    mKice = pow( k_sol, (1.-VWCsat)) * pow( k_ice, VWCsat);

    qDebug() << "Setup Permafrost: RID " << mWC->mRU->id() << QString(": VWCsat: %1, Kdry: %2, Ksat: %3, Kice: %4. (rho_soil: %5)").arg(VWCsat).arg(mKdry).arg(mKsat).arg(mKice).arg(rho_soil);
}

double Permafrost::thermalConductivity(bool from_below) const
{
    double rel_water_content;
//    static int bug_counter = 0;
//    if (mWC->fieldCapacity()>0 && mWC->fieldCapacity() < 0.0000001)
//        ++bug_counter;

    // assume full water saturation in the soil for energy flux from below
    if (!from_below && mWC->fieldCapacity() > 0.001)
        rel_water_content = limit(mWC->currentContent() / mWC->fieldCapacity(), 0.001, 1.);
    else
        rel_water_content = 1.;

    // Eq 4
    double k_e = mSoilIsCoarse ? 1.0 + 0.7 * log10(rel_water_content) : 1.0 + log10(rel_water_content);
    // Eq 4
    double k = mKdry + (mKsat - mKdry) * k_e;
    return k;
}

double Permafrost::thermalConductivityFrozen() const
{
    double rel_water_content = 1.; // assume saturation
    if (mCurrentSoilFrozen > 0.)
        rel_water_content = mCurrentWaterFrozen / mCurrentSoilFrozen * 0.001;

    // for frozen soil k_e = rel_water_content
    double k = mKdry + (mKice - mKdry) * rel_water_content;
    return k;
}

Permafrost::FTResult Permafrost::calcFreezeThaw(double at, double temp, bool lowerIceEdge, bool fromAbove)
{
    Permafrost::FTResult result;
    result.orig_depth = at;
    result.new_depth = at;

    // check for all frozen / thawed
    if (mTop == 0. && mBottom == 0. && temp>=0. )
        return result; // everything is already thawed
    if (mTop == 0. && mBottom >= par.maxPermafrostDepth && temp<=0.)
        return result; // everything is frozen

    const double cTempIce = 0.; // temperature of frozen soil

    double Rtotal; // thermal resistence [m2*K / W]

    if (fromAbove) {

        // (1) calc thermal resistance of the soil (including snow and organic layer)
        double d_snow = mWC->mSnowPack.snowDepth();
        double lambda_soil = thermalConductivity(false);

        // thermal resistance of the whole sandwich of layers [m2*K / W]
        Rtotal = d_snow / par.lambdaSnow + mSOLDepth / par.lambdaOrganicLayer + std::max(at, 0.05) / lambda_soil;

    } else {
        // energy flux from below
        double dist_to_layer = std::max(par.groundBaseDepth - at, 0.5);
        double lambda_soil = thermalConductivity(true); // unfrozen
        // todo: Question: is soil below fully saturated?
        if (temp < cTempIce)
            lambda_soil = thermalConductivityFrozen(); // frozen
        Rtotal = dist_to_layer / lambda_soil;
    }

    // energy flux I (J/s): depends on resistance and temperature difference (delta T)
    double I = 1. / Rtotal * (temp - cTempIce);

    // total energy transferred per day: I -> MJ/day
    double Einput = I * 86400./1000000.;

    // this energy now can freeze (or thaw) an amount of water at the edge of the active layer
    // Efusion: MJ/litre = MJ/mm / m2; Einput/Efusion: mm/day; a positive value indicates thawing
    double delta_mm = Einput / par.EFusion;
    // the amount of freezing/thawing is capped per day (to ensure numerical stability close to the surface)
    delta_mm = std::max( std::min(delta_mm, par.maxFreezeThawPerDay), -par.maxFreezeThawPerDay);

    //double max_water_content = mWC->fieldCapacity() / mWC->soilDepth(); // [-]

    // the water content of soil to freeze is determined by the current water content (mm/mm)
    double current_water_content =  mWC->soilDepth() > 0. ? mWC->currentContent() / mWC->soilDepth() : 0.;
    // for thawing, the water content is taken from the "frozen soil bucket"
    if ( I > 0. && mCurrentSoilFrozen > 0.)
        current_water_content = mCurrentWaterFrozen / mCurrentSoilFrozen / 1000.;

    // convert to change in units of soil (m)
    double delta_soil;
    // we use the actual iLand water content only if there is at least 10cm of unfrozen soil left, and
    // the the current position is within the depth of the iLand soil. We assume saturated conditions otherwise.
    if (current_water_content > 0. && mWC->soilDepth() > 100 && at < mSoilDepth)
        delta_soil = delta_mm / current_water_content / 1000.; // current water content
    else
        delta_soil = delta_mm / (mFC / mSoilDepth); // assume saturated soil

    double new_depth;
    if (lowerIceEdge)
        new_depth = at - delta_soil;
    else
        new_depth = at + delta_soil;

    // test against boundaries and limit
    // if further freezing cannot be realized (because all water is frozen already), no change in soil depth
    if (delta_soil == 0. && delta_mm < 0.)
        delta_mm = 0.;

    // fluxes within the depth of the effective soil depth
    if (new_depth < 0.) {
        // full thawing can not be realized
        double factor = fabs(at / delta_soil);
        delta_mm *= factor;
        delta_soil *= factor;
        new_depth = 0.;
    } else if (at > mSoilDepth && new_depth > mSoilDepth) {
        // no effect on effective soil layer (changes happening to deep below)
        delta_mm = 0.;
        delta_soil = 0.;
    } else  if ( (at <= mSoilDepth && new_depth > mSoilDepth) ||
                 (at >= mSoilDepth && new_depth < mSoilDepth)) {
        // full freezing/thawing around the lower boundary of the soil
        // can not be realized
        double factor = 1. - fabs( (new_depth - mSoilDepth) / delta_soil );
        delta_mm *= factor;
        delta_soil *= factor;
    }

    if (new_depth > par.maxPermafrostDepth) {
        // limit to max depth of permafrost - no effect on fluxes
        new_depth = par.maxPermafrostDepth;
    }


    result.delta_mm = delta_mm;
    result.delta_soil = delta_soil;
    result.new_depth = new_depth;
    return result;
}


// function used for visualizing permafrost data in iLand (i.e., by returning a value for each cell for a given data layer)
// see also names()
double PermafrostLayers::value(ResourceUnit * const &data, const int index) const
{
    if (!data || !data->waterCycle() || !data->waterCycle()->permafrost())
        return 0.;
    const Permafrost *pf = data->waterCycle()->permafrost();
    switch (index) {
    case 0: return pf->stats.maxFreezeDepth;
    case 1: return pf->stats.maxThawDepth;
    case 2: return pf->mGroundBaseTemperature;
    case 3: return pf->stats.maxSnowDepth*100.;
    case 4: return pf->mSOLDepth*100.;
    case 5: return pf->mossLayerThickness()*100.;
    default: return 0.;
    }
}

// set names and description of available data layer for iLand visualization of permafrost layers.
const QVector<LayeredGridBase::LayerElement> &PermafrostLayers::names()
{
    if (mNames.isEmpty())
        mNames= QVector<LayeredGridBase::LayerElement>()
                << LayeredGridBase::LayerElement(QStringLiteral("maxDepthFrozen"), QStringLiteral("maximum depth of freezing (m). Is 2m for full freeze."), GridViewTurbo)
                << LayeredGridBase::LayerElement(QStringLiteral("maxDepthThawed"), QStringLiteral("maximum depth of thawing (m). Is 2m for fully thawed soil"), GridViewTurbo)
                << LayeredGridBase::LayerElement(QStringLiteral("deepSoilTemperature"), QStringLiteral("temperature of ground deep below the soil (C)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("maxSnowCover"), QStringLiteral("maximum snow height (cm)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("SOLDepth"), QStringLiteral("depth of the soil organic layer (litter+dead moss) (cm)"), GridViewTurbo)
                << LayeredGridBase::LayerElement(QStringLiteral("moss"), QStringLiteral("depth of the life moss layer (cm)"), GridViewTurbo);
    return mNames;

}


} // end namespace
