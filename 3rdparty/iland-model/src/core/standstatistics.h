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

#ifndef STANDSTATISTICS_H
#define STANDSTATISTICS_H
class Tree;
struct TreeGrowthData;
class ResourceUnitSpecies;
class SaplingStat;

class StandStatistics
{
public:
    StandStatistics() { mRUS=0; clear();}
    void setResourceUnitSpecies(const ResourceUnitSpecies *rus) { mRUS=rus; }

    void add(const StandStatistics &stat); ///< add aggregates of @p stat to own aggregates
    void addAreaWeighted(const StandStatistics &stat, const double weight); ///< add aggregates of @p stat to this aggregate and scale using the weight (e.g. stockable area)
    void add(const Tree *tree, const TreeGrowthData *tgd); ///< call for each tree within the domain
    void addNPP(const TreeGrowthData *tgd); ///< add only the NPP

    void add(const SaplingStat *sapling); ///< call for regeneration layer of a species in resource unit
    void clear(); ///< call before trees are aggregated
    void clearOnlyTrees(); ///< clear the statistics only for tree biomass (keep NPP, regen, ...)
    void calculate(); ///< call after all trees are processed (postprocessing)
    void calculateAreaWeighted(); ///< call after a series of addAreaWeighted
    // getters
    double count() const { return mCount; }
    double dbh_avg() const { return mAverageDbh; } ///< average dbh (cm)
    double height_avg() const { return mAverageHeight; } ///< average tree height (m)
    double volume() const { return mSumVolume; } ///< sum of tree volume (m3/ha)
    double gwl() const { return mGWL;} ///< total increment (m3/ha)
    double basalArea() const { return mSumBasalArea; } ///< sum of basal area of all trees (m2/ha)
    double leafAreaIndex() const { return mLeafAreaIndex; } ///< [m2/m2]/ha stocked area (trees > 4m)
    double leafAreaIndexSaplings() const { return mLAISaplings; } ///< m2/m2 LAI of sapling layer
    double npp() const { return mNPP; } ///< sum. of NPP (kg Biomass increment, above+belowground, trees >4m)/ha
    double nppAbove() const { return mNPPabove; } ///< above ground NPP (kg Biomass increment)/ha
    double nppSaplings() const { return mNPPsaplings; } ///< carbon gain of saplings (kg Biomass increment)/ha
    int cohortCount() const { return static_cast<int>(mCohortCount); } ///< number of cohorts of saplings / ha
    int saplingCount() const { return static_cast<int>(mSaplingCount); } ///< number individuals in regeneration layer (represented by "cohortCount" cohorts) > 1.3m N/ha
    double saplingAge() const { return mAverageSaplingAge; } ///< average age of sapling (currenty not weighted with represented sapling numbers...)
    double saplingBasalArea() const { return mBasalAreaSaplings; } ///< total basal area (m2) of saplings (>1.3m)
    // carbon/nitrogen cycle
    double cStem() const { return mCStem; }
    double nStem() const { return mNStem; }
    double cBranch() const { return mCBranch; }
    double nBranch() const { return mNBranch; }
    double cFoliage() const { return mCFoliage; }
    double nFoliage() const { return mNFoliage; }
    double cCoarseRoot() const { return mCCoarseRoot; }
    double nCoarseRoot() const { return mNCoarseRoot; }
    double cFineRoot() const { return mCFineRoot; }
    double nFineRoot() const { return mNFineRoot; }
    double cRegeneration() const { return mCRegeneration; }
    double nRegeneration() const { return mNRegeneration; }
    /// total carbon stock: sum of carbon of all living trees + regeneration layer
    double totalCarbon() const { return mCStem + mCBranch + mCFoliage + mCFineRoot + mCCoarseRoot + mCRegeneration; }

private:
    inline void addBiomass(const double biomass, const double CNRatio, double *C, double *N);
    const ResourceUnitSpecies *mRUS; ///< link to the resource unit species
    double mCount;
    double mSumDbh;
    double mSumHeight;
    double mSumBasalArea;
    double mSumVolume;
    double mGWL;
    double mAverageDbh;
    double mAverageHeight;
    double mLeafAreaIndex;
    double mNPP;
    double mNPPabove;
    double mNPPsaplings; // carbon gain of saplings (kg Biomass)
    // regeneration layer
    double mCohortCount; ///< number of cohrots
    double mSaplingCount; ///< number of sapling (Reinekes Law)
    double mSumSaplingAge;
    double mAverageSaplingAge;
    double mLAISaplings; ///< LAI (m2/m2) of sapling layer
    double mBasalAreaSaplings; ///< basal area (m2) in sapling layer
    // carbon and nitrogen pools
    double mCStem, mCFoliage, mCBranch, mCCoarseRoot, mCFineRoot;
    double mNStem, mNFoliage, mNBranch, mNCoarseRoot, mNFineRoot;
    double mCRegeneration, mNRegeneration;
};


/** holds a couple of system statistics primarily aimed for performance and memory analyis.
  */
class SystemStatistics
{
public:
    SystemStatistics() { reset(); }
    void reset() { treeCount=0; saplingCount=0; newSaplings=0;
                   tManagement = 0.; tApplyPattern=tReadPattern=tTreeGrowth=0.;
                   tSeedDistribution=tSapling=tEstablishment=tCarbonCycle=tWriteOutput=tTotalYear=0.; }
    void writeOutput();
    // the system counters
    int treeCount;
    int saplingCount;
    int newSaplings;
    // timings
    double tManagement;
    double tApplyPattern;
    double tReadPattern;
    double tTreeGrowth;
    double tSeedDistribution;
    double tSapling;
    double tEstablishment;
    double tCarbonCycle;
    double tWriteOutput;
    double tTotalYear;

};

#endif // STANDSTATISTICS_H
