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

#ifndef SNAG_H
#define SNAG_H
#include <QList>
#include <QVariant>
class Tree; // forward
class Species; // forward
class ResourceUnit; // forward

/** CNPair stores a duple of carbon and nitrogen (kg/ha)
    use addBiomass(biomass, cnratio) to add biomass; use operators (+, +=, *, *=) for simple operations. */
class CNPair
{
public:
    CNPair(): C(0.), N(0.) {}
    static void setCFraction(const double fraction) { biomassCFraction = fraction; } ///< set the global fraction of carbon of biomass
    CNPair(const double c, const double n) {C=c; N=n; }
    bool isEmpty() const { return C==0.; } ///< returns true if pool is empty
    bool isValid() const { return C>=0. && N>=0.; } ///< return true if pool is valid (content of C or N >=0)
    double C; // carbon pool (kg C/ha)
    double N; // nitrogen pool (kg N/ha)
    double CN() { return N>0?C/N:0.; } ///< current CN ratio
    void clear() {C=0.; N=0.; }
    /// retrieve the amount of biomass (kg/ha). Uses the global C-fraciton. Soil pools are in t/ha!!!
    double biomass() const {return C / biomassCFraction; }
    /// add biomass to the pool (kg dry mass/ha); CNratio is used to calculate the N-Content, the global C-Fraction of biomass is used to
    /// calculate the amount of carbon of 'biomass'.
    void addBiomass(const double biomass, const double CNratio) { C+=biomass*biomassCFraction; N+=biomass*biomassCFraction/CNratio; }
    // some simple operators
    void operator+=(const CNPair &s) { C+=s.C; N+=s.N; } ///< add contents of a pool
    void operator*=(const double factor) { C*=factor; N*=factor; } ///< Multiply pool with 'factor'
    const CNPair operator+(const CNPair &p2) const { return CNPair(C+p2.C, N+p2.N); } ///< return the sum of two pools
    const CNPair operator-(const CNPair &p2) const { return CNPair(C-p2.C, N-p2.N); } ///< return the difference of two pools
    const CNPair operator*(const double factor) const { return CNPair(C*factor, N*factor); } ///< return the pool multiplied with 'factor'
protected:
    static double biomassCFraction;
};

/** CNPool provides (in addition to CNPair) also a weighted parameter value (e.g. a decay rate) */
class CNPool: public CNPair
{
public:
    CNPool(): CNPair(), mParameter(0.) {}
    CNPool(const double c, const double n, const double param_value) {C=c; N=n; mParameter=param_value; }
    double parameter() const { return mParameter; } ///< get weighting parameter
    double parameter(const CNPool &s) const; ///< 'simulate' weighting (get weighted param value of 's' with the current content)
    void clear() { CNPair::clear(); mParameter=0.; }

    /// add biomass with a specific 'CNRatio' and 'parameter_value'
    void addBiomass(const double biomass, const double CNratio, const double parameter_value);
    void add(const CNPair &s, const double parameter_value) { operator+=(CNPool(s.C, s.N, parameter_value));} ///< convenience function
    void setParameter(const double value) { mParameter = value; }

    void operator+=(const CNPool &s);
    const CNPool operator*(const double factor) const { return CNPool(C*factor, N*factor, mParameter); } ///< return the pool multiplied with 'factor'
private:
    double mParameter;
};

class Snag
{
public:
    Snag();
    static void setupThresholds(const double lower, const double upper); ///< setup class thresholds, needs to be called only once... (static)
    void setup( const ResourceUnit *ru); ///< initial setup routine.
    void scaleInitialState(); ///< used to scale the input to the actual area of the resource unit
    void newYear(); ///< to be executed at the beginning of a simulation year. This cleans up the transfer pools.
    void calculateYear(); ///< to be called at the end of the year (after tree growth, harvesting). Calculates flow to the soil.
    // access
    bool isStateEmpty() const { return mTotalSnagCarbon == 0.; }
    bool isEmpty() const { return mLabileFlux.isEmpty() && mRefractoryFlux.isEmpty() && isStateEmpty(); }
    const CNPool &labileFlux() const { return mLabileFlux; } ///< litter flux to the soil (kg/ha)
    const CNPool &refractoryFlux() const {return  mRefractoryFlux; } ///< deadwood flux to the soil (kg/ha)
    double labileFluxAbovegroundCarbon() const { return mLabileFluxAbovegroundCarbon; } ///< C input to the labile flux from aboveground sources (kg/ha)
    double refractoryFluxAbovegroundCarbon() const { return mRefrFluxAbovegroundCarbon; } ///< C input of the input to the refractory (woody) flux from aboveground sources (kg/ha)
    double climateFactor() const { return mClimateFactor; } ///< the 're' climate factor to modify decay rates (also used in ICBM/2N model)
    double totalCarbon() const { return mTotalSnagCarbon; } ///< total carbon in snags (kg/RU): not scaled to 1ha!!
    const CNPair &totalSWD() const { return mTotalSWD; } ///< sum of C and N in SWD pools (stems) kg/RU
    const CNPair &totalOtherWood() const { return mTotalOther; } ///< sum of C and N in other woody pools (branches + coarse roots) kg/RU
    double otherWoodAbovegroundFraction() const { return mOtherWoodAbovegroundFrac; } ///< fraction of branches in 'other' pools (0..1)
    const CNPair &fluxToAtmosphere() const { return mTotalToAtm; } ///< total kg/RU heterotrophic respiration / flux to atm
    const CNPair &fluxToExtern() const { return mTotalToExtern; } ///< total kg/RU harvests
    const CNPair &fluxToDisturbance() const { return mTotalToDisturbance; } ///< total kg/RU due to disturbance (e.g. fire)

    /// deciduous foliage litter (kg/ha) from the previous year
    double freshDeciduousFoliage() const { return mDeciduousFoliageLitter; }
    /// set deciduous foliage litter to 0 - this is not ideal, but timing is complicated
    /// as litter input comes from both trees and saplings, and the only "user" at the moment
    /// is permafrost, which is executed between both processes
    void resetDeciduousFoliage() { mDeciduousFoliageLitter = 0.; }
    // actions
    /// add for a tree with diameter
    void addTurnoverLitter(const Species *species, const double litter_foliage, const double litter_fineroot);
    void addTurnoverWood(const Species *species, const double woody_biomass);

    /// adds the 'tree' to the appropriate Snag pools.
    void addMortality(const Tree* tree);

    /// add residual biomass of 'tree' after harvesting.
    /// remove_(stem, branch, foliage)_pct: percentage of biomass compartment that is removed by the harvest operation.
    /// the harvested biomass is collected.
    void addHarvest(const Tree* tree, const double remove_stem_pct, const double remove_branch_pct, const double remove_foliage_pct );

    /// a tree dies and the biomass of the tree is split between snags/soils/removals
    /// @param tree the tree to process
    /// @param stem_to_snag fraction (0..1) of the stem biomass that should be moved to a standing snag
    /// @param stem_to_soil fraction (0..1) of the stem biomass that should go directly to the soil
    /// @param branch_to_snag fraction (0..1) of the branch biomass that should be moved to a standing snag
    /// @param branch_to_soil fraction (0..1) of the branch biomass that should go directly to the soil
    /// @param foliage_to_soil fraction (0..1) of the foliage biomass that should go directly to the soil
    void addDisturbance(const Tree *tree, const double stem_to_snag, const double stem_to_soil,
                        const double branch_to_snag, const double branch_to_soil,
                        const double foliage_to_soil) { addBiomassPools(tree, stem_to_snag, stem_to_soil, branch_to_snag, branch_to_soil, foliage_to_soil);}

    /// add (died) biomass from the regeneration layer
    void addToSoil(const Species *species, const CNPair &woody_pool, const CNPair &litter_pool, double woody_aboveground_C, double fine_aboveground_C);

    /// add non-tree biomass to soil (litter input). Inputs in kg/ha. decomposition rate in parameter value of CNPool.
    void addBiomassToSoil(const CNPool &woody_pool, const CNPool &litter_pool);

    /// disturbance function: remove the fraction of 'factor' of biomass from the SWD pools; 0: remove nothing, 1: remove all
    void removeCarbon(const double factor);
    /// cut down swd and move to soil pools
    void management(const double factor);
    QList<QVariant> debugList(); ///< return a debug output
private:
    /// split the biomass of a tree into snag pools or move part of the tree directly to the soil
    void addBiomassPools(const Tree *tree, const double stem_to_snag, const double stem_to_soil, const double branch_to_snag, const double branch_to_soil, const double foliage_to_soil);
    double calculateClimateFactors(); ///< calculate climate factor 're' for the current year
    double mClimateFactor; ///< current mean climate factor (influenced by temperature and soil water content)
    const ResourceUnit *mRU; ///< link to resource unit
    /// access SWDPool as function of diameter (cm)
    int poolIndex(const float dbh) { if (dbh<mDBHLower) return 0; if (dbh>mDBHHigher) return 2; return 1;}
    CNPool mSWD[3]; ///< standing woody debris pool (0: smallest dimater class, e.g. <10cm, 1: medium, 2: largest class (e.g. >30cm)) kg/ha
    CNPair mTotalSWD; ///< sum of mSWD[x]
    double mNumberOfSnags[3]; ///< number of snags in diameter class
    double mAvgDbh[3]; ///< average diameter in class (cm)
    double mAvgHeight[3]; ///< average height in class (m)
    double mAvgVolume[3]; ///< average volume in class (m3)
    double mTimeSinceDeath[3]; ///< time since death: mass-weighted age of the content of the snag pool
    double mKSW[3]; ///< standing woody debris decay rate (weighted average of species values)
    double mCurrentKSW[3]; ///< swd decay rate (average for trees of the current year)
    double mHalfLife[3]; ///< half-life values (yrs) (averaged)
    CNPool mToSWD[3]; ///< transfer pool; input of the year is collected here (for each size class)
    CNPool mLabileFlux; ///< flux to labile soil pools (kg/ha)
    CNPool mRefractoryFlux; ///< flux to the refractory soil pool (kg/ha)
    CNPool mOtherWood[5]; ///< pool for branch biomass and coarse root biomass
    CNPair mTotalOther; ///< sum of mOtherWood[x]
    int mBranchCounter; ///< index which of the branch pools should be emptied
    double mDeciduousFoliageLitter; ///< foliage litter (biomass) form deciduous trees of the year (kg/ha) [Note: is cleared at the end of the year]
    double mLabileFluxAbovegroundCarbon; ///< C input to the labile flux from aboveground sources (kg/ha)
    double mRefrFluxAbovegroundCarbon; ///< C input to the woody (refractory) pools from aboveground sources (kg/ha)
    double mOtherWoodAbovegroundFrac; ///< fraction of other wood from aboveground
    double mTotalSnagCarbon; ///< sum of carbon content in all snag compartments (kg/ha)
    CNPair mTotalIn; ///< total input to the snag state (i.e. mortality/harvest and litter)
    CNPair mSWDtoSoil; ///< total flux from standing dead wood (book-keeping) -> soil (kg/ha)
    CNPair mTotalToAtm; ///< flux to atmosphere (kg/ha)
    CNPair mTotalToExtern; ///< total flux of masses removed from the site (i.e. harvesting) kg/ha
    CNPair mTotalToDisturbance; ///< fluxes due to disturbance
    static double mDBHLower, mDBHHigher; ///< thresholds used to classify to SWD-Pools
    static double mCarbonThreshold[3]; ///< carbon content thresholds that are used to decide if the SWD-pool should be emptied

    friend class Snapshot;
};

#endif // SNAG_H
