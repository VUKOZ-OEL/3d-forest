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

#ifndef TREE_H
#define TREE_H
#include <QPointF>

#include "grid.h"


// mortality workshop 2015 / COST Action with H. Bugmann
//#define ALT_TREE_MORTALITY


// forwards
class Species;
class Stamp;
class ResourceUnit;
struct HeightGridValue;
struct TreeGrowthData;
class TreeOut;
class TreeRemovedOut;
class LandscapeRemovedOut;
class Saplings;
class ScriptTree;

class Tree
{
public:
    // lifecycle
    Tree();
    void setup(); ///< calculates initial values for biomass pools etc. after dimensions are set.

    // access to properties
    int id() const { return mId; } ///< numerical unique ID of the tree
    int age() const { return mAge; } ///< the tree age (years)
    /// metric coordinates of the tree
    const QPointF position() const { Q_ASSERT(mGrid!=0); return mGrid->cellCenterPoint(mPositionIndex); }

    /// @property positionIndex The tree does not store the floating point coordinates but only the index of pixel on the LIF grid
    const QPoint positionIndex() const { return mPositionIndex; } ///< the x/y indicies (2m grid) of the tree
    const Species* species() const { Q_ASSERT(mRU!=0); return mSpecies; } ///< pointer to the tree species of the tree.
    const ResourceUnit *ru() const { Q_ASSERT(mRU!=0); return mRU; } ///< pointer to the ressource unit the tree belongs to.

    // properties
    float dbh() const { return mDbh; } ///< dimater at breast height in cm
    float height() const { return mHeight; } ///< tree height in m
    float lightResourceIndex() const { return mLRI; } ///< LRI of the tree (updated during readStamp())
    float leafArea() const { return mLeafArea; } ///< leaf area (m2) of the tree
    double volume() const; ///< volume (m3) of stem volume based on geometry and density calculated on the fly.
    double basalArea() const; ///< basal area of the tree at breast height in m2
    bool isDead() const { return flag(Tree::TreeDead); } ///< returns true if the tree is already dead.
    float crownRadius() const; ///< fetch crown radius (m) from the attached stamp
    // biomass properties
    float biomassFoliage() const { return mFoliageMass; } ///< mass (kg) of foliage
    float biomassBranch() const { return mBranchMass; }  ///< mass (kg) of branches
    float biomassFineRoot() const { return mFineRootMass; } ///< mass (kg) of fine roots
    float biomassCoarseRoot() const { return mCoarseRootMass; } ///< mass (kg) of coarse roots
    float biomassStem() const { return mStemMass + mNPPReserve; } ///< mass (kg) of stem, conceputally stem biomass + reserve pool
    float biomassReserve() const { return mNPPReserve; } /// mass (kg) of the reserve pool
    double barkThickness() const; ///< thickness of the bark (cm)
    float stressIndex() const { return mStressIndex; } ///< the scalar stress rating (0..1)

    // actions
    enum TreeRemovalType { TreeDeath=0, TreeHarvest=1, TreeDisturbance=2, TreeSalavaged=3, TreeKilled=4, TreeCutDown=5};
    /// the tree dies (is killed)
    void die(TreeGrowthData *d=nullptr);
    /// remove the tree (management). removalFractions for tree compartments: if 0: all biomass stays in the system, 1: all is "removed"
    /// default values: all biomass remains in the forest (i.e.: kill()).
    void remove(double removeFoliage=0., double removeBranch=0., double removeStem=0. );
    /// remove the tree due to an special event (disturbance)
    /// the part of the biomass that goes not to soil/snags is removed (e.g. fire)
    /// @param stem_to_soil_fraction (0..1) of stem biomass that is routed to the soil
    /// @param stem_to_snag_fraction (0..1) of the stem biomass continues as standing dead
    /// @param branch_to_soil_fraction (0..1) of branch biomass that is routed to the soil
    /// @param branch_to_snag_fraction (0..1) of the branch biomass continues as standing dead
    /// @param foliage_to_soil_fraciton (0..1) fraction of biomass that goes directly to the soil. The rest (1.-fraction) is removed.
    void removeDisturbance(const double stem_to_soil_fraction, const double stem_to_snag_fraction,
                           const double branch_to_soil_fraction, const double branch_to_snag_fraction,
                           const double foliage_to_soil_fraction);

    void enableDebugging(const bool enable=true) {setFlag(Tree::TreeDebugging, enable); }
    /// removes fractions (0..1) for foliage, branches, stem, and roots from a tree, e.g. due to a fire.
    /// values of "0" remove nothing, "1" removes the full compartent.
    void removeBiomassOfTree(const double removeFoliageFraction, const double removeBranchFraction, const double removeStemFraction);
    /// remove root biomass of trees (e.g. due to funghi)
    void removeRootBiomass(const double removeFineRootFraction, const double removeCoarseRootFraction);

    // setters for initialization
    void setNewId() { mId = m_nextId++; } ///< force a new id for this object (after copying trees)
    void setId(const int id) { mId = id; } ///< set a spcific ID (if provided in stand init file).
    void setPosition(const QPointF pos) { Q_ASSERT(mGrid!=nullptr); mPositionIndex = mGrid->indexAt(pos); }
    void setPosition(const QPoint posIndex) { mPositionIndex = posIndex; }
    void setDbh(const float dbh) { mDbh=dbh; }
    void setHeight(const float height);
    void setSpecies(Species *ts) { mSpecies=ts; }
    void setRU(ResourceUnit *ru) { mRU = ru; }
    void setAge(const int age, const float treeheight);

    // management flags (used by ABE management system)
    void markForHarvest(bool do_mark) { setFlag(Tree::MarkForHarvest, do_mark);}
    bool isMarkedForHarvest() const { return flag(Tree::MarkForHarvest);}
    void markForCut(bool do_mark) { setFlag(Tree::MarkForCut, do_mark);}
    bool isMarkedForCut() const { return flag(Tree::MarkForCut);}
    void markCropTree(bool do_mark) { setFlag(Tree::MarkCropTree, do_mark);}
    bool isMarkedAsCropTree() const { return flag(Tree::MarkCropTree);}
    void markCropCompetitor(bool do_mark) { setFlag(Tree::MarkCropCompetitor, do_mark);}
    bool isMarkedAsCropCompetitor() const { return flag(Tree::MarkCropCompetitor);}
    // death reasons
    void setDeathReasonWind()  { setFlag(Tree::TreeDeadWind, true); }
    void setDeathReasonBarkBeetle()  { setFlag(Tree::TreeDeadBarkBeetle, true); }
    void setDeathReasonFire()  { setFlag(Tree::TreeDeadFire, true); }
    void setDeathCutdown()  { setFlag(Tree::TreeDeadKillAndDrop, true); }
    void setAffectedBite() { setFlag(Tree::TreeAffectedBite, true); }
    void setIsHarvested()  { setFlag(Tree::TreeHarvested, true); }

    bool isDeadWind() const { return flag(Tree::TreeDeadWind);}
    bool isDeadBarkBeetle() const { return flag(Tree::TreeDeadBarkBeetle);}
    bool isDeadFire() const { return flag(Tree::TreeDeadFire);}
    bool isAffectedBite() const { return flag(Tree::TreeAffectedBite); }
    bool isCutdown() const { return flag(Tree::TreeDeadKillAndDrop);}
    bool isHarvested() const { return flag(Tree::TreeHarvested);}

    // grid based light-concurrency functions
    void applyLIP(); ///< apply LightInfluencePattern onto the global grid
    void readLIF(); ///< calculate the lightResourceIndex with multiplicative approach
    void heightGrid(); ///< calculate the height grid

    void applyLIP_torus(); ///< apply LightInfluencePattern on a closed 1ha area
    void readLIF_torus(); ///< calculate LRI from a closed 1ha area
    void heightGrid_torus(); ///< calculate the height grid

    void calcLightResponse(); ///< calculate light response
    // growth, etc.
    void grow(); ///< main growth function to update the tree state.

    // static functions
    static void setGrid(FloatGrid* gridToStamp, Grid<HeightGridValue> *dominanceGrid);
    // statistics
    static void resetStatistics();
    static int statPrints() { return m_statPrint; }
    static int statCreated() { return m_statCreated; }
#ifdef ALT_TREE_MORTALITY
    static void mortalityParams(double dbh_inc_threshold, int stress_years, double stress_mort_prob);
#endif

    QString dump();
    void dumpList(QList<QVariant> &rTargetList);
    const Stamp *stamp() const { return mStamp; } ///< TODO: only for debugging purposes

private:
    // helping functions
    void partitioning(TreeGrowthData &d); ///< split NPP into various plant pools.
    double relative_height_growth(); ///< estimate height growth based on light status.
    void grow_diameter(TreeGrowthData &d); ///< actual growth of the tree's stem.
    void mortality(TreeGrowthData &d); ///< main function that checks whether trees is to die

#ifdef ALT_TREE_MORTALITY
    void altMortality(TreeGrowthData &d); ///< alternative version of the mortality sub module
#endif
    void notifyTreeRemoved(TreeRemovalType reason); ///< record the removed volume in the height grid

    // state variables
    int mId; ///< unique ID of tree
    int mAge; ///< age of tree in years
    float mDbh; ///< diameter at breast height [cm]
    float mHeight; ///< tree height [m]
    QPoint mPositionIndex; ///< index of the trees position on the basic LIF grid
    // biomass compartements
    float mLeafArea; ///< m2 leaf area
    float mOpacity; ///< multiplier on LIP weights, depending on leaf area status (opacity of the crown)
    float mFoliageMass; ///< kg of foliage (dry)
    float mStemMass; ///< kg biomass of aboveground stem biomass
    float mBranchMass; ///< kg biomass of branches
    float mFineRootMass; ///< kg biomass of fine roots (linked to foliage mass)
    float mCoarseRootMass; ///< kg biomass of coarse roots (allometric equation)
    // production relevant
    float mNPPReserve; ///< NPP reserve pool [kg] - stores a part of assimilates for use in less favorable years
    float mLRI; ///< resulting lightResourceIndex
    float mLightResponse; ///< light response used for distribution of biomass on RU level
    // auxiliary
    float mDbhDelta; ///< diameter growth [cm]
    float mStressIndex; ///< stress index (used for mortality)

    // Stamp, Species, Resource Unit
    const Stamp *mStamp;
    Species *mSpecies;
    ResourceUnit *mRU;

    // various flags
    int mFlags;
    /// (binary coded) tree flags
    enum Flags { TreeDead=1, TreeDebugging=2,
                 TreeDeadBarkBeetle=16, TreeDeadWind=32, TreeDeadFire=64, TreeDeadKillAndDrop=128, TreeHarvested=256,
                 MarkForCut=512, // mark tree for being cut down
                 MarkForHarvest=1024, // mark tree for being harvested
                 MarkCropTree=2048, // mark as crop tree
                 MarkCropCompetitor=4096, // mark as competitor for a crop tree
                 TreeAffectedBite=8192 // affected or killed by biotic disturbance module (BITE)
               };
    /// set a Flag 'flag' to the value 'value'.
    void setFlag(const Tree::Flags flag, const bool value) { if (value) mFlags |= flag; else mFlags &= (flag ^ 0xffffff );}
    /// set a number of flags (need to be constructed by or'ing flags together) at the same time to the Boolean value 'value'.
    void setFlag(const int flag, const bool value) { if (value) mFlags |= flag; else mFlags &= (flag ^ 0xffffff );}
    /// retrieve the value of the flag 'flag'.
    bool flag(const Tree::Flags flag) const { return mFlags & flag; }
    /// retrieve the flag value as a single integer
    int flags() const {return mFlags; }

    // special functions
    bool isDebugging() { return flag(Tree::TreeDebugging); }

    // static data
    static FloatGrid *mGrid;
    static Grid<HeightGridValue> *mHeightGrid;
    static TreeRemovedOut *mRemovalOutput;
    static void setTreeRemovalOutput(TreeRemovedOut *rout) { mRemovalOutput=rout; }
    static LandscapeRemovedOut *mLSRemovalOutput;
    static void setLandscapeRemovalOutput(LandscapeRemovedOut *rout) { mLSRemovalOutput=rout; }
    static Saplings *saps;

    // statistics
    static int m_statPrint;
    static int m_statAboveZ;
    static int m_statCreated;
    static int m_nextId;

    // friends
    friend class TreeWrapper;
    friend class StandStatistics;
    friend class TreeOut;
    friend class TreeRemovedOut;
    friend class LandscapeRemovedOut;
    friend class Snapshot;
    friend class SnapshotItem;
    friend class ScriptTree;
};

/// internal data structure which is passed between function and to statistics
struct TreeGrowthData
{
    double NPP; ///< total NPP (kg)
    double NPP_above; ///< NPP aboveground (kg) (NPP - fraction roots), no consideration of tree senescence
    double NPP_stem;  ///< NPP used for growth of stem (dbh,h)
    double stress_index; ///< stress index used for mortality calculation
    TreeGrowthData(): NPP(0.), NPP_above(0.), NPP_stem(0.), stress_index(0.) {}
};
#endif // TREE_H
