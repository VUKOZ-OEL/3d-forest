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

#include "snag.h"
#include "tree.h"
#include "species.h"
#include "globalsettings.h"
#include "expression.h"
// for calculation of climate decomposition
#include "resourceunit.h"
#include "watercycle.h"
#include "climate.h"
#include "model.h"

/** @class Snag
  @ingroup core
  Snag deals with carbon / nitrogen fluxes from the forest until the reach soil pools.
  Snag lives on the level of the ResourceUnit; carbon fluxes from trees enter Snag, and parts of the biomass of snags
  is subsequently forwarded to the soil sub model.
  Carbon is stored in three classes (depending on the size)
  The Snag dynamics class uses the following species parameter:
  cnFoliage, cnFineroot, cnWood, snagHalflife, snagKSW

  */
// static variables
double Snag::mDBHLower = -1.;
double Snag::mDBHHigher = 0.;
double Snag::mCarbonThreshold[] = {0., 0., 0.};

double CNPair::biomassCFraction = biomassCFraction; // get global from globalsettings.h

/// add biomass and weigh the parameter_value with the current C-content of the pool
void CNPool::addBiomass(const double biomass, const double CNratio, const double parameter_value)
{
    if (biomass==0.)
        return;
    double new_c = biomass*biomassCFraction;
    double p_old = C / (new_c + C);
    mParameter = mParameter*p_old + parameter_value*(1.-p_old);
    CNPair::addBiomass(biomass, CNratio);
}

// increase pool (and weigh the value)
void CNPool::operator+=(const CNPool &s)
{
    if (s.C==0.)
        return;
    mParameter = parameter(s); // calculate weighted parameter
    C+=s.C;
    N+=s.N;
}

double CNPool::parameter(const CNPool &s) const
{
    if (s.C==0.)
        return parameter();
    double p_old = C / (s.C + C);
    double result =  mParameter*p_old + s.parameter()*(1.-p_old);
    return result;
}


void Snag::setupThresholds(const double lower, const double upper)
{
    if (mDBHLower == lower)
        return;
    mDBHLower = lower;
    mDBHHigher = upper;
    mCarbonThreshold[0] = lower / 2.;
    mCarbonThreshold[1] = lower + (upper - lower)/2.;
    mCarbonThreshold[2] = upper + (upper - lower)/2.;
    //# threshold levels for emptying out the dbh-snag-classes
    //# derived from Psme woody allometry, converted to C, with a threshold level set to 1%
    //# values in kg!
    for (int i=0;i<3;i++)
        mCarbonThreshold[i] = 0.10568*pow(mCarbonThreshold[i],2.4247)*0.5*0.01;
}


Snag::Snag()
{
    mRU = 0;
    CNPair::setCFraction(biomassCFraction);
}

void Snag::setup( const ResourceUnit *ru)
{
    mRU = ru;
    mClimateFactor = 0.;
    // branches
    mBranchCounter=0;
    for (int i=0;i<3;i++) {
        mTimeSinceDeath[i] = 0.;
        mNumberOfSnags[i] = 0.;
        mAvgDbh[i] = 0.;
        mAvgHeight[i] = 0.;
        mAvgVolume[i] = 0.;
        mKSW[i] = 0.;
        mCurrentKSW[i] = 0.;
        mHalfLife[i] = 0.;
    }
    mTotalSnagCarbon = 0.;
    mDeciduousFoliageLitter = 0.;

    if (mDBHLower<=0)
        throw IException("Snag::setupThresholds() not called or called with invalid parameters.");

    // Inital values from XML file
    XmlHelper xml=GlobalSettings::instance()->settings();
    double kyr = xml.valueDouble("model.site.youngRefractoryDecompRate", -1);
    // put carbon of snags to the middle size class
    xml.setCurrentNode("model.initialization.snags");
    mSWD[1].C = xml.valueDouble(".swdC");
    mSWD[1].N = mSWD[1].C / xml.valueDouble(".swdCN", 50.);
    mSWD[1].setParameter(kyr);
    mKSW[1] = xml.valueDouble(".swdDecompRate");
    mNumberOfSnags[1] = xml.valueDouble(".swdCount");
    mHalfLife[1] = xml.valueDouble(".swdHalfLife");
    // and for the Branch/coarse root pools: split the init value into five chunks
    CNPool other(xml.valueDouble(".otherC"), xml.valueDouble(".otherC")/xml.valueDouble(".otherCN", 50.), kyr );
    mOtherWoodAbovegroundFrac = xml.valueDouble(".otherAbovegroundFraction", 0.5);
    mTotalSnagCarbon = other.C + mSWD[1].C;

    other *= 0.2;
    for (int i=0;i<5;i++)
        mOtherWood[i] = other;
}

void Snag::scaleInitialState()
{
    double area_factor = mRU->stockableArea() / cRUArea; // fraction stockable area
    // avoid huge snag pools on very small resource units (see also soil.cpp)
    // area_factor = std::max(area_factor, 0.1);
    mSWD[1] *= area_factor;
    mNumberOfSnags[1] *= area_factor;
    for (int i=0;i<5;i++)
        mOtherWood[i]*= area_factor;
    mTotalSnagCarbon *= area_factor;

}

// debug outputs
QList<QVariant> Snag::debugList()
{
    // list columns
    // for three pools
    QList<QVariant> list;

    // totals
    list << mTotalSnagCarbon << mTotalIn.C << mTotalToAtm.C << mSWDtoSoil.C << mSWDtoSoil.N;
    // fluxes to labile soil pool and to refractory soil pool
    list << mLabileFlux.C << mLabileFlux.N << mRefractoryFlux.C << mRefractoryFlux.N;

    for (int i=0;i<3;i++) {
        // pools "swdx_c", "swdx_n", "swdx_count", "swdx_tsd", "toswdx_c", "toswdx_n"
        list << mSWD[i].C << mSWD[i].N << mNumberOfSnags[i] << mTimeSinceDeath[i] << mToSWD[i].C << mToSWD[i].N;
        list << mAvgDbh[i] << mAvgHeight[i] << mAvgVolume[i];
    }

    // branch/coarse wood pools (5 yrs)
    for (int i=0;i<5;i++) {
        list << mOtherWood[i].C << mOtherWood[i].N;
    }
    return list;
}


void Snag::newYear()
{
    for (int i=0;i<3;i++) {
        mToSWD[i].clear(); // clear transfer pools to standing-woody-debris
        mCurrentKSW[i] = 0.;
    }
    mLabileFlux.clear();
    mRefractoryFlux.clear();
    mTotalToAtm.clear();
    mTotalToExtern.clear();
    mTotalToDisturbance.clear();
    mTotalIn.clear();
    mSWDtoSoil.clear();
    mLabileFluxAbovegroundCarbon = mRefrFluxAbovegroundCarbon = 0.;

}

/// calculate the dynamic climate modifier for decomposition 're'
/// calculation is done on the level of ResourceUnit because the water content per day is needed.
double Snag::calculateClimateFactors()
{
    // the calculation of climate factors requires calculated evapotranspiration. In cases without vegetation (trees or saplings)
    // we have to trigger the water cycle calculation for ourselves [ the waterCycle checks if it has already been run in a year and doesn't run twice in that case ]
    const_cast<WaterCycle*>(mRU->waterCycle())->run();
    double ft, fw;
    double f_sum = 0.;
    int iday=0;
    // calculate the water-factor for each month (see Adair et al 2008)
    double fw_month[12];
    double ratio;
    for (int m=0;m<12;m++) {
        if (mRU->waterCycle()->referenceEvapotranspiration()[m]>0.)
            ratio = mRU->climate()->precipitationMonth()[m] /  mRU->waterCycle()->referenceEvapotranspiration()[m];
        else
            ratio = 0;
        fw_month[m] = 1. / (1. + 30.*exp(-8.5*ratio));
        if (logLevelDebug()) qDebug() <<"month"<< m << "PET" << mRU->waterCycle()->referenceEvapotranspiration()[m] << "prec" <<mRU->climate()->precipitationMonth()[m];
    }

    for (const ClimateDay *day=mRU->climate()->begin(); day!=mRU->climate()->end(); ++day, ++iday)
    {
        // empirical variable Q10 model of Lloyd and Taylor (1994), see also Adair et al. (2008)
        // Note: function becomes unstable with very low temperatures (e.g. Alaska)
        ft = day->temperature > -30 ? exp(308.56*(1./56.02-1./((273.+day->temperature)-227.13))) : 0.;
        fw = fw_month[day->month-1];

        f_sum += ft*fw;
    }
    // the climate factor is defined as the arithmentic annual mean value
    mClimateFactor = f_sum / double(mRU->climate()->daysOfYear());
    return mClimateFactor;
}

/// do the yearly calculation
/// see https://iland-model.org/snag+dynamics
void Snag::calculateYear()
{
    mSWDtoSoil.clear();

    // calculate anyway, because also the soil module needs it (and currently one can have Snag and Soil only as a couple)
    calculateClimateFactors();
    const double climate_factor_re = mClimateFactor;

    if (isEmpty()) // nothing to do
        return;

    // process branches and coarse roots: every year one of the five baskets is emptied and transfered to the refractory soil pool
    mRefractoryFlux+=mOtherWood[mBranchCounter];
    mRefrFluxAbovegroundCarbon += mOtherWood[mBranchCounter].C * mOtherWoodAbovegroundFrac; // content * aboveground_fraction
    mOtherWood[mBranchCounter].clear();
    mBranchCounter= (mBranchCounter+1) % 5; // increase index, roll over to 0.

    // decay of branches/coarse roots
    for (int i=0;i<5;i++) {
        if (mOtherWood[i].C>0.) {
            double survive_rate = exp(- climate_factor_re * mOtherWood[i].parameter() ); // parameter: the "kyr" value...
            mTotalToAtm.C += mOtherWood[i].C * (1. - survive_rate); // flux to atmosphere (decayed carbon)
            mOtherWood[i].C *= survive_rate;
        }
    }

    // process standing snags.
    // the input of the current year is in the mToSWD-Pools
    for (int i=0;i<3;i++) {

        // update the swd-pool with this years' input
        if (!mToSWD[i].isEmpty()) {
            // update decay rate (apply average yearly input to the state parameters)
            mKSW[i] = mKSW[i]*(mSWD[i].C/(mSWD[i].C+mToSWD[i].C)) + mCurrentKSW[i]*(mToSWD[i].C/(mSWD[i].C+mToSWD[i].C));
            //move content to the SWD pool
            mSWD[i] += mToSWD[i];
            if (mSWD[i].C < 0.)
                qDebug() << "Snag:calculateYear: C < 0.";
        }
        if (mSWD[i].C < 0.)
            qDebug() << "Snag:calculateYear: C < 0.";

        if (mSWD[i].C > 0.) {
            // reduce the Carbon (note: the N stays, thus the CN ratio changes)
            // use the decay rate that is derived as a weighted average of all standing woody debris
            double survive_rate = exp(-mKSW[i] *climate_factor_re * 1. ); // 1: timestep
            mTotalToAtm.C += mSWD[i].C * (1. - survive_rate);
            mSWD[i].C *= survive_rate;

            // transition to downed woody debris
            // update: use negative exponential decay, species parameter: half-life
            // modified for the climatic effect on decomposition, i.e. if decomp is slower, snags stand longer and vice versa
            // this is loosely oriented on Standcarb2 (http://andrewsforest.oregonstate.edu/pubs/webdocs/models/standcarb2.htm),
            // where lag times for cohort transitions are linearly modified with re although here individual good or bad years have
            // an immediate effect, the average climatic influence should come through (and it is inherently transient)
            // note that swd.hl is species-specific, and thus a weighted average over the species in the input (=mortality)
            // needs to be calculated, followed by a weighted update of the previous swd.hl.
            // As weights here we use stem number, as the processes here pertain individual snags
            // calculate the transition probability of SWD to downed dead wood

            double half_life = mHalfLife[i] / climate_factor_re;
            double rate = -M_LN2 / half_life; // M_LN2: math. constant

            // higher decay rate for the class with smallest diameters
            if (i==0)
                rate*=2.;

            double transfer = 1. - exp(rate);
            if (transfer<0. || transfer>1.)
                qDebug() << "transfer alarm!";

            // calculate flow to soil pool...
            mSWDtoSoil += mSWD[i] * transfer;
            mRefractoryFlux += mSWD[i] * transfer;
            mRefrFluxAbovegroundCarbon += mSWD[i].C * transfer; // all snag biomass is aboveground

            mSWD[i] *= (1.-transfer); // reduce pool
            // calculate the stem number of remaining snags
            mNumberOfSnags[i] = mNumberOfSnags[i] * (1. - transfer);

            mTimeSinceDeath[i] += 1.;
            // if stems<0.5, empty the whole cohort into DWD, i.e. release the last bit of C and N and clear the stats
            // also, if the Carbon of an average snag is less than 10% of the original average tree
            // (derived from allometries for the three diameter classes), the whole cohort is emptied out to DWD
            if (mNumberOfSnags[i] < 0.5 || mSWD[i].C / mNumberOfSnags[i] < mCarbonThreshold[i]) {
                // clear the pool: add the rest to the soil, clear statistics of the pool
                mRefractoryFlux += mSWD[i];
                mRefrFluxAbovegroundCarbon += mSWD[i].C;
                mSWDtoSoil += mSWD[i];
                mSWD[i].clear();
                mAvgDbh[i] = 0.;
                mAvgHeight[i] = 0.;
                mAvgVolume[i] = 0.;
                mKSW[i] = 0.;
                mCurrentKSW[i] = 0.;
                mHalfLife[i] = 0.;
                mTimeSinceDeath[i] = 0.;
                mNumberOfSnags[i] = 0.;
            }

        }

    }
    // total carbon in the snag-container on the RU *after* processing is the content of the
    // standing woody debris pools + the branches
    mTotalSnagCarbon = mSWD[0].C + mSWD[1].C + mSWD[2].C +
                       mOtherWood[0].C + mOtherWood[1].C + mOtherWood[2].C + mOtherWood[3].C + mOtherWood[4].C;
    if (mTotalSnagCarbon < 0.) {
        qDebug() << "SnagCarbon < 0: " << debugList();
    }
    if (isnan(mRefractoryFlux.C))
        qDebug() << "Snag:calculateYear: refr.flux is NAN";

    mTotalSWD = mSWD[0] + mSWD[1] + mSWD[2];
    mTotalOther = mOtherWood[0] + mOtherWood[1] + mOtherWood[2] + mOtherWood[3] + mOtherWood[4];

    if (mTotalOther.N < 0.)
        qDebug() << "Snag-Other N < 0 on RU (index):" << mRU->index();

}

/// foliage and fineroot litter is transferred during tree growth.
void Snag::addTurnoverLitter(const Species *species, const double litter_foliage, const double litter_fineroot)
{
    mLabileFlux.addBiomass(litter_foliage, species->cnFoliage(), species->snagKyl());
    mLabileFluxAbovegroundCarbon += litter_foliage*biomassCFraction;
    mLabileFlux.addBiomass(litter_fineroot, species->cnFineroot(), species->snagKyl());
    if (!species->isConiferous())
        mDeciduousFoliageLitter += litter_foliage;
    DBGMODE(
    if (isnan(mLabileFlux.C))
        qDebug("Snag::addTurnoverLitter: NaN");
                );
}

void Snag::addTurnoverWood(const Species *species, const double woody_biomass)
{
    // NOTE: currently, woody_biomass is *only* coarse root, therefore no split in aboveground/belowground flux here
    mRefractoryFlux.addBiomass(woody_biomass, species->cnWood(), species->snagKyr());
    DBGMODE(
    if (isnan(mRefractoryFlux.C))
        qDebug("Snag::addTurnoverWood: NaN");
                );

}


/** process the remnants of a single tree.
    The part of the stem / branch not covered by snag/soil fraction is removed from the system (e.g. harvest, fire)
  @param tree the tree to process
  @param stem_to_snag fraction (0..1) of the stem biomass that should be moved to a standing snag
  @param stem_to_soil fraction (0..1) of the stem biomass that should go directly to the soil
  @param branch_to_snag fraction (0..1) of the branch biomass that should be moved to a standing snag
  @param branch_to_soil fraction (0..1) of the branch biomass that should go directly to the soil
  @param foliage_to_soil fraction (0..1) of the foliage biomass that should go directly to the soil

*/
void Snag::addBiomassPools(const Tree *tree,
                           const double stem_to_snag, const double stem_to_soil,
                           const double branch_to_snag, const double branch_to_soil,
                           const double foliage_to_soil)
{
    const Species *species = tree->species();

    double branch_biomass = tree->biomassBranch();
    // fine roots go to the labile pool
    mLabileFlux.addBiomass(tree->biomassFineRoot(), species->cnFineroot(), species->snagKyl());

    // a part of the foliage goes to the soil
    mLabileFlux.addBiomass(tree->biomassFoliage() * foliage_to_soil, species->cnFoliage(), species->snagKyl());
    mLabileFluxAbovegroundCarbon += tree->biomassFoliage() * foliage_to_soil * biomassCFraction;

    // aboveground fraction of the "other" pool: aboveground_current + ag_new / (total_c)
    if (mTotalOther.C + branch_to_snag*branch_biomass + tree->biomassCoarseRoot()>0.)
        mOtherWoodAbovegroundFrac =  (mTotalOther.C * mOtherWoodAbovegroundFrac + branch_to_snag*branch_biomass) / (mTotalOther.C + branch_to_snag*branch_biomass + tree->biomassCoarseRoot());

    //coarse roots and a part of branches are equally distributed over five years:
    double biomass_rest = (tree->biomassCoarseRoot() + branch_to_snag*branch_biomass) * 0.2;
    if (biomass_rest < 0.) // make sure we do not have invalid fluxes / debugging
        biomass_rest = 0.;
    for (int i=0;i<5; i++)
        mOtherWood[i].addBiomass(biomass_rest, species->cnWood(), species->snagKyr());

    // the other part of the branches goes directly to the soil
    mRefractoryFlux.addBiomass(branch_biomass*branch_to_soil, species->cnWood(), species->snagKyr() );
    mRefrFluxAbovegroundCarbon += branch_biomass*branch_to_soil*biomassCFraction;

    // a part of the stem wood goes directly to the soil
    mRefractoryFlux.addBiomass(tree->biomassStem()*stem_to_soil, species->cnWood(), species->snagKyr() );
    mRefrFluxAbovegroundCarbon += tree->biomassStem()*stem_to_soil*biomassCFraction;

    if (isnan(mRefractoryFlux.C))
        qDebug() << "addBiomassPools: NAN in refractory pool!";

    // just for book-keeping: keep track of all inputs of branches / roots / swd into the "snag" pools
    mTotalIn.addBiomass(tree->biomassBranch()*branch_to_snag + tree->biomassCoarseRoot() + tree->biomassStem()*stem_to_snag, species->cnWood());
    // stem biomass is transferred to the standing woody debris pool (SWD), increase stem number of pool
    int pi = poolIndex(tree->dbh()); // get right transfer pool

    if (stem_to_snag>0.) {
        // update statistics - stemnumber-weighted averages
        // note: here the calculations are repeated for every died trees (i.e. consecutive weighting ... but delivers the same results)
        double p_old = mNumberOfSnags[pi] / (mNumberOfSnags[pi] + 1); // weighting factor for state vars (based on stem numbers)
        double p_new = 1. / (mNumberOfSnags[pi] + 1); // weighting factor for added tree (p_old + p_new = 1).
        mAvgDbh[pi] = mAvgDbh[pi]*p_old + tree->dbh()*p_new;
        mAvgHeight[pi] = mAvgHeight[pi]*p_old + tree->height()*p_new;
        mAvgVolume[pi] = mAvgVolume[pi]*p_old + tree->volume()*p_new;
        mTimeSinceDeath[pi] = mTimeSinceDeath[pi]*p_old + 1.*p_new;
        mHalfLife[pi] = mHalfLife[pi]*p_old + species->snagHalflife()* p_new;

        // average the decay rate (ksw); this is done based on the carbon content
        // aggregate all trees that die in the current year (and save weighted decay rates to CurrentKSW)
        if (tree->biomassStem()>0.) {
            p_old = mToSWD[pi].C / (mToSWD[pi].C + tree->biomassStem()* biomassCFraction);
            p_new =tree->biomassStem()* biomassCFraction / (mToSWD[pi].C + tree->biomassStem()* biomassCFraction);
            mCurrentKSW[pi] = mCurrentKSW[pi]*p_old + species->snagKsw() * p_new;
        }
        mNumberOfSnags[pi]++;
    }

    // finally add the biomass of the stem to the standing snag pool
    CNPool &to_swd = mToSWD[pi];
    if (to_swd.C < 0.)
        qDebug() << "Snag:addBiomassPool: swd<0";
    to_swd.addBiomass(tree->biomassStem()*stem_to_snag, species->cnWood(), species->snagKyr());
    if (to_swd.C < 0.)
        qDebug() << "Snag:addBiomassPool: swd<0";

    // the biomass that is not routed to snags or directly to the soil
    // is removed from the system (to atmosphere or harvested)
    mTotalToExtern.addBiomass(tree->biomassFoliage()* (1. - foliage_to_soil) +
                              branch_biomass*(1. - branch_to_snag - branch_to_soil) +
                              tree->biomassStem()*(1. - stem_to_snag - stem_to_soil), species->cnWood());

}


/// after the death of the tree the five biomass compartments are processed.
void Snag::addMortality(const Tree *tree)
{
    addBiomassPools(tree, 1., 0.,  // all stem biomass goes to snag
                    1., 0.,        // all branch biomass to snag
                    1.);           // all foliage to soil

//    const Species *species = tree->species();

//    // immediate flows: 100% of foliage, 100% of fine roots: they go to the labile pool
//    mLabileFlux.addBiomass(tree->biomassFoliage(), species->cnFoliage(), tree->species()->snagKyl());
//    mLabileFlux.addBiomass(tree->biomassFineRoot(), species->cnFineroot(), tree->species()->snagKyl());

//    // branches and coarse roots are equally distributed over five years:
//    double biomass_rest = (tree->biomassBranch()+tree->biomassCoarseRoot()) * 0.2;
//    for (int i=0;i<5; i++)
//        mOtherWood[i].addBiomass(biomass_rest, species->cnWood(), tree->species()->snagKyr());

//    // just for book-keeping: keep track of all inputs into branches / roots / swd
//    mTotalIn.addBiomass(tree->biomassBranch() + tree->biomassCoarseRoot() + tree->biomassStem(), species->cnWood());
//    // stem biomass is transferred to the standing woody debris pool (SWD), increase stem number of pool
//    int pi = poolIndex(tree->dbh()); // get right transfer pool

//    // update statistics - stemnumber-weighted averages
//    // note: here the calculations are repeated for every died trees (i.e. consecutive weighting ... but delivers the same results)
//    double p_old = mNumberOfSnags[pi] / (mNumberOfSnags[pi] + 1); // weighting factor for state vars (based on stem numbers)
//    double p_new = 1. / (mNumberOfSnags[pi] + 1); // weighting factor for added tree (p_old + p_new = 1).
//    mAvgDbh[pi] = mAvgDbh[pi]*p_old + tree->dbh()*p_new;
//    mAvgHeight[pi] = mAvgHeight[pi]*p_old + tree->height()*p_new;
//    mAvgVolume[pi] = mAvgVolume[pi]*p_old + tree->volume()*p_new;
//    mTimeSinceDeath[pi] = mTimeSinceDeath[pi]*p_old + 1.*p_new;
//    mHalfLife[pi] = mHalfLife[pi]*p_old + species->snagHalflife()* p_new;

//    // average the decay rate (ksw); this is done based on the carbon content
//    // aggregate all trees that die in the current year (and save weighted decay rates to CurrentKSW)
//    if (tree->biomassStem()==0)
//        throw IException("Snag::addMortality: tree without stem biomass!!");
//    p_old = mToSWD[pi].C / (mToSWD[pi].C + tree->biomassStem()* biomassCFraction);
//    p_new =tree->biomassStem()* biomassCFraction / (mToSWD[pi].C + tree->biomassStem()* biomassCFraction);
//    mCurrentKSW[pi] = mCurrentKSW[pi]*p_old + species->snagKsw() * p_new;
//    mNumberOfSnags[pi]++;

//    // finally add the biomass
//    CNPool &to_swd = mToSWD[pi];
//    to_swd.addBiomass(tree->biomassStem(), species->cnWood(), tree->species()->snagKyr());
}

/// add residual biomass of 'tree' after harvesting.
/// remove_{stem, branch, foliage}_fraction: percentage of biomass compartment that is *removed* by the harvest operation [0..1] (i.e.: not to stay in the system)
/// records on harvested biomass is collected (mTotalToExtern-pool).
void Snag::addHarvest(const Tree* tree, const double remove_stem_fraction, const double remove_branch_fraction, const double remove_foliage_fraction )
{
    addBiomassPools(tree,
                    0., 1.-remove_stem_fraction, // "remove_stem_fraction" is removed -> the rest goes to soil
                    0., 1.-remove_branch_fraction, // "remove_branch_fraction" is removed -> the rest goes directly to the soil
                    1.-remove_foliage_fraction); // the rest of foliage is routed to the soil
//    const Species *species = tree->species();

//    // immediate flows: 100% of residual foliage, 100% of fine roots: they go to the labile pool
//    mLabileFlux.addBiomass(tree->biomassFoliage() * (1. - remove_foliage_fraction), species->cnFoliage(), tree->species()->snagKyl());
//    mLabileFlux.addBiomass(tree->biomassFineRoot(), species->cnFineroot(), tree->species()->snagKyl());

//    // for branches, add all biomass that remains in the forest to the soil
//    mRefractoryFlux.addBiomass(tree->biomassBranch()*(1.-remove_branch_fraction), species->cnWood(), tree->species()->snagKyr());
//    // the same treatment for stem residuals
//    mRefractoryFlux.addBiomass(tree->biomassStem() * (1. - remove_stem_fraction), species->cnWood(), tree->species()->snagKyr());

//    // split the corase wood biomass into parts (slower decay)
//    double biomass_rest = (tree->biomassCoarseRoot()) * 0.2;
//    for (int i=0;i<5; i++)
//        mOtherWood[i].addBiomass(biomass_rest, species->cnWood(), tree->species()->snagKyr());


//    // for book-keeping...
//    mTotalToExtern.addBiomass(tree->biomassFoliage()*remove_foliage_fraction +
//                              tree->biomassBranch()*remove_branch_fraction +
//                              tree->biomassStem()*remove_stem_fraction, species->cnWood());
}

// add flow from regeneration layer (dead trees) to soil
void Snag::addToSoil(const Species *species, const CNPair &woody_pool, const CNPair &litter_pool, double woody_aboveground_C, double fine_aboveground_C)
{
    mLabileFlux.add(litter_pool, species->snagKyl());
    mLabileFluxAbovegroundCarbon += fine_aboveground_C;
    mRefractoryFlux.add(woody_pool, species->snagKyr());
    mRefrFluxAbovegroundCarbon += woody_aboveground_C;
    DBGMODE(
    if (isnan(mLabileFlux.C) || isnan(mRefractoryFlux.C))
        qDebug("Snag::addToSoil: NaN in C Pool");
            );
}

void Snag::addBiomassToSoil(const CNPool &woody_pool, const CNPool &litter_pool)
{
    // add the biomass (fluxes in kg/ha)
    mLabileFlux.add(litter_pool, litter_pool.parameter());
    mRefractoryFlux.add(woody_pool, woody_pool.parameter());
    // assume all biomass input is from above
    mLabileFluxAbovegroundCarbon += litter_pool.C;
    mRefrFluxAbovegroundCarbon += woody_pool.C;
}

/// disturbance function: remove the fraction of 'factor' of biomass from the SWD pools; 0: remove nothing, 1: remove all
/// biomass removed by this function goes to the atmosphere
void Snag::removeCarbon(const double factor)
{
    if (factor < 0. || factor > 1.) {
        qDebug() << "Snag:removeCarbon: invalid factor (allowed: [0-1]):" << factor;
    }
    // reduce pools of currently standing dead wood and also of pools that are added
    // during (previous) management operations of the current year
    for (int i=0;i<3;i++) {
        mTotalToDisturbance += (mSWD[i] + mToSWD[i]) * factor;
        mSWD[i] *= (1. - factor);
        mToSWD[i] *= (1. - factor);
    }

    for (int i=0;i<5;i++) {
        mTotalToDisturbance += mOtherWood[i]*factor;
        mOtherWood[i] *= (1. - factor);
    }
}


/// cut down swd (and branches) and move to soil pools
/// @param factor 0: cut 0%, 1: cut and slash 100% of the wood
void Snag::management(const double factor)
{
    if (factor<0. || factor>1.)
        throw IException(QString("Invalid factor in Snag::management (valid: [0,1]: '%1'").arg(factor));
    // swd pools
    for (int i=0;i<3;i++) {
        mSWDtoSoil += mSWD[i] * factor;
        mRefractoryFlux += mSWD[i] * factor;
        mRefrFluxAbovegroundCarbon += mSWD[i].C * factor;
        mSWD[i] *= (1. - factor);
        //mSWDtoSoil += mToSWD[i] * factor;
        //mToSWD[i] *= (1. - factor);
    }
    // for branches, we move only the fraction of aboveground carbon to the ground
    for (int i=0;i<5;i++) {
        double ag_factor = mOtherWoodAbovegroundFrac * factor; // we move only aboveground carbon to the soil
        mRefractoryFlux+=mOtherWood[i]*ag_factor;
        mRefrFluxAbovegroundCarbon+=mOtherWood[i].C * ag_factor;
        mOtherWood[i]*=(1. - ag_factor);
        if (ag_factor < 1.)
            mOtherWoodAbovegroundFrac = (mOtherWoodAbovegroundFrac - ag_factor) / (1. - ag_factor); // the fraction of aboveground carbon
    }

}


