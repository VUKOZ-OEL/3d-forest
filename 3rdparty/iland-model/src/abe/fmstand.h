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
#ifndef FMSTAND_H
#define FMSTAND_H

#include <QHash>
#include <QJSValue>

#include "activity.h"

class Species; // forward (iLand species)
class Tree; // forward (iLand tree)
//enum TreeRemovalType; // forward

namespace ABE {

class FOMEWrapper; // forward
class FMUnit; // forward
class Patches; // forward

struct SSpeciesStand
{
    SSpeciesStand(): species(0), basalArea(0.), relBasalArea(0.) {}
    const Species *species; ///< the ID of the species (ie a pointer)
    double basalArea; ///< basal area m2
    double relBasalArea; ///< fraction [0..1] fraction of species based on basal area.
};

/** FMStand encapsulates a forest stand for the forest management engine.
 *  The spatial coverage is defined by a "stand grid".
 * */

class FMStand
{
public:
    /// c'tor: link stand to a forest management unit
    FMStand(FMUnit *unit, const int id);
    ~FMStand();
    /// set the stand to be managed by a given 'stp'
    void setSTP(FMSTP *stp) {mSTP = stp; }
    void initialize();
    /// sets the STP but nothing else (after disturbance related clearance)
    void reset(FMSTP *stp);
    /// returns true if tracing is enabled for the stand
    bool trace() const { return property(QStringLiteral("trace")).toBool(); }
    const QString &context() const { return mContextStr; }

    void checkArea();
    void setArea(const double new_area_ha) { mArea = new_area_ha; } // area in ha

    void reload(bool force=false); // fetch new data from the forest stand
    // general properties
    int id() const {return mId; }
    const FMUnit *unit() const { return mUnit; }
    Activity::Phase phase() const { return mPhase; }
    int standType() const { return mStandType; }
    FMSTP *stp() const {return mSTP; }
    int lastUpdate() const { return mLastUpdate; }
    int lastExecution() const { return mLastExecution; }
    int initialStandId() const { return mInitialId; }
    void setInitialId(int origin_id) { mInitialId = origin_id; }
    // agent properties
    /// rotation period (years)
    double U() const { return mU; }
    /// thinning intensity (class); 1: low, 2: medium, 3: high
    int thinningIntensity() const { return mThinningIntensityClass; }
    /// species composition key
    int targetSpeciesIndex() const { return mSpeciesCompositionIndex; }

    void setU(const double rotation_length) { mU = rotation_length; }
    void setThinningIntensity(const int th_class) { mThinningIntensityClass = th_class; }
    void setTargetSpeciesIndex(const int index) { mSpeciesCompositionIndex = index; }

    // access to the patches within the stand
    bool hasPatches() const { return mPatches != nullptr; }
    Patches *patches() const;

    // stand properties
    /// total area of the stand (ha)
    double area() const    {  return mArea; }
    /// absolute age: years since the rotation has started (years)
    double absoluteAge() const;
    /// total basal area (m2/ha)
    double basalArea() const {return mTotalBasalArea; }
    /// (average) age of the stand (weighted with basal area)
    double age() const {return mAge; }
    /// total standing volume (m3/ha) in the stand
    double volume() const {return mVolume; }
    /// number of trees of the stand (stems/ha) (>4m)
    double stems() const {return mStems; }
    /// mean dbh (basal area weighted, of trees>4m) in cm
    double dbh() const {return mDbh; }
    /// mean tree height (basal area weighted, of trees>4m), in m
    double height() const {return mHeight; }
    /// top height (mean height of the 100 thickest trees/ha), in m
    double topHeight() const {return mTopHeight; }
    /// scheduled harvest (planned harvest by activities, m3)
    double scheduledHarvest() const {return mScheduledHarvest; }
    /// total realized harvest (m3 on the full stand area)
    double totalHarvest() const { return mFinalHarvested + mThinningHarvest + mSalvaged; }
    /// total realized thinning/tending harvests (m3 on the full stand area)
    double totalThinningHarvest() const { return mThinningHarvest; }
    /// total disturbed timber volume, includes also disturbed trees *not* harvested, m3
    double disturbedTimber() const { return mDisturbed; }
    /// total amount of timber removed by salvage operation
    double salvagedTimber() const { return mSalvaged; }

    /// mean annual increment (MAI), m3 timber/ha for the last decade
    double meanAnnualIncrement() const { return mMAIdecade; }
    /// mean annual increment (MAI), m3 timber/ha for the full rotation period
    double meanAnnualIncrementTotal() const { return mMAItotal; }

    bool readyForFinalHarvest() {return absoluteAge()> 0.8*U(); } // { return currentActivity()?(currentFlags().isFinalHarvest() && currentFlags().isScheduled()):false; }

    // specialized functions (invokable also from javascript)
    double basalArea(const QString &species_id) const;
    double relBasalArea(const QString &species_id) const;

    int nspecies() const  { return mSpeciesData.count(); }
    /// retrieve species-specific meta data by index (0: largest basal area share, up to nspecies()-1)
    SSpeciesStand &speciesData(const int index) {return mSpeciesData[index]; }
    SSpeciesStand &speciesData(const Species *species); ///< species-specific meta data by Species pointer


    // actions
    /// main function
    bool execute(); ///< execute the current activity
    bool executeActivity(Activity *act); ///< execute activity given by "act".
    bool afterExecution(bool cancel = false);

    /// add a (simulated) harvest to the amount of planned harvest (used by the scheduling)
    void addScheduledHarvest(const double add_volume) {mScheduledHarvest += add_volume; }
    /// is called whenever a tree is removed (death, management, disturbance)
    void notifyTreeRemoval(Tree *tree, int reason);
    /// is called when bark beetles are likely to attack: return ABE changed forest structure
    bool notifyBarkBeetleAttack(double generations, int infested_px_per_ha);

    /// resets the harvest counters
    void resetHarvestCounter() { mFinalHarvested = 0.;  mThinningHarvest=0.;  }
    void resetDisturbanceSalvage() { mSalvaged=0.;  mDisturbed=0.; }

    /// sleep() pauses the evaluation/execution of management activities
    /// for 'years_to_sleep'.
    void sleep(int years_to_sleep, bool also_shorten=false);
    int sleepYears() const {return mYearsToWait; }
    /// stop sleeping = pausing eval/execuation of management activities
    void wakeUp() { mYearsToWait = 0; }

    /// calculate mean annual increment (m3/ha) and return total MAI.
    double calculateMAI();

    /// set the absolute age of the stand
    void setAbsoluteAge(const double age);

    /// set active activity to the latest activity with forced=true
    /// (if a stand is outside the timeframe of all other activities)
    int setToLatestForcedActivity();


    // return stand-specific flags
    ActivityFlags &flags(const int index)  {return mStandFlags[index]; }
    /// flags of currently active Activity
    ActivityFlags &currentFlags()  { return flags(mCurrentIndex); }

    /// get a pointer to the current activity; returns 0 if no activity is set.
    Activity *currentActivity() const { return mCurrentIndex>-1?mStandFlags[mCurrentIndex].activity():nullptr; }
    /// the index of the current activity
    int currentActivityIndex() const { return mCurrentIndex; }

    /// set a new activity index. No checks performed.
    void setActivityIndex(int index) { mCurrentIndex = index; }

    /// get a pointer to the last executed activity; returns 0 if no activity has been executed before.
    Activity *lastExecutedActivity() const { return mLastExecutedIndex>-1?mStandFlags[mLastExecutedIndex].activity():nullptr; }

    int lastExecutionAge() const { return absoluteAge()>0 ? static_cast<int>(absoluteAge()) : mLastRotationAge; }

    void setLastExecution(int index);
    // custom property storage
    static void clearAllProperties() { mStandPropertyStorage.clear(); }
    /// set a property value for the current stand with the name 'name'
    void setProperty(const QString &name, QJSValue value);
    /// retrieve the value of the property 'name'. Returns an empty QJSValue if the property is not defined.
    QJSValue property(const QString &name) const;

    /// general JS object of a stand
    QJSValue &JSobj() { return mJSObj; }
    QJSValue signalParameter() { return mJSSignalParameter; }
    void setSignalParameter(QJSValue value) { mJSSignalParameter = value; }

    // retrieve current state of the object
    QStringList info();
    friend class FOMEWrapper;
private:
    int mId; ///< the unique numeric ID of the stand
    FMUnit *mUnit; ///< management unit that
    FMSTP *mSTP; ///< the stand treatment program assigned to this stand
    Activity::Phase mPhase; ///< silvicultural phase
    int mInitialId; ///< stand-id that was assigned in the beginning (this Id is kept when stands are split)
    int mStandType; ///< enumeration of stand (compositional)
    double mArea; ///< total stand area (ha)
    double mTotalBasalArea; ///< basal area of the stand
    double mAge; ///< average age (yrs) of the stand (basal area weighted)
    double mVolume; ///< standing volume (m3/ha) of the stand
    double mStems; ///< stems per ha (above 4m)
    double mDbh; ///< mean dbh (basal area weighted, of trees>4m) in cm
    double mHeight; ///< mean tree height (basal area weighted, of trees>4m), in m
    double mTopHeight; ///< top height (mean height of the 100 thickest trees per ha)
    double mScheduledHarvest; ///< harvest (m3) that is scheduled by activities
    double mFinalHarvested; ///< m3 of timber volume that has been harvested (regeneration phase)
    double mThinningHarvest; ///< m3 of timber that was harvested for thinning/tending
    double mDisturbed; ///< affected by disturbance (m3)
    double mSalvaged; ///< m3 removed by salvage operation

    double mRemovedVolumeDecade; ///< removed volume of the decade (m3/ha)
    double mRemovedVolumeTotal; ///< removed volume of the rotation (m3/ha)

    double mLastMAIVolume; ///< safe the standing volume
    double mMAIdecade; ///< decadal mean annual increment (m3/ha*yr)
    double mMAItotal; ///< total (over the full rotation) mean annual increment (m3/ha*yr)


    int mRotationStartYear; ///< absolute year the current rotation has started
    int mYearsToWait; ///< variable indicates time to wait
    int mCurrentIndex; ///< the index of the current activity
    int mLastUpdate; ///< year of the last reload of data
    int mLastExecution; ///< year of the last execution of an activity
    int mLastExecutedIndex; ///< index of the last executed activity
    int mLastRotationAge; ///< age at which the last rotation ended

    double mU; ///< rotation length
    int mSpeciesCompositionIndex; ///< index of the active target species composition
    int mThinningIntensityClass; ///< currently active thinning intensity level

    void newRotatation(); ///< reset

    // storage for stand meta data (species level)
    QVector<SSpeciesStand> mSpeciesData;
    // storage for stand-specific management properties
    QVector<ActivityFlags> mStandFlags;
    // additional property values for each stand
    QString mContextStr;
    // access to patches
    Patches *mPatches;
    static QHash<const FMStand*, QHash<QString, QJSValue> > mStandPropertyStorage;
    QJSValue mJSObj;
    QJSValue mJSSignalParameter;

    friend class StandObj;
};


} // namespace
#endif // FMSTAND_H
