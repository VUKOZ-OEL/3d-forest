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

#ifndef RESOURCEUNIT_H
#define RESOURCEUNIT_H

#include "tree.h"
#include "resourceunitspecies.h"
#include "standstatistics.h"
#include <QtCore/QVector>
#include <QtCore/QRectF>
// forward declarations
class SpeciesSet;
class Climate;
class WaterCycle;
class Snag;
class Soil;
struct SaplingCell;
class Microclimate;
class SVDStates; class SVDStateOut;

struct ResourceUnitVariables
{
    ResourceUnitVariables(): nitrogenAvailable(0.), cumCarbonUptake(0.), cumCarbonToAtm(0.), cumNEP(0.), carbonUptake(0.), carbonToAtm(0.), NEP(0.) { nitrogenAvailableDelta=0.; }
    double nitrogenAvailable; ///< nitrogen content (kg/m2/year)
    double cumCarbonUptake; ///< NPP  (kg C/ha)
    double cumCarbonToAtm; ///< total flux of carbon to atmosphere (kg C/ha)
    double cumNEP; ///< cumulative ecosystem productivity (kg C/ha), i.e. cumulative(NPP-losses(atm,harvest)
    double carbonUptake, carbonToAtm, NEP; ///< values of the current year (NPP, flux to atmosphere, net ecosystem prod., all values in kgC/ha)
    static double nitrogenAvailableDelta; ///< delta which is added to nitrogenAvailable every year (and can be changed via Time Events)
};

class ResourceUnit
{
public:
    ResourceUnit(const int index);
    ~ResourceUnit();
    // setup/maintenance
    void setup(); ///< setup operations after the creation of the model space.
    void setSpeciesSet(SpeciesSet *set);
    void setClimate(Climate* climate) { mClimate = climate; }
    void setBoundingBox(const QRectF &bb);
    void setID(const int id) { mID = id; }

    // data types
    /// potential sources of disturbance used for SVD state tracking
    enum ERUDisturbanceType { dtFire, dtBarkBeetle, dtWind, dtBite, dtAbe, dtManagement };


    // access to elements
    const Climate *climate() const { return mClimate; } ///< link to the climate on this resource unit
    SpeciesSet *speciesSet() const { return  mSpeciesSet; } ///< get SpeciesSet this RU links to.
    const WaterCycle *waterCycle() const { return mWater; } ///< water model of the unit
    Snag *snag() const { return mSnag; } ///< access the snag object
    Soil *soil() const { return mSoil; } ///< access the soil model
    SaplingCell *saplingCellArray() const { return mSaplings; } ///< access the array of sapling-cells
    SaplingCell *saplingCell(const QPoint &lifCoords) const; ///< return a pointer to the 2x2m SaplingCell located at 'lifCoords'
    /// return the area (m2) which is covered by saplings (cells >0 saplings)
    /// if  `below130cm` is false, then only pixels with saplings >1.3m are counted; otherwise
    /// pixel are counted when saplings < 1.3m are present or when grass cover is present
    double saplingCoveredArea(bool below130cm) const;

    ResourceUnitSpecies &resourceUnitSpecies(const Species *species); ///< get RU-Species-container of @p species from the RU
    const ResourceUnitSpecies *constResourceUnitSpecies(const Species *species) const; ///< get RU-Species-container of @p species from the RU
    ResourceUnitSpecies *resourceUnitSpecies(const int species_index) const { return mRUSpecies[species_index]; } ///< get RU-Species-container with index 'species_index' from the RU
    const QList<ResourceUnitSpecies*> &ruSpecies() const { return mRUSpecies; }
    QVector<Tree> &trees() { return mTrees; } ///< reference to the tree list.
    const QVector<Tree> &constTrees() const { return mTrees; } ///< reference to the (const) tree list.
    Tree *tree(const int index) { return &(mTrees[index]);} ///< get pointer to a tree
    const ResourceUnitVariables &resouceUnitVariables() const { return mUnitVariables; } ///< access to variables that are specific to resourceUnit (e.g. nitrogenAvailable)
    const StandStatistics &statistics() const {return mStatistics; }
    const Microclimate *microClimate() const { return mMicroclimate; }

    // properties
    int index() const { return mIndex; }
    int id() const { return mID; }
    const QRectF &boundingBox() const { return mBoundingBox; } ///< bounding box (metric) of the RU
    const QPoint &cornerPointOffset() const { return mCornerOffset; } ///< coordinates on the LIF grid of the upper left corner of the RU
    double area() const { return mPixelCount*100; } ///< get the resource unit area in m2
    double stockedArea() const { return mStockedArea; } ///< get the stocked area in m2
    double stockableArea() const { return mStockableArea; } ///< total stockable area in m2
    double productiveArea() const { return mEffectiveArea; } ///< TotalArea - Unstocked Area - loss due to BeerLambert (m2)
    /// Total Leaf Area Index (m2/m2) of trees>4m
    double leafAreaIndex() const { return stockableArea()?mAggregatedLA / stockableArea():0.; }
    double leafArea() const { return mAggregatedLA; } ///< total leaf area of resource unit (m2)
    double interceptedArea(const double LA, const double LightResponse) { return mEffectiveArea_perWLA * LA * LightResponse; }
    const double &LRImodifier() const { return mLRI_modification; }
    double averageAging() const { return mAverageAging; } ///< leaf area weighted average aging
    /// calculate the top tree height (as 90th percentile of the top heights on the 10m pixels), 'rIrregular' is set to true when 50% of the area < 50% of topheight
    double topHeight(bool &rIrregular) const;
    /// the Id of the state the resource unit is in
    int svdStateId() const { return mSVDState.stateId; }
    /// the Id of the state the resource unit was previously in
    int svdPreviousStateId() const { return mSVDState.previousStateId; }
    /// the number of years the RU is already in the current state svdStateId()
    int svdStateTime() const {return mSVDState.time; }
    /// the number of years that the RU was in the previous state svdPreviousStateId()
    int svdPreviousTime() const {return mSVDState.previousTime; }
    /// notify a disturbance/management related activity happened on the resource unit
    /// this information is used for SVD states / context for state changes
    /// source: process caused activity (module), info: addition information, e.g. % killed trees
    void notifyDisturbance(ERUDisturbanceType source, double info) const;

    // actions
    Tree &newTree();  ///< returns a modifiable reference to a free space inside the tree-vector. should be used for tree-init.
    int newTreeIndex(); ///< returns the index of a newly inserted tree
    void cleanTreeList(); ///< remove dead trees from the tree storage.
    void treeDied() { mHasDeadTrees = true; } ///< sets the flag that indicates that the resource unit contains dead trees
    bool hasDiedTrees() const { return mHasDeadTrees; } ///< if true, the resource unit has dead trees and needs maybe some cleanup
    /// addWLA() is called by each tree to aggregate the total weighted leaf area on a unit
    void addWLA(const float LA, const float LRI) { mAggregatedWLA += LA*LRI; mAggregatedLA += LA; }
    void addLR(const float LA, const float LightResponse) { mAggregatedLR += LA*LightResponse; }
    /// function that distributes effective interception area according to the weight of Light response and LeafArea of the indivudal (@sa production())
    void calculateInterceptedArea();
    void addTreeAging(const double leaf_area, const double aging_factor) { mAverageAging += leaf_area*aging_factor; } ///< aggregate the tree aging values (weighted by leaf area)
    void addTreeAgingForAllTrees(); ///< calculate average tree aging for all trees of a RU. Used directly after stand initialization.
    // stocked area calculation
    void countStockedPixel(bool pixelIsStocked) { mPixelCount++; if (pixelIsStocked) mStockedPixelCount++; }
    void createStandStatistics(); ///< helping function to create an initial state for stand statistics
    void recreateStandStatistics(bool recalculate_stats); ///< re-build stand statistics after some change happened to the resource unit
    void setStockableArea(const double area) { mStockableArea = area; } ///< set stockable area (m2)
    void setCreateDebugOutput(const bool do_dbg) { mCreateDebugOutput = do_dbg; } ///< enable/disable output generation for RU
    bool shouldCreateDebugOutput() const { return mCreateDebugOutput; } ///< is debug output enabled for the RU?

    void analyzeMicroclimate(); ///< run vegetation analysis for microclimate

    // snag / snag dynamics
    // snag dynamics, soil carbon and nitrogen cycle
    void snagNewYear() { if (snag()) snag()->newYear(); } ///< clean transfer pools
    void calculateCarbonCycle(); ///< calculate snag dynamics at the end of a year
    // model flow
    void newYear(); ///< reset values for a new simulation year
    // LIP/LIF-cylcle -> Model
    void production(); ///< called after the LIP/LIF calc, before growth of individual trees. Production (3PG), Water-cycle
    void beforeGrow(); ///< called before growth of individuals
    // the growth of individuals -> Model
    void afterGrow(); ///< called after the growth of individuals
    void yearEnd(); ///< called at the end of a year (after regeneration??)

private:
    void updateSVDState(); ///< (if enabled) update the state of the RU
    int mIndex; ///< internal index
    int mID; ///< ID provided by external stand grid
    bool mHasDeadTrees; ///< flag that indicates if currently dead trees are in the tree list
    Climate *mClimate; ///< pointer to the climate object of this RU
    SpeciesSet *mSpeciesSet; ///< pointer to the species set for this RU
    WaterCycle *mWater; ///< link to the Soil water calculation engine
    Snag *mSnag; ///< ptr to snag storage / dynamics
    Soil *mSoil; ///< ptr to CN dynamics soil submodel
    QList<ResourceUnitSpecies*> mRUSpecies; ///< data for this ressource unit per species
    QVector<Tree> mTrees; ///< storage container for tree individuals
    SaplingCell *mSaplings; ///< pointer to the array of Sapling-cells for the resource unit
    Microclimate *mMicroclimate; ///< pointer to the microclimate-array
    QRectF mBoundingBox; ///< bounding box (metric) of the RU
    QPoint mCornerOffset; ///< coordinates on the LIF grid of the upper left corner of the RU
    double mAggregatedLA; ///< sum of leafArea
    double mAggregatedWLA; ///< sum of lightResponse * LeafArea for all trees
    double mAggregatedLR; ///< sum of lightresponse*LA of the current unit
    double mEffectiveArea; ///< total "effective" area per resource unit, i.e. area of RU - non-stocked - beerLambert-loss
    double mEffectiveArea_perWLA; ///<
    double mLRI_modification;
    double mAverageAging; ///< leaf-area weighted average aging f this species on this RU.
    float *mSaplingHeightMap; ///< pointer to array that holds max-height for each 2x2m pixel. Note: this information is not persistent
    struct RUSVDState {
        RUSVDState(): localComposition(0), midDistanceComposition(0), disturbanceEvents(0) {}
        ~RUSVDState() { if (localComposition) {delete localComposition; delete midDistanceComposition; delete disturbanceEvents; } }
        qint16 stateId; ///< the Id of the state the resource unit is in
        qint16 previousStateId; ///< the Id of the state the resource unit was previously in
        qint16 time; ///< the number of years the RU is in state 'stateId'
        qint16 previousTime; ///< the number of years that the RU was in the previous state
        QVector<float> *localComposition; ///< save for each species the relative share in the moore-neighborhood
        QVector<float> *midDistanceComposition; ///< save for each species the relative share in a regional neighborhood (200-300m)
        /// structure to store individual disturbance events
        struct SVDDisturbanceEvent {
            SVDDisturbanceEvent(int myear, ERUDisturbanceType msrc, double minfo): year(myear), source(msrc), info(minfo){}
            int year;
            ERUDisturbanceType source;
            double info;
        };
        QVector<SVDDisturbanceEvent> *disturbanceEvents;

        void clear() { stateId=previousStateId=time=previousTime=0; }
    } mSVDState;


    int mPixelCount; ///< count of (Heightgrid) pixels thare are inside the RU
    int mStockedPixelCount;  ///< count of pixels that are stocked with trees
    double mStockedArea; ///< size of stocked area
    double mStockableArea; ///< area of stockable area (defined by project setup)
    StandStatistics mStatistics; ///< aggregate values on stand value
    ResourceUnitVariables mUnitVariables;
    bool mCreateDebugOutput; ///< should debug output be created for the RU?

    friend class RUWrapper;
    friend class SVDStates;
    friend class SVDStateOut;
    friend class SVDIndicatorOut;
};


#endif // RESOURCEUNIT_H
