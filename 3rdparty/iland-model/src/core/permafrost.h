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

#ifndef PERMAFROST_H
#define PERMAFROST_H

#include "layeredgrid.h"

class WaterCycle; // forward
struct ClimateDay; // forward
class ResourceUnit; // forward
class WaterOut; // forward

namespace Water {

/**
 * @brief The Permafrost class simulates the permafrost layer of a resource unit
 *
 * The module mechanistically simulates daily changes in depth to permafrost, annual SOL accumulation, and
 * links to / interact with vegetation processes (e.g., establishment). The module models processes
 * per resource unit and runs at daily resolution.
 *
 * Setup of data structures/parameters: see setup() (invoked from the setup of the WaterCycle in iLand)
 * During a simulation:
 * run() -> daily update of freeze depth / water availability
 * newYear() -> annual processes (e.g., moss dynamics) (invoked during the calculation of the water cycle)
 *
 * See iland-model.org/permafrost
 *
 *
 */
class Permafrost
{
public:
    Permafrost();
    ~Permafrost();
    void setup(WaterCycle *wc);
    void setFromSnapshot(double moss_biomss, double soil_temp, double depth_frozen, double water_frozen);

    //const SStats &stats() const { return stats; }

    /// start a new year
    void newYear();

    /// run the permafrost calculations for a given resource unit and day
    void run(const ClimateDay *clim_day);

    /// burn some of the life moss (kg / ha)
    void burnMoss(const double biomass_kg) { mMossBiomass = mMossBiomass - biomass_kg/cRUArea;
                                           mMossBiomass = std::max(mMossBiomass, cMinMossBiomass); }
    /// add permafrost related debug output to the list 'out'
    void debugData(DebugList &out);

    // access to values
    /// thickness of the moss layer in meters
    double mossLayerThickness() const { return mMossBiomass / mosspar.bulk_density; } // kg/m2  / rho [kg/m3] = m
    /// thickness of the soil organic layer (in meters)
    double SOLLayerThickness() const {return mSOLDepth; }
    /// moss biomass (kg/m2)
    double mossBiomass() const { return mMossBiomass; }
    /// temperature deep below the surface (updated annually)
    double groundBaseTemperature() const { return mGroundBaseTemperature; }
    /// depth (m) at where below the soil is frozen (at the end of the year)
    double depthFrozen() const { return mCurrentSoilFrozen; }
    /// amount of water (mm) that is trapped in ice (at the end of the year)
    double waterFrozen() const { return mCurrentWaterFrozen; }

private:
    const double cMinMossBiomass = 0.0001; // kg/m2
    /// setup function of the moss layer on the resource unit
    void setupMossLayer();
    /// annual calculations for the moss layer
    void calculateMoss();


    /// setup of thermal properties of the soil on RU
    void setupThermalConductivity();

    /// thermal conductivity of the mineral soil [W / m2 / K]
    double thermalConductivity(bool from_below) const;
    double thermalConductivityFrozen() const;

    struct FTResult {
        FTResult(): delta_mm(0.), delta_soil(0.), new_depth(0.), orig_depth(0.) {}
        double delta_mm; // change of water (mm within iLand water bucket), freezing is negative
        double delta_soil; // change of ice layer (m) (within iLand water bucket), freezing is negative
        double new_depth; // final depth (m)
        double orig_depth;  // starting depth (m)
    };
    FTResult calcFreezeThaw(double at, double temp, bool lowerIceEdge, bool fromAbove);
    FTResult mResult; ///< keep the results of the last run for debug output

    WaterCycle *mWC;
    /// depth of soil (m) that is currently frozen (this is a part of the soil plant accessible soil)
    double mCurrentSoilFrozen;
    /// amount of water (mm) trapped currently in ice (in mCurrentSoilFrozen)
    double mCurrentWaterFrozen;
    /// soil depth (m) of iLand
    double mSoilDepth;
    double mPWP; ///< permanent wilting point iLand of the full soil column (mm)
    double mFC; ///< field capacity iLand of the full soil column (mm)
    /// top of frozen layer (m) when thawing (above that soil is thawed)
    double mTop;
    bool mTopFrozen; ///< is the top of the soil frozen?
    /// bottom of the frozen layer (m) (important for seasonal permafrost; soil is frozen *up to* this depth)
    /// for permanent permafrost, the bottom is infinite?
    double mBottom;
    double mFreezeBack; ///< depth (m) up to which the soil is frozen again (in autumn)

    double mSOLDepth; ///< depth of the soil organic layer on top of the mineral soil (m)

    double mGroundBaseTemperature; ///< temperature (C) of the zone with secondary heat flux

    /// thermal conductivity for dry (unfrozen) soil W/m/K
    double mKdry;
    /// thermal conductivity for saturated (unfrozen) soil W/m/K
    double mKsat;
    /// thermal conductivity for saturated and frozen soil W/m/K
    double mKice;
    bool mSoilIsCoarse; ///< switch based on sand content (used for calc. of thermal conductivity)

    // moss related variables
    double mMossBiomass; ///< moss biomass in kg/m2
    // stats
    struct SStats {
        void reset() { maxSnowDepth=0.; daysSnowCover=0; maxFreezeDepth=0.; maxThawDepth=0.; }
        double maxSnowDepth; ///< maxium snow depth (m) of a year
        int daysSnowCover; ///< days of the year with snow cover
        double maxFreezeDepth; ///< maximum depth of frozen soil (m)
        double maxThawDepth; ///< maximum depth of thawed soil (m)
        double mossFLight;  ///< value of fLight (-)
        double mossFDecid;  ///< value of fDecid (-)
    };
    SStats stats;

    // parameters of the permafrost model are stored in a static struct
    // (and exist only once)
    struct SParam {
        void init() { EFusion = 0.333; // MJ/litre H20
                      maxPermafrostDepth = 2.; // 2m
                 }
        // physical constants
        double EFusion; ///< latent heat of fusion, i.e. energy requied to thaw/freeze 1l of water
        double maxPermafrostDepth; ///< maximum depth (m) of active layer

        double lambdaSnow; ///< thermal conductivity [W/m*K] of snow
        double lambdaOrganicLayer; ///< thermal conductivity [W/m*K] of the organic layer

        double organicLayerDensity; ///< density (kg/m3) of the organic layer
        double SOLDefaultDepth = 0.1; ///< 0.1m default depth of soil organic litter

        // parameters
        double groundBaseDepth; ///< depth (m) of the zone below the active layer from where the secondary heat flux originates
        double maxFreezeThawPerDay; ///< maximum amount of freeze/thaw (mm watercolumn) per day

        bool onlySimulate; ///< if true, peramfrost has no effect on the available water (and soil depth)
    };

    // parameters of the moss submodel
    struct SMossParam {
        const double SLA=1.; ///< specific leaf area of moss (set to 1 m2/kg)
        const double AMax=0.3; ///< maximum moss productivity (0.3kg/m2/yr) (Foster et al 2019)
        double light_k; ///< light extinction koefficient used for canopy+moss layer
        double light_comp; ///< light compensation point (proportion of light level above canopy)
        double light_sat; ///< light saturation point (proportion of light level above canopy)
        double respiration_q, respiration_b; ///< parameter of moss respiration function: q: respiration, b: turnover
        double bulk_density; ///< density (kg/m3) of moss layer
        // carbon cycle
        double CNRatio; ///< carbon : nitrogen ratio of moss (used for litter input)
        double r_decomp; ///< decomposition rate of moss (compare KYL of species)
        double r_deciduous_inhibition; ///< factor for inhibiting effect of fresh broadleaved litter
    };

    static SParam par; ///< (static) parameters of the permafrost module
    static SMossParam mosspar; ///< (static) parameters of the moss module

    friend class PermafrostLayers;
    friend class ::WaterOut;
};

// Class for visualizing permafrost data in iLand
class PermafrostLayers: public LayeredGrid<ResourceUnit*> {
  public:
    ~PermafrostLayers() {}
    void setGrid(const Grid<ResourceUnit*> &grid) { mGrid = &grid; }
    void clearGrid() { mGrid = nullptr; }
    double value(ResourceUnit* const &data, const int index) const;
    const QVector<LayeredGridBase::LayerElement> &names();
    //bool onClick(const QPointF &world_coord) const;
private:
    QVector<LayeredGridBase::LayerElement> mNames;
};


} // end namespace

#endif // PERMAFROST_H
