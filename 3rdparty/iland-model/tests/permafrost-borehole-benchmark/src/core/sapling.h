/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    http://iland.boku.ac.at
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

#ifndef SAPLING_H
#define SAPLING_H
#include <QtCore/QVector>
#include <QtCore/QPoint>
#include <bitset>
#include "grid.h"
#include "snag.h"
#include "model.h"
/// SaplingTreeOld holds information of a sapling (which represents N trees). Emphasis is on efficient storage.
class SaplingTreeOld {
public:
    SaplingTreeOld() { pixel=0; age.age=0; age.stress_years=0; height=0.05f; }
    bool isValid() const {return pixel!=0; }
    float *pixel; // pointer to the lifpixel the sapling lives on, set to 0 if sapling died/removed
    QPoint coords() const { return GlobalSettings::instance()->model()->grid()->indexOf(pixel); }
    struct  { // packed two 16bit to a 32 bit integer
        short unsigned int age;  // number of consectuive years the sapling suffers from dire conditions
        short unsigned int stress_years; // (upper 16bits) + age of sapling (lower 16 bits)
    } age;
    float height; // height of the sapling in meter
private:
};

class ResourceUnitSpecies; // forward
class Species;

/// saplings from 5cm to 4m
class Sapling
{
public:
    // maintenance
    Sapling();
    void setup(ResourceUnitSpecies *masterRUS) { mRUS = masterRUS; }
    void cleanupStorage(); // maintenance operation - remove dead/recruited trees from vector
    void clearStatistics();
    void newYear() { clearStatistics(); }
    void clear() { mSaplingTrees.clear(); mSapBitset.reset(); }
    static void setRecruitmentVariation(const double variation) { mRecruitmentVariation = variation; }
    static void updateBrowsingPressure();

    // access
    const QVector<SaplingTreeOld> &saplings() const {return mSaplingTrees; }
    // actions
    void calculateGrowth(); ///< perform growth + mortality + recruitment of all saplings of this RU and species
    /// add a new sapling at 'pos_lif' (i.e. QPoint with LIF-coordiantes) and with (optionally) 'height' (m) and 'age' (years)
    /// Returns the index of the newly added sapling.
    int addSapling(const QPoint &pos_lif, const float height=0.05f, const int age=1);
    /// clear (either remove or kill) a specific sapling
    void clearSapling(SaplingTreeOld &tree, const bool remove);
    void clearSapling(int index, const bool remove);
    void clearSaplings(const QPoint &position); ///< clear  saplings on a given position (after recruitment)
    void clearSaplings(const QRectF &rectangle, const bool remove_biomass); ///< clear  saplings within a given rectangle
    bool hasSapling(const QPoint &position) const; ///< return true if sapling is present at position
    double heightAt(const QPoint &position) const; ///< return the height at given position or 0 if position is not occupied
    // access to statistics
    int newSaplings() const { return mAdded; }
    int diedSaplings() const { return mDied; }
    int livingSaplings() const { return mLiving; } ///< get the number
    int recruitedSaplings() const { return mRecruited; }
    double livingStemNumber(double &rAvgDbh, double &rAvgHeight, double &rAvgAge) const; ///< returns the *represented* (Reineke's Law) number of trees (N/ha) and the mean dbh/height (cm/m)
    double averageHeight() const { return mAvgHeight; }
    double averageAge() const { return mAvgAge; }
    double averageDeltaHPot() const { return mAvgDeltaHPot; }
    double averageDeltaHRealized() const { return mAvgHRealized; }
    /// return the number of trees represented by one sapling of the current species and given 'height'
    double representedStemNumber(float height) const;
    // carbon and nitrogen
    const CNPair &carbonLiving() const { return mCarbonLiving; } ///< state of the living
    const CNPair &carbonGain() const { return mCarbonGain; } ///< state of the living
    // output maps
    void fillMaxHeightGrid(Grid<float> &grid) const;
    const std::bitset<cPxPerRU*cPxPerRU> &presentPositions() const { return mSapBitset; }
private:
    bool growSapling(SaplingTreeOld &tree, const double f_env_yr, Species* species);
    void setBit(const QPoint &pos_index, bool value);
    ResourceUnitSpecies *mRUS;
    QVector<SaplingTreeOld> mSaplingTrees;
    std::bitset<cPxPerRU*cPxPerRU> mSapBitset;
    int mAdded; ///< number of trees added
    int mRecruited; ///< number recruited (i.e. grown out of regeneration layer)
    int mDied; ///< number of trees died
    double mSumDbhDied; ///< running sum of dbh of died trees (used to calculate detritus)
    int mLiving; ///< number of trees (cohorts!!!) currently in the regeneration layer
    double mAvgHeight; ///< average height of saplings (m)
    double mAvgAge; ///< average age of saplings (years)
    double mAvgDeltaHPot; ///< average height increment potential (m)
    double mAvgHRealized; ///< average realized height increment
    static double mRecruitmentVariation; ///< defines range of random variation for recruited trees
    static double mBrowsingPressure; ///< scalar for the browsing pressure
    CNPair mCarbonLiving;
    CNPair mCarbonGain; ///< net growth (kg / ru) of saplings


    friend class Snapshot;
};

#endif // SAPLING_H
