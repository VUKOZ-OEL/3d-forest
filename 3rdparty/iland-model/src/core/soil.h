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

#ifndef SOIL_H
#define SOIL_H

#include "snag.h"
struct SoilParams; // forward
class ResourceUnit; // forward
class SoilInputOut; // forward

class Soil
{
public:
    // lifecycle
    Soil(ResourceUnit *ru=0);
    /// set initial pool contents
    void setInitialState(const CNPool &young_labile_kg_ha, const CNPool &young_refractory_kg_ha, const CNPair &SOM_kg_ha, double young_labile_aboveground_frac, double young_refractory_aboveground_frac);

    // actions
    void setSoilInput(const CNPool &labile_input_kg_ha, const CNPool &refractory_input_kg_ha, double labile_aboveground_C, double refractory_aboveground_C); ///< provide values for input pools
    void setClimateFactor(const double climate_factor_re) { mRE = climate_factor_re; } ///< set the climate decomposition factor for the current year
    void newYear(); ///< reset of counters
    void calculateYear(); ///< main calculation function: calculates the update of state variables

    /// remove part of the biomass (e.g.: due to fire).
    /// @param DWDfrac fraction of downed woody debris (yR) to remove (0: nothing, 1: remove 100% percent)
    /// @param litterFrac fraction of litter pools (yL) to remove (0: nothing, 1: remove 100% percent)
    /// @param soilFrac fraction of soil pool (SOM) to remove (0: nothing, 1: remove 100% percent)
    void disturbance(double DWDfrac, double litterFrac, double soilFrac);
    /// remove biomass from the soil layer (e.g.: due to fire).
    /// @param DWD_kg_ha downed woody debris (yR) to remove kg/ha
    /// @param litter_kg_ha biomass in litter pools (yL) to remove kg/ha
    /// @param soil_kg_ha biomass in soil pool (SOM) to remove kg/ha
    void disturbanceBiomass(double DWD_kg_ha, double litter_kg_ha, double soil_kg_ha);

    // access
    const CNPool &youngLabile() const { return mYL;} ///< young labile matter (t/ha)
    const CNPool &youngRefractory() const { return mYR;} ///< young refractory matter (t/ha)
    const CNPair &oldOrganicMatter() const { return mSOM;} ///< old matter (SOM) (t/ha)
    double youngLabileAbovegroundFraction() const { return mYLaboveground_frac; } ///< fraction (0..1) of aboveground biomass in litter layer (yL), source is foliage
    double youngRefractoryAbovegroundFraction() const { return mYRaboveground_frac; } ///< fraction (0..1) of aboveground biomass in woody litter layer (yR), source=branches, stems
    double availableNitrogen() const { return mAvailableNitrogen; } ///< return available Nitrogen (kg/ha*yr)
    double totalCarbon() const; ///< total soil carbon t/ha (result is per ha, not the real area)

    const CNPair &fluxToAtmosphere() const { return mTotalToAtmosphere; } ///< total flux due to heterotrophic respiration kg/ha
    const CNPair &fluxToDisturbance() const { return mTotalToDisturbance; } ///< total flux due to disturbance events (e.g. fire) kg/ha

    QList<QVariant> debugList(); ///< return a debug output
private:
    ResourceUnit *mRU; ///< link to containing resource unit
    void fetchParameters(); ///< set iland parameters for soil
    static SoilParams *mParams; // static container for parameters
    // variables
    double mRE; ///< climate factor 're' (see Snag::calculateClimateFactors())
    double mAvailableNitrogen; ///< plant available nitrogen (kg/ha)
    double mAvailableNitrogenFromLabile; ///< plant available nitrogen from labile pool (kg/ha)
    double mAvailableNitrogenFromRefractory; ///< plant available nitrogen from refractory pool (kg/ha)
    double mKyl; ///< litter decomposition rate
    double mKyr; ///< downed woody debris (dwd) decomposition rate
    double mKo; ///< decomposition rate for soil organic matter (i.e. the "old" pool sensu ICBM)
    double mH; ///< humification rate

    CNPool mInputLab; ///< input pool labile matter (t/ha)
    CNPool mInputRef; ///< input pool refractory matter (t/ha)
    // state variables
    CNPool mYL; ///< C/N Pool for young labile matter (i.e. litter) (t/ha)
    CNPool mYR; ///< C/N Pool for young refractory matter (i.e. downed woody debris) (t/ha)
    CNPair mSOM; ///< C/N Pool for old matter (t/ha) (i.e. soil organic matter, SOM)
    // aboveground fraction (=fraction of the pool content for which the source is AG biomass (leafs, branches, stems)
    double mYLaboveground_frac; ///< aboveground fraction (0..1) of the litter pool (=leafs)
    double mYRaboveground_frac; ///< aboveground fraciton (0..1) of the refr. pool (all except coarse roots)

    CNPair mTotalToDisturbance; ///< book-keeping pool for heterotrophic respiration (kg/*ha)
    CNPair mTotalToAtmosphere; ///< book-keeping disturbance envents (fire) (kg/ha)

    static double mNitrogenDeposition; ///< annual nitrogen deposition (kg N/ha*yr)
    friend class Snapshot;
    friend class SoilInputOut;
};

#endif // SOIL_H
