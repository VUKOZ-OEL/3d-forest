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

#include "soil.h"
#include "globalsettings.h"
#include "xmlhelper.h" // for load settings
#include "exception.h"
#include "resourceunit.h"
/** @class Soil provides an implementation of the ICBM/2N soil carbon and nitrogen dynamics model.
  @ingroup core
  The ICBM/2N model was developed by Kaetterer and Andren (2001) and used by others (e.g. Xenakis et al, 2008).
  See https://iland-model.org/soil+C+and+N+cycling for a model overview and the rationale of the model choice.

  */

double Soil::mNitrogenDeposition = 0.;

// site-specific parameters
// i.e. parameters that need to be specified in the environment file
// note that leaching is not actually influencing soil dynamics but reduces availability of N to plants by assuming that some N
// (proportional to its mineralization in the mineral soil horizon) is leached
// see separate wiki-page (https://iland-model.org/soil+parametrization+and+initialization)
// and R-script on parameter estimation and initialization
static struct SoilParams {
    // ICBM/2N parameters
    SoilParams(): qb(5.), qh(25.), leaching(0.15), el(0.0577), er(0.073), is_setup(false) {}
    double qb; ///< C/N ratio of soil microbes
    double qh; ///< C/N ratio of SOM
    double leaching; ///< how many percent of the mineralized nitrogen in O is not available for plants but is leached
    double el; ///< microbal efficiency in the labile pool, auxiliary parameter (see parameterization example)
    double er; ///< microbal efficiency in the refractory pool, auxiliary parameter (see parameterization example)
    bool is_setup;
} global_soilpar;
SoilParams *Soil::mParams = &global_soilpar; // save a ptr to the single value container as a static class variable

void Soil::fetchParameters()
{
    XmlHelper xml_site(GlobalSettings::instance()->settings().node("model.site"));
    mKo = xml_site.valueDouble("somDecompRate", 0.02);
    mH =  xml_site.valueDouble("soilHumificationRate", 0.3);

    if (mParams->is_setup || !GlobalSettings::instance()->model())
        return;
    XmlHelper xml(GlobalSettings::instance()->settings().node("model.settings.soil"));
    mParams->qb =  xml.valueDouble("qb", 5.);
    mParams->qh =  xml.valueDouble("qh", 25.);
    mParams->leaching =  xml.valueDouble("leaching", 0.15);
    mParams->el =  xml.valueDouble("el", 0.0577);
    mParams->er = xml.valueDouble("er", 0.073);

    mParams->is_setup = true;

    mNitrogenDeposition = xml.valueDouble("nitrogenDeposition",0.);
}


Soil::Soil(ResourceUnit *ru)
{
    mRU = ru;
    mRE = 0.;
    mAvailableNitrogen = 0.;
    mKyl = 0.;
    mKyr = 0.;
    mH = 0.;
    mKo = 0.;
    mYLaboveground_frac = mYRaboveground_frac = 0.;
    fetchParameters();
}

// reset of bookkeeping variables
void Soil::newYear()
{
    mTotalToDisturbance.clear();
    mTotalToAtmosphere.clear();
}

/// setup initial content of the soil pool (call before model start)
void Soil::setInitialState(const CNPool &young_labile_kg_ha,
                           const CNPool &young_refractory_kg_ha,
                           const CNPair &SOM_kg_ha,
                           double young_labile_aboveground_frac,
                           double young_refractory_aboveground_frac)
{
    mYL = young_labile_kg_ha*0.001; // pool sizes are stored in t/ha
    mYR = young_refractory_kg_ha*0.001;
    mSOM = SOM_kg_ha*0.001;

    mKyl = young_labile_kg_ha.parameter();
    mKyr = young_refractory_kg_ha.parameter();

    if (mKyl<=0. || mKyr<=0.)
        throw IException(QString("setup of Soil: kyl or kyr invalid: kyl: %1 kyr: %2").arg(mKyl).arg(mKyr));
    if (!mYL.isValid())
        throw IException(QString("setup of Soil: yl-pool invalid: c: %1 n: %2").arg(mYL.C).arg(mYL.N));
    if (!mYL.isValid())
        throw IException(QString("setup of Soil: yr-pool invalid: c: %1 n: %2").arg(mYR.C).arg(mYR.N));
    if (!mYL.isValid())
        throw IException(QString("setup of Soil: som-pool invalid: c: %1 n: %2").arg(mSOM.C).arg(mSOM.N));

    mYLaboveground_frac = young_labile_aboveground_frac;
    mYRaboveground_frac = young_refractory_aboveground_frac;
}

/// set soil inputs of current year (litter and deadwood)
/// @param labile_input_kg_ha The input to the labile pool (kg/ha); this comprises of leaves, fine roots
/// @param refractory_input_kg_ha The input to the refr. pool (kg/ha); branches, stems, coarse roots
/// @param labile_aboveground_C Carbon in the labile input from aboveground sources (kg/ha)
/// @param refr_aboveground_C Carbon in the woody input from aboveground sources (kg/ha)
void Soil::setSoilInput(const CNPool &labile_input_kg_ha, const CNPool &refractory_input_kg_ha, double labile_aboveground_C, double refractory_aboveground_C)
{
    // stockable area:
    // if the stockable area is < 1ha, then
    // scale the soil inputs to a full hectare
    double area_ha = mRU?mRU->stockableArea() / cRUArea:1.;

    if (area_ha==0.) {
        qDebug() << "Soil::setSoilInput: stockable area is 0!";
        return;
        //throw IException("Soil::setSoilInput: stockable area is 0!");
    }
    // for the carbon input flow from snags/trees we assume a minimum size of the "stand" of 0.1ha
    // this reduces rapid input pulses (e.g. if one large tree dies).
    // Put differently: for resource units with stockable area < 0.1ha, we add a "blank" area.
    // the soil module always calculates per ha values, so nothing else needs to be done here.
    // area_ha = std::max(area_ha, 0.1);

    mInputLab = labile_input_kg_ha * (0.001 / area_ha); // transfer from kg/ha -> tons/ha and scale to 1 ha
    mInputRef = refractory_input_kg_ha * (0.001 / area_ha);
    // calculate the decomposition rates
    mKyl = mYL.parameter(mInputLab);
    mKyr = mYR.parameter(mInputRef);
    if (isnan(mKyr) || isnan(mYR.C))
        qDebug() << "mKyr is NAN";
    if (mKyr == 0.) {
        mKyr = 0.0001;
        qDebug() << "Soil::setSoilInput: Invalid value (0.) for dwd decomp rate (mKyr). Set to 0.0001.";
    }
    if (mKyl == 0.) {
        mKyl = 0.0001;
        qDebug() << "Soil::setSoilInput: Invalid value (0.) for litter decomp rate (mKyl). Set to 0.0001.";
    }

    // update the aboveground fraction
    // conceptually this is a weighted mean of the AG fraction of the content with the input
    mYLaboveground_frac = (mYL.C * mYLaboveground_frac + labile_aboveground_C * (0.001 / area_ha)) / (mYL.C + mInputLab.C);
    mYRaboveground_frac = (mYR.C * mYRaboveground_frac + refractory_aboveground_C * (0.001 / area_ha)) / (mYR.C + mInputRef.C);

    if (mYLaboveground_frac<0. || mYLaboveground_frac>1. || mYL.C<0. || mInputLab.C<0.) {
        qDebug() << "Soil:setSoilInput: invalid input: InputLabC:" << mInputLab.C << "YLC:" << mYL.C << "YLabovegroundFrac:" << mYLaboveground_frac << "Ru-index:" << mRU->index();
    }
    if (mYRaboveground_frac<0. || mYRaboveground_frac>1. || mYR.C<0. || mInputRef.C<0.) {
        qDebug() << "Soil:setSoilInput: invalid input: InputRefC:" << mInputRef.C << "YRC:" << mYR.C << "YRabovegroundFrac:" << mYRaboveground_frac << "Ru-index:" << mRU->index();
    }

}


/// Main calculation function
/// must be called after snag dyanmics (i.e. to ensure input fluxes are available)
/// See Appendix of Kaetterer et al 2001 for integrated equations
void Soil::calculateYear()
{
    SoilParams &sp = *mParams;
    // checks
    if (mRE==0.) {
        throw IException("Soil::calculateYear(): Invalid value for 're' (=0) for RU(index): " + QString::number(mRU->index()));
    }
    const double t = 1.; // timestep (annual)
    // auxiliary calculations
    CNPair total_before = mYL + mYR + mSOM;

    CNPair total_in = mInputLab + mInputRef;
    if (isnan(total_in.C) || isnan(mKyr))
        qDebug() << "soil input is NAN.";

    double ylss = mInputLab.C / (mKyl * mRE); // Yl stedy state C (eq A13)
    double cl = sp.el * (1. - mH)/sp.qb - mH*(1.-sp.el)/sp.qh; // eta l in the paper
    double ynlss = 0.;
    if (!mInputLab.isEmpty()) {
        ynlss = mInputLab.C / (mKyl*mRE*(1.-mH)) * ((1.-sp.el)/mInputLab.CN() + cl); // Yl steady state N
        if (ynlss < 0.)
            ynlss = 0.; // do not allow a negative value for steady state
    }

    double yrss = mInputRef.C / (mKyr * mRE); // Yr steady state C (eq A14)
    double cr = sp.er * (1. - mH)/sp.qb - mH*(1.-sp.er)/sp.qh; // eta r in the paper
    double ynrss = 0.;
    if (!mInputRef.isEmpty()) {
        ynrss = mInputRef.C / (mKyr*mRE*(1.-mH)) * ((1.-sp.er)/mInputRef.CN() + cr); // Yr steady state N
        if (ynrss <0.)
            ynrss = 0.; // do not allow negative steady state
    }

    double oss = mH*total_in.C / (mKo*mRE); // O steady state C
    double onss = mH*total_in.C / (sp.qh*mKo*mRE); // O steady state N

    double al = mH*(mKyl*mRE* mYL.C - mInputLab.C) / ((mKo-mKyl)*mRE);
    double ar = mH*(mKyr*mRE* mYR.C - mInputRef.C) / ((mKo-mKyr)*mRE);

    // update of state variables
    // precalculations
    double lfactor = exp(-mKyl*mRE*t);
    double rfactor = exp(-mKyr*mRE*t);
    // young labile pool
    CNPair yl=mYL;
    mYL.C = ylss + (yl.C-ylss)*lfactor;
    // N: see eq A18
    mYL.N = ynlss + (yl.N-ynlss-cl/(sp.el-mH)*(yl.C-ylss))*exp(-mKyl*mRE*(1.-mH)*t/(1.-sp.el)) + cl/(sp.el-mH)*(yl.C-ylss)*lfactor;
    if (mYL.N < 0.)
        mYL.N = 0.;

    mYL.setParameter( mKyl ); // update decomposition rate

    // young ref. pool
    CNPair yr=mYR;
    mYR.C = yrss + (yr.C-yrss)*rfactor;
    // N: see eq A19.
    mYR.N = ynrss + (yr.N-ynrss-cr/(sp.er-mH)*(yr.C-yrss))*exp(-mKyr*mRE*(1.-mH)*t/(1.-sp.er)) + cr/(sp.er-mH)*(yr.C-yrss)*rfactor;
    if (mYR.N < 0.) {
        mYR.N = 0.;
        //qDebug() << "YR.N <0 ";
    }
    mYR.setParameter( mKyr ); // update decomposition rate
    // SOM pool (old)
    CNPair o = mSOM;
    mSOM.C = oss + (o.C -oss - al - ar)*exp(-mKo*mRE*t) + al*lfactor + ar*rfactor;
    mSOM.N = onss + (o.N - onss -(al+ar)/sp.qh)*exp(-mKo*mRE*t) + al/sp.qh * lfactor + ar/sp.qh * rfactor;

    if (!mYL.isValid() || !mYR.isValid() || !mSOM.isValid()) {
        qDebug() << "Soil::calculateYear: invalid soil pools in yL, yR, or SOM";
    }

    // calculate delta (i.e. flux to atmosphere)
    CNPair total_after = mYL + mYR + mSOM;
    CNPair flux = total_before + total_in - total_after;
    if (flux.C < 0.) {
        qDebug() << "negative flux to atmosphere?!?";
        flux.clear();
    }
    mTotalToAtmosphere += flux;


    // calculate plant available nitrogen
    mAvailableNitrogenFromLabile = mKyl*mRE*(1.-mH)/(1.-sp.el) * (mYL.N - sp.el*mYL.C/sp.qb);  // N from labile...
    mAvailableNitrogenFromRefractory = mKyr*mRE*(1-mH)/(1.-sp.er)* (mYR.N - sp.er*mYR.C/sp.qb); // + N from refractory...
    double nav_from_som = mKo*mRE*mSOM.N*(1.-sp.leaching); // + N from SOM pool (reduced by leaching (leaching modeled only from slow SOM Pool))

    mAvailableNitrogenFromLabile *= 1000.; // t/ha -> kg/ha
    mAvailableNitrogenFromRefractory *= 1000.; // t/ha -> kg/ha
    nav_from_som *= 1000.; // t/ha -> kg/ha

    mAvailableNitrogen = mAvailableNitrogenFromLabile + mAvailableNitrogenFromRefractory + nav_from_som;

    if (mAvailableNitrogen<0.)
        mAvailableNitrogen = 0.;
    if (isnan(mAvailableNitrogen) || isnan(mYR.C))
        qDebug() << "Available Nitrogen is NAN.";

    // add nitrogen deposition
    mAvailableNitrogen += mNitrogenDeposition;

    // stedy state for n-available
    //    double navss = mKyl*mRE*(1.-mH)/(1.-sp.el)*(ynlss-sp.el*ylss/sp.qb); // available nitrogen (steady state)
    //    navss += mKyr*mRE*(1.-mH)/(1.-sp.er)*(ynrss - sp.er*yrss/sp.qb);
    //    navss += mKo*mRE*onss*(1.-sp.leaching);

}

QList<QVariant> Soil::debugList()
{
    QList<QVariant> list;
    // (1) inputs of the year
    list << mInputLab.C << mInputLab.N << mInputLab.parameter() << mInputRef.C << mInputRef.N << mInputRef.parameter() << mRE;
    // (2) states
    list << mKyl << mKyr << mYL.C << mYL.N << mYR.C << mYR.N << mSOM.C << mSOM.N;
    // (3) nav
    list << mAvailableNitrogen << mAvailableNitrogenFromLabile << mAvailableNitrogenFromRefractory << (mAvailableNitrogen-mAvailableNitrogenFromLabile-mAvailableNitrogenFromRefractory);
    return list;
}

/// remove part of the biomass (e.g.: due to fire).
/// @param DWDfrac fraction of downed woody debris (yR) to remove (0: nothing, 1: remove 100% percent)
/// @param litterFrac fraction of litter pools (yL) to remove (0: nothing, 1: remove 100% percent)
/// @param soilFrac fraction of soil pool (SOM) to remove (0: nothing, 1: remove 100% percent)
void Soil::disturbance(double DWDfrac, double litterFrac, double soilFrac)
{
    if (DWDfrac<0. || DWDfrac>1.)
        qDebug() << "warning: Soil:disturbance: DWD-fraction invalid" << DWDfrac;
    if (litterFrac<0. || litterFrac>1.)
        qDebug() << "warning: Soil:disturbance: litter-fraction invalid" << litterFrac;
    if (soilFrac<0. || soilFrac>1.)
        qDebug() << "warning: Soil:disturbance: soil-fraction invalid" << soilFrac;
    // force to 0-1
    DWDfrac = limit(DWDfrac, 0., 1.);
    litterFrac = limit(litterFrac, 0., 1.);
    soilFrac = limit(soilFrac, 0., 1.);

    // dwd
    mTotalToDisturbance += mYR*DWDfrac;
    mYR *= (1. - DWDfrac);
    // litter
    mTotalToDisturbance += mYL*litterFrac;
    mYL *= (1. - litterFrac);
    // old soil organic matter
    mTotalToDisturbance += mSOM*soilFrac;
    mSOM *= (1. - soilFrac);
    if (!mYL.isValid() || !mYR.isValid() || !mSOM.isValid())
        qDebug() << "Soil::disturbance: invalid pool (yL, yR, or SOM)";
    if (isnan(mAvailableNitrogen) || isnan(mYR.C))
        qDebug() << "Available Nitrogen is NAN.";

}

/// remove biomass from the soil layer (e.g.: due to fire).
/// @param DWD_kg_ha downed woody debris (yR) to remove kg/ha
/// @param litter_kg_ha biomass in litter pools (yL) to remove kg/ha
/// @param soil_kg_ha biomass in soil pool (SOM) to remove kg/ha
void Soil::disturbanceBiomass(double DWD_kg_ha, double litter_kg_ha, double soil_kg_ha)
{
    double frac_dwd = 0.;
    double frac_litter = 0.;
    double frac_som = 0.;
    if (!mYR.isEmpty())
        frac_dwd = DWD_kg_ha / 1000. / mYR.biomass();
    if (!mYL.isEmpty())
        frac_litter = litter_kg_ha / 1000. / mYL.biomass();
    if (!mSOM.isEmpty())
        frac_som = soil_kg_ha / 1000. / mSOM.biomass();

    if (frac_litter<0. || frac_litter>1.) {
        qDebug() << "disturbanceBiomass: frac_litter " << frac_litter << " mYL: " << mYL.biomass() << "abovegroundfrac YR:" << youngRefractoryAbovegroundFraction() << "agfrac lab:" << youngLabileAbovegroundFraction() << "RU-index:" << mRU->index();
    }
    disturbance(frac_dwd, frac_litter, frac_som);
}

double Soil::totalCarbon() const
{
    // total carbon content: yR + yL + SOM in t/ha
    return youngRefractory().C + youngLabile().C + oldOrganicMatter().C;
}








