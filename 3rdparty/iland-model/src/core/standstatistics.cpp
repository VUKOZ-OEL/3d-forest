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

/** @class StandStatistics
  @ingroup tools
  Collects information on stand level for each tree species.
  Call clear() to clear the statistics, then call add() for each tree and finally calculate().
  To aggregate on a higher level, use add() for each StandStatistics object to include, and then
  calculate() on the higher level.
  Todo-List for new items:
  - add a member variable and a getter
  - add to "add(Tree)" and "calculate()"
  - add to "add(StandStatistics)" as well!
  */
#include "standstatistics.h"
#include "tree.h"
#include "resourceunit.h"
#include "resourceunitspecies.h"
//#include "sapling.h"
#include "saplings.h"
#include "species.h"

void StandStatistics::clear()
{
    // reset all values
    mCount = 0;
    mSumDbh=mSumHeight = mAverageDbh=mAverageHeight =0.;
    mSumBasalArea = mSumVolume = mGWL = 0.;
    mLeafAreaIndex = 0.;
    mNPP = mNPPabove = 0.;
    mNPPsaplings = 0.;
    mCohortCount = mSaplingCount = 0;
    mAverageSaplingAge = 0.;
    mSumSaplingAge = 0.;
    mLAISaplings = 0.;
    mBasalAreaSaplings=0.;
    mCStem=0., mCFoliage=0., mCBranch=0., mCCoarseRoot=0., mCFineRoot=0.;
    mNStem=0., mNFoliage=0., mNBranch=0., mNCoarseRoot=0., mNFineRoot=0.;
    mCRegeneration=0., mNRegeneration=0.;

}

void StandStatistics::clearOnlyTrees()
{
    // reset only those values that are directly accumulated from trees
    mCount = 0;
    mSumDbh=mSumHeight = mAverageDbh=mAverageHeight =0.;
    mSumBasalArea = mSumVolume = mGWL = 0.;
    mLeafAreaIndex = 0.;
    /*mNPP = mNPPabove = 0.;
    mNPPsaplings = 0.;
    mCohortCount = mSaplingCount = 0;
    mAverageSaplingAge = 0.;
    mSumSaplingAge = 0.;*/
    mCStem=0., mCFoliage=0., mCBranch=0., mCCoarseRoot=0., mCFineRoot=0.;
    mNStem=0., mNFoliage=0., mNBranch=0., mNCoarseRoot=0., mNFineRoot=0.;
    /*mCRegeneration=0., mNRegeneration=0.;*/

}

void StandStatistics::addBiomass(const double biomass, const double CNRatio, double *C, double *N)
{
    *C+=biomass*biomassCFraction;
    *N+=biomass*biomassCFraction/CNRatio;
}

void StandStatistics::add(const Tree *tree, const TreeGrowthData *tgd)
{
    mCount++;
    mSumDbh+=tree->dbh();
    mSumHeight+=tree->height();
    mSumBasalArea+=tree->basalArea();
    mSumVolume += tree->volume();
    mLeafAreaIndex += tree->leafArea(); // warning: sum of leafarea!
    if (tgd) {
        mNPP += tgd->NPP;
        mNPPabove += tgd->NPP_above;
    }
    // carbon and nitrogen pools
    addBiomass(tree->biomassStem(), tree->species()->cnWood(), &mCStem, &mNStem);
    addBiomass(tree->biomassBranch(), tree->species()->cnWood(), &mCBranch, &mNBranch);
    addBiomass(tree->biomassFoliage(), tree->species()->cnFoliage(), &mCFoliage, &mNFoliage);
    addBiomass(tree->biomassFineRoot(), tree->species()->cnFineroot(), &mCFineRoot, &mNFineRoot);
    addBiomass(tree->biomassCoarseRoot(), tree->species()->cnWood(), &mCCoarseRoot, &mNCoarseRoot);
}

void StandStatistics::addNPP(const TreeGrowthData *tgd)
{
    // add NPP of trees that died due to mortality
    mNPP += tgd->NPP;
    mNPPabove += tgd->NPP_above;

}

// note: mRUS = 0 for aggregated statistics
void StandStatistics::calculate()
{
    if (mCount>0.) {
        mAverageDbh = mSumDbh / mCount;
        mAverageHeight = mSumHeight / mCount;
        if (mRUS && mRUS->ru()->stockableArea()>0.)
            mLeafAreaIndex /= mRUS->ru()->stockableArea(); // convert from leafarea to LAI
    }
    if (mCohortCount)
        mAverageSaplingAge = mSumSaplingAge / double(mCohortCount);

    // scale values to per hectare if resource unit <> 1ha
    // note: do this only on species-level (avoid double scaling)
    if (mRUS) {
        double area_factor =  cRUArea / mRUS->ru()->stockableArea();
        if (area_factor!=1.) {
            mCount = mCount * area_factor;
            mSumBasalArea *= area_factor;
            mSumVolume *= area_factor;
            mSumDbh *= area_factor;
            mNPP *= area_factor;
            mNPPabove *= area_factor;
            mNPPsaplings *= area_factor;
            //mGWL *= area_factor;
            mCohortCount *= area_factor;
            mSaplingCount *= area_factor;
            //double mCStem, mCFoliage, mCBranch, mCCoarseRoot, mCFineRoot;
            //double mNStem, mNFoliage, mNBranch, mNCoarseRoot, mNFineRoot;
            //double mCRegeneration, mNRegeneration;
            mCStem *= area_factor; mNStem *= area_factor;
            mCFoliage *= area_factor; mNFoliage *= area_factor;
            mCBranch *= area_factor; mNBranch *= area_factor;
            mCCoarseRoot *= area_factor; mNCoarseRoot *= area_factor;
            mCFineRoot *= area_factor; mNFineRoot *= area_factor;
            mCRegeneration *= area_factor; mNRegeneration *= area_factor;

        }
        mGWL = mSumVolume + mRUS->removedVolume(); // removedVolume: per ha, SumVolume now too
    }
}

void StandStatistics::calculateAreaWeighted()
{
    // let a= area ru / total area: (see addAreaWeighted())
    // mAverageDbh = sum( count_ha * average_dbh * a )
    // mCount = sum( count_ha * a )

    // calculate averages
    if (mCount>0.){
        mAverageDbh=mSumDbh / mCount;
        mAverageHeight=mSumHeight / mCount;
    }
    if (mSaplingCount>0.) {
        mAverageSaplingAge=mSumSaplingAge / mSaplingCount;
    }
}

void StandStatistics::add(const StandStatistics &stat)
{
    mCount+=stat.mCount;
    mSumBasalArea+=stat.mSumBasalArea;
    mSumDbh+=stat.mSumDbh;
    mSumHeight+=stat.mSumHeight;
    mSumVolume+=stat.mSumVolume;
    mLeafAreaIndex += stat.mLeafAreaIndex;
    mNPP += stat.mNPP;
    mNPPabove += stat.mNPPabove;
    mNPPsaplings += stat.mNPPsaplings;
    mGWL+=stat.mGWL;
    // regeneration
    mCohortCount += stat.mCohortCount;
    mSaplingCount += stat.mSaplingCount;
    mSumSaplingAge += stat.mSumSaplingAge;
    mLAISaplings += stat.mLAISaplings;
    mBasalAreaSaplings += stat.mBasalAreaSaplings;
    // carbon/nitrogen pools
    mCStem += stat.mCStem; mNStem += stat.mNStem;
    mCBranch += stat.mCBranch; mNBranch += stat.mNBranch;
    mCFoliage += stat.mCFoliage; mNFoliage += stat.mNFoliage;
    mCFineRoot += stat.mCFineRoot; mNFineRoot += stat.mNFineRoot;
    mCCoarseRoot += stat.mCCoarseRoot; mNCoarseRoot += stat.mNCoarseRoot;
    mCRegeneration += stat.mCRegeneration; mNRegeneration += stat.mNRegeneration;

}

void StandStatistics::addAreaWeighted(const StandStatistics &stat, const double weight)
{
    // aggregates that are not scaled to hectares
    mCount+=stat.mCount * weight;
    mSumBasalArea+=stat.mSumBasalArea * weight;
    mSumDbh+=stat.mAverageDbh * stat.mCount * weight;
    mSumHeight+=stat.mAverageHeight * stat.mCount * weight;
    mSumSaplingAge+=stat.mAverageSaplingAge * stat.mSaplingCount * weight;
    mSumVolume+=stat.mSumVolume * weight;
    // averages that are scaled to per hectare need to be scaled
    //mAverageDbh+=stat.mAverageDbh * weight;
    //mAverageHeight+=stat.mAverageHeight * weight;
    //mAverageSaplingAge+=stat.mAverageSaplingAge * weight;
    mLeafAreaIndex += stat.mLeafAreaIndex * weight;
    mLAISaplings += stat.mLAISaplings * weight;
    mBasalAreaSaplings += stat.mBasalAreaSaplings * weight;

    mNPP += stat.mNPP * weight;
    mNPPabove += stat.mNPPabove * weight;
    mNPPsaplings += stat.mNPPsaplings * weight;
    mGWL+=stat.mGWL * weight;
    // regeneration
    mCohortCount += stat.mCohortCount * weight;
    mSaplingCount += stat.mSaplingCount * weight;

    // carbon/nitrogen pools
    mCStem += stat.mCStem * weight; mNStem += stat.mNStem * weight;
    mCBranch += stat.mCBranch * weight; mNBranch += stat.mNBranch * weight;
    mCFoliage += stat.mCFoliage * weight; mNFoliage += stat.mNFoliage * weight;
    mCFineRoot += stat.mCFineRoot * weight; mNFineRoot += stat.mNFineRoot * weight;
    mCCoarseRoot += stat.mCCoarseRoot * weight; mNCoarseRoot += stat.mNCoarseRoot * weight;
    mCRegeneration += stat.mCRegeneration * weight; mNRegeneration += stat.mNRegeneration * weight;

}



void StandStatistics::add(const SaplingStat *sapling)
{
    mCohortCount += sapling->livingCohorts();
    mSaplingCount += sapling->livingSaplings(); // saplings with height >1.3m

    mSumSaplingAge += sapling->averageAge() * sapling->livingCohorts();

    mCRegeneration += sapling->carbonLiving().C;
    mNRegeneration += sapling->carbonLiving().N;

    mNPPsaplings += sapling->carbonGain().C / biomassCFraction;
    mLAISaplings += sapling->leafAreaIndex();

    mBasalAreaSaplings += sapling->basalArea();

}

void SystemStatistics::writeOutput()
{
    if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dPerformance)) {
        DebugList &out = GlobalSettings::instance()->debugList(0, GlobalSettings::dPerformance);
        out << treeCount << saplingCount << newSaplings << tManagement
            << tApplyPattern << tReadPattern << tTreeGrowth
            << tSeedDistribution  << tEstablishment << tSapling
            << tCarbonCycle << tWriteOutput << tTotalYear;
    }
}


