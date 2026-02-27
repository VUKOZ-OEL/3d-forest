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

#ifndef MODEL_H
#define MODEL_H
#include <QtCore>
#include <QtXml>

#include "global.h"

#include "grid.h"
#include "threadrunner.h"
#include "modelsettings.h"

// forward declarations
class ResourceUnit;
class SpeciesSet;
class Management;
class Saplings;

namespace ABE {
class ForestManagementEngine;
}
class Climate;
class Environment;
class TimeEvents;
class MapGrid;
class Modules;
class DEM;
class GrassCover;
class SVDStates;
namespace BITE { class BiteEngine; }

struct HeightGridValue
{
    float height; ///< dominant tree height (m)
    int count() const { return mCount & 0x0000ffff; } ///< get count of trees on pixel
    void increaseCount() { mCount++; } ///< increase the number of trees on pixel
    void resetCount() { mCount &= 0xffff0000; } ///< set the count to 0
    /// a value of 1: not valid (returns false).
    /// true: pixel is stockable and within the project area.
    /// false: pixel is not stockable (outside of the project area or a non-stockable pixel within the project area)
    bool isValid() const { return !isBitSet(mCount, 16); }
    void setValid(const bool valid) { setBit(mCount, 16, !valid); } ///< set bit to 1: pixel is not valid
    void setForestOutside(const bool is_outside) { setBit(mCount, 17, is_outside); }
    bool isForestOutside() const {return isBitSet(mCount, 17); } ///< true if a pixel is outside of the project area but considered as forested
    void setIsRadiating() { setBit(mCount, 18, true); } ///< bit 18: if set, the pixel is actively radiating influence on the LIF (such pixels are on the edge of "forestOutside")
    bool isRadiating() const { return isBitSet(mCount, 18); }
    /// get the (coarse, 1m classes) local height (only trees with the stem on the cell are counted here)
    float stemHeight() const { return static_cast<float> (mCount >> 20 & 0xff);}
    /// reset the stem height
    void clearStemHeight() { mCount &= 0xF00FFFFF; }
    /// set the height of the tree *local* (i.e. the stem of the tree is on the pixel); compare to the 'height': here also crowns can spread to neighboring cells.
    void setStemHeight(float h) {    unsigned int hval = static_cast<unsigned int>(h + 0.5f); // round: +0.5 + truncate
                                      unsigned int w = mCount;
                                      w &= 0xF00FFFFF;
                                      w |= hval << 20;
                                      mCount = w; }
    /// set values for height and count (this overwrites all the flags!)
    void init(const float aheight, const int acount) { height=aheight;mCount=static_cast<unsigned int>(acount); }
private:
    ///
    /// the lower 16 bits are to count, the heigher for flags.
    /// bit 16: valid (0=valid, 1=outside of project area)
    /// bit 17: is the pixel considered as forested (only for out of project area pixels)
    /// bit 18: pixel is "radiating" a LIF influence into the landscape
    /// bits 20-28 (8 bits): local height (1m resolution, 256m is max)
    quint32 mCount;  // 32 bit unsigned int (even on 64bit platforms)

};
typedef Grid<HeightGridValue> HeightGrid;

class Model
{
public:
    Model();
    ~Model();
    // start/stop/run
    void beforeRun(); ///< initializations
    void runYear(); ///< run a single year
    void afterStop(); ///< finish and cleanup

    // access to elements
    const ThreadRunner &threadExec() const {return threadRunner; }
    const QRectF &extent() const { return mModelRect; } ///< extent of the model (without buffer)
    double totalStockableArea() const { return mTotalStockableArea; } ///< total stockable area of the landscape (ha)
    ResourceUnit *ru() { return mRU.front(); }
    ResourceUnit *ru(QPointF coord); ///< ressource unit at given coordinates
    ResourceUnit *ru(int index) { return (index>=0&&index<mRU.count())? mRU[index] : NULL; } ///< get resource unit by index
    ResourceUnit *ruById(int id) const; ///< find the resource unit with Id 'id' or return NULL
    /// ruList contains all simulated resource units (that are in the project area)
    const QList<ResourceUnit*> &ruList() const {return mRU; }
    Management *management() const { return mManagement; }
    ABE::ForestManagementEngine *ABEngine() const { return mABEManagement; }
    BITE::BiteEngine *biteEngine() const { return mBiteEngine; }

    Environment *environment() const {return mEnvironment; }
    Saplings *saplings() const {return mSaplings; }
    TimeEvents *timeEvents() const { return mTimeEvents; }
    Modules *modules() const { return mModules; }
    const DEM *dem() const { return mDEM; }
    GrassCover *grassCover() const { return mGrassCover; }
    SpeciesSet *speciesSet() const { if (mSpeciesSets.count()==1) return mSpeciesSets.first(); return NULL; }
    const QList<Climate*> climates() const { return mClimates; }
    SVDStates *svdStates() const { return mSVDStates; }

    // global grids
    FloatGrid *grid() { return mGrid; } ///< this is the global 'LIF'-grid (light patterns) (currently 2x2m)
    HeightGrid *heightGrid() { return mHeightGrid; } ///< stores maximum heights of trees and some flags (currently 10x10m)
    const MapGrid *standGrid() { return mStandGrid; } ///< retrieve the spatial grid that defines the stands (10m resolution)
    const Grid<ResourceUnit*> &RUgrid() { return mRUmap; }
    /// get the value of the (10m) Height grid at the position index ix and iy (of the LIF grid)
    const HeightGridValue heightGridValue(const int ix, const int iy) const { return  mHeightGrid->constValueAtIndex(ix/cPxPerHeight, iy/cPxPerHeight); }
    const HeightGridValue &heightGridValue(const float *lif_ptr) const { QPoint p = mGrid->indexOf(lif_ptr); return mHeightGrid->constValueAtIndex(p.x()/cPxPerHeight, p.y()/cPxPerHeight); }

    // setup/maintenance
    void clear(); ///< free ressources
    void loadProject(); ///< setup and load a project
    bool isSetup() const { return mSetup; } ///< return true if the model world is correctly setup.
    static const ModelSettings &settings() {return mSettings;} ///< access to global model settings.
    static ModelSettings &changeSettings() {return mSettings;} ///< write access to global model settings.
    void onlyApplyLightPattern() { applyPattern(); readPattern(); }
    void reloadABE(); ///< force a recreate of the agent based forest management engine
    QString currentTask() const { return mCurrentTask; }
    void setCurrentTask(QString what) { mCurrentTask = what; }

    // actions
    /// build stand statistics (i.e. stats based on resource units)
    void createStandStatistics();
    /// clean the tree data structures (remove harvested trees) - call after management operations.
    void cleanTreeLists(bool recalculate_stats);
    /// execute a function for each resource unit using multiple threads. "funcptr" is a ptr to a simple function
    void executePerResourceUnit(void (*funcptr)(ResourceUnit*), const bool forceSingleThreaded=false) { threadRunner.run(funcptr, forceSingleThreaded);}

private:
    void initialize(); ///< basic startup without creating a simulation
    void setupSpace(); ///< setup the "world"(spatial grids, ...), create ressource units
    void initOutputDatabase(); ///< setup output database (run metadata, ...)

    void applyPattern(); ///< apply LIP-patterns of all trees
    void readPattern(); ///< retrieve LRI for trees
    void grow(); ///< grow - both on RU-level and tree-level

    void calculateStockedArea(); ///< calculate area stocked with trees for each RU
    void calculateStockableArea(); ///< calculate the stockable area for each RU (i.e.: with stand grid values <> -1)
    void initializeGrid(); ///< initialize the LIF grid

    void test();
    void debugCheckAllTrees();
    bool multithreading() const { return threadRunner.multithreading(); }
    ThreadRunner threadRunner;
    static ModelSettings mSettings;
    QString mCurrentTask;
    bool mSetup;
    /// container holding all ressource units
    QList<ResourceUnit*> mRU;
    /// grid specifying a map of ResourceUnits
    Grid<ResourceUnit*> mRUmap;
    /// container holding all species sets
    QList<SpeciesSet*> mSpeciesSets;
    /// container holding all the climate objects
    QList<Climate*> mClimates;
    //
    Modules *mModules; ///< the list of modules/plugins
    //
    QRectF mModelRect; ///< extent of the model (without buffer)
    double mTotalStockableArea; ///< total stockable area (ha)
    // global grids...
    FloatGrid *mGrid; ///< the main LIF grid of the model (2x2m resolution)
    HeightGrid *mHeightGrid; ///< grid with 10m resolution that stores maximum-heights, tree counts and some flags
    Saplings *mSaplings;
    Management *mManagement; ///< management sub-module (simple mode)
    ABE::ForestManagementEngine *mABEManagement; ///< management sub-module (agent based management engine)
    BITE::BiteEngine *mBiteEngine; ///< biotic disturbance module BITE
    Environment *mEnvironment; ///< definition of paramter values on resource unit level (modify the settings tree)
    TimeEvents *mTimeEvents; ///< sub module to handle predefined events in time (modifies the settings tree in time)
    MapGrid *mStandGrid; ///< map of the stand map (10m resolution)
    // Digital elevation model
    DEM *mDEM; ///< digital elevation model
    GrassCover *mGrassCover; ///< cover of the ground with grass / herbs
    /// SVD States
    /// collection of all realized SVD states in the model
    SVDStates *mSVDStates;
};

class Tree;
class AllTreeIterator
{
public:
    AllTreeIterator(Model* model): mModel(model), mTreeEnd(0),mCurrent(0) {}
    void reset() { mTreeEnd=0; mCurrent=0; }
    Tree *next();
    Tree *nextLiving();
    Tree *current() const;
    Tree *operator*() const { return current();  }
    ResourceUnit *currentRU() const { return *mRUIterator; }
private:
    Model *mModel;
    Tree *mTreeEnd;
    Tree *mCurrent;
    QList<ResourceUnit*>::const_iterator mRUIterator;
};
#endif // MODEL_H
