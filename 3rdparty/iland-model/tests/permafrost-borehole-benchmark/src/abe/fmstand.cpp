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

#include "abe_global.h"
#include "fmstand.h"

#include "fmunit.h"
#include "management.h"
#include "fmtreelist.h"
#include "forestmanagementengine.h"
#include "mapgrid.h"
#include "fmstp.h"
#include "scheduler.h"
#include "fomescript.h"
#include "agent.h"
#include "agenttype.h"

#include "tree.h"
#include "species.h"

#include "statdata.h"
#include "debugtimer.h"

namespace ABE {

/** @class FMStand
    @ingroup abe
    The FMStand class encapsulates forest stands which are defined as polygons. FMStand tracks properties of the stands (e.g. mean volume), and
    is a central player in the ABE system.


  */


FMStand::FMStand(FMUnit *unit, const int id)
{
    mUnit = unit;
    mId = id;
    mInitialId = id;
    mPhase = Activity::Invalid;

    // testing:
    mPhase = Activity::Tending;
    mStandType = 1; // just testing...

    mU = 0; mSpeciesCompositionIndex = -1; mThinningIntensityClass = -1;

    newRotatation();
    mSTP = nullptr;
    mVolume = 0.;
    mAge = 0.;
    mTotalBasalArea = 0.;
    mStems = 0.;
    mDbh = 0.;
    mHeight = 0.;
    mScheduledHarvest = 0.;
    mFinalHarvested = 0.;
    mThinningHarvest = 0.;
    mDisturbed = 0.;
    mSalvaged = 0.;
    mRotationStartYear = 0;
    mLastUpdate = -1.;
    mLastExecution = -1.;

    mCurrentIndex=-1;
    mLastExecutedIndex=-1;
    mLastRotationAge = -1;

    mArea = ForestManagementEngine::standGrid()->area(mId)/cRUArea;

}


void FMStand::initialize()
{
    if (!mSTP)
        throw IException(QString("FMStand::initialize, no valid STP for stand %1").arg(id()));
    // copy activity flags
    mStandFlags = mSTP->defaultFlags();
    mCurrentIndex=-1;
    mLastExecutedIndex=-1;
    mYearsToWait=0;
    mContextStr = QString("S%2Y%1:").arg(ForestManagementEngine::instance()->currentYear()).arg(id()); // initialize...

    // load data and aggregate averages
    reload();
    if (mRotationStartYear==0.) // only set if not explicitely set previously.
        mRotationStartYear = ForestManagementEngine::instance()->currentYear() - age();
    // when a stand is initialized, we assume that 20% of the standing volume
    // have been removed already.
    if (mRemovedVolumeTotal==0.) {

        if (absoluteAge()>0) {
            double factor = 0.;
            if (absoluteAge()>=20 && absoluteAge()<100)
                factor =  0.35*(absoluteAge()-20.)/80.; // linear ramp to 35% at age 100, spruce yield table 7
            if (absoluteAge()>=100)
                factor = 0.35 + 0.1*std::min((absoluteAge()-100.)/100., 1.); // linear from 35% to 45% (age 200), then linear onwards

            mRemovedVolumeTotal = volume() * factor;
            mMAItotal = volume() * (1. + factor) / absoluteAge();
       } else {
            mMAItotal = 0.;
        }

        mMAIdecade = mMAItotal;
        mLastMAIVolume = volume();
    }

    // find out the first activity...
    int min_years_to_wait = 100000;
    for (int i=0;i<mStandFlags.count(); ++i) {
        // run the onSetup event
        // specifically set 'i' as the activity to be evaluated.
        FomeScript::setExecutionContext(this);
        FomeScript::bridge()->activityObj()->setActivityIndex(i);
        mStandFlags[i].activity()->events().run(QStringLiteral("onSetup"), 0);

        if (!mStandFlags[i].enabled() || !mStandFlags[i].active())
            continue;
        // set active to false which have already passed
        if (!mStandFlags[i].activity()->isRepeatingActivity()) {
            if (!mStandFlags[i].activity()->schedule().absolute && mStandFlags[i].activity()->latestSchedule(U()) < absoluteAge()) {
                mStandFlags[i].setActive(false);
            } else {
                int delta = mStandFlags[i].activity()->earliestSchedule(U()) - absoluteAge();
                if (mStandFlags[i].activity()->schedule().absolute)
                    delta += absoluteAge(); // absolute timing: starting from 0

                if (delta<min_years_to_wait) {
                    min_years_to_wait = qMax(delta,0); // limit to 0 years
                    mCurrentIndex = i; // first activity to execute
                }
            }
        }
    }
    if (mCurrentIndex==-1) {
        // the stand is "outside" the time frames provided by the activities.
        // set the last activity with "force" = true as the active
        setToLatestForcedActivity();

    }

    if (min_years_to_wait<100000)
        sleep(min_years_to_wait);

    // call onInit handler on the level of the STP
    mSTP->events().run(QStringLiteral("onInit"), this);
    if (mCurrentIndex>-1) {
        mStandFlags[mCurrentIndex].activity()->events().run(QStringLiteral("onEnter"), this);

        // if it is a scheduled activity, then execute (to get initial estimates for harvests)
        if (currentFlags().isScheduled())
            executeActivity(currentActivity());
    }

}

void FMStand::reset(FMSTP *stp)
{
    mSTP = stp;
    newRotatation();
    mCurrentIndex = -1;
}

void FMStand::checkArea()
{
    mArea = ForestManagementEngine::standGrid()->area(mId)/cRUArea;
}

bool relBasalAreaIsHigher(const SSpeciesStand &a, const SSpeciesStand &b)
{
    return a.relBasalArea > b.relBasalArea;
}

void FMStand::reload(bool force)
{
    if (!force && mLastUpdate == ForestManagementEngine::instance()->currentYear())
        return;

    DebugTimer t("ABE:FMStand::reload");
    // load all trees that are located on this stand
    mTotalBasalArea = 0.;
    mVolume = 0.;
    mAge = 0.;
    mStems = 0.;
    mDbh = 0.;
    mHeight = 0.;
    mTopHeight = 0.;
    mLastUpdate = ForestManagementEngine::instance()->currentYear();
    mSpeciesData.clear();

    // load all trees of the forest stand (use the treelist of the current execution context)
    FMTreeList *trees = ForestManagementEngine::instance()->scriptBridge()->treesObj();
    trees->setStand(this);
    trees->loadAll();

    //qDebug() << "fmstand-reload: load trees from map:" << t.elapsed();
    // use: value_per_ha = value_stand * area_factor
    double area_factor = 1. / area();
    const QVector<QPair<Tree*, double> > &treelist = trees->trees();

    // calculate top-height: diameter of the 100 thickest trees per ha
    QVector<double> dbhvalues;
    dbhvalues.reserve(trees->trees().size());

    for ( QVector<QPair<Tree*, double> >::const_iterator it=treelist.constBegin(); it!=treelist.constEnd(); ++it)
        dbhvalues.push_back(it->first->dbh());

    double topheight_threshhold=0.;
    double topheight_height = 0.;
    int topheight_trees = 0;
    if (treelist.size()>0) {
        StatData s(dbhvalues);
        topheight_threshhold= s.percentile(static_cast<int>( 100.*(1.- area()*100./treelist.size()) ) ); // sorted ascending -> thick trees at the end of the list
    }
    for ( QVector<QPair<Tree*, double> >::const_iterator it=treelist.constBegin(); it!=treelist.constEnd(); ++it) {
        double ba = it->first->basalArea() * area_factor;
        mTotalBasalArea+=ba;
        mVolume += it->first->volume() * area_factor;
        mAge += it->first->age()*ba;
        mDbh += it->first->dbh()*ba;
        mHeight += it->first->height()*ba;
        mStems++;
        SSpeciesStand &sd = speciesData(it->first->species());
        sd.basalArea += ba;
        if (it->first->dbh() >= topheight_threshhold) {
            topheight_height += it->first->height();
            ++topheight_trees;
        }
    }
    if (mTotalBasalArea>0.) {
        mAge /= mTotalBasalArea;
        mDbh /= mTotalBasalArea;
        mHeight /= mTotalBasalArea;
        for (int i=0;i<mSpeciesData.count();++i) {
            mSpeciesData[i].relBasalArea =  mSpeciesData[i].basalArea / mTotalBasalArea;
        }
    }
    if (topheight_trees>0) {
        mTopHeight = topheight_height / double(topheight_trees);
    }
    mStems *= area_factor; // convert to stems/ha
    // sort species data by relative share....
    std::sort(mSpeciesData.begin(), mSpeciesData.end(), relBasalAreaIsHigher);
}

// return stand area in ha


double FMStand::absoluteAge() const
{
    return ForestManagementEngine::instance()->currentYear() - mRotationStartYear;
}


bool FMStand::execute()
{
    //  the age of the stand increases by one
    mAge++;

    // do nothing if we are still waiting (sleep)
    if (mYearsToWait>0) {
        if (--mYearsToWait > 0) {
            return false;
        }
    }

    mContextStr = QString("S%2Y%1:").arg(ForestManagementEngine::instance()->currentYear()).arg(id());


    // what to do if there is no active activity??
    if (mCurrentIndex==-1) {
        if (trace())
            qCDebug(abe) << context() << "*** No action - no currently active activity ***";
        return false;
    }
    if (trace())
        qCDebug(abe) << context() << "*** start evaluate activity:" << currentActivity()->name();

    // do nothing if there is already an activity in the scheduler
    if (currentFlags().isPending()) {
        if (trace())
            qCDebug(abe) << context() << "*** No action - stand in the scheduler. ***";
        return false;
    }

    // do nothing if the the current year is not within the window of opportunity of the activity
    double p_schedule = currentActivity()->scheduleProbability(this);
    if (p_schedule == -1.) {
        if (trace())
            qCDebug(abe)<< context()  << "*** Activity expired. ***";
        // cancel the activity
        currentFlags().setActive(false);
        afterExecution(true);
        return false;
    }
    if (p_schedule>=0. && p_schedule < 0.00001) {
        if (trace())
            qCDebug(abe)<< context()  << "*** No action - Schedule probability 0. ***";
        return false;
    }


    // we need to renew the stand data
    reload();


    // check if there are some constraints that prevent execution....
    double p_execute = currentActivity()->execeuteProbability(this);
    if (p_execute == 0.) {
        if (trace())
            qCDebug(abe)<< context() << "*** No action - Constraints preventing execution. ***";
        return false;
    }

    // ok, we should execute the current activity.
    // if it is not scheduled, it is executed immediately, otherwise a ticket is created.
    if (currentFlags().isScheduled()) {
        // ok, we schedule the current activity
        if (trace())
            qCDebug(abe)<< context() << "adding ticket for execution.";

        mScheduledHarvest = 0.;
        bool should_schedule = currentActivity()->evaluate(this);
        if (trace())
            qCDebug(abe) << context() << "evaluated stand. add a ticket:" << should_schedule;
        if (should_schedule) {
            mUnit->scheduler()->addTicket(this, &currentFlags(), p_schedule, p_execute );
        } else {
            // cancel the activity
            currentFlags().setActive(false);
            afterExecution(true);
        }
        return should_schedule;
    } else {
        // execute immediately
        if (trace())
            qCDebug(abe) << context() << "executing activty" << currentActivity()->name();
        mScheduledHarvest = 0.;
        bool executed = currentActivity()->execute(this);
        if (!currentActivity()) // special case: the activity invalidated the active activtity
            return executed;

        if (!currentActivity()->isRepeatingActivity()) {
            currentFlags().setActive(false);
            afterExecution(!executed); // check what comes next for the stand
        }
        return executed;
    }
}

bool FMStand::executeActivity(Activity *act)
{
    int old_activity_index = mCurrentIndex;

    int new_index = stp()->activityIndex(act);
    bool result=false;
    if (new_index>-1) {
        mCurrentIndex = new_index;
        int old_years = mYearsToWait;
        mYearsToWait = 0;
        result = execute();
        mAge--; // undo modification of age
        mYearsToWait = old_years; // undo...
    }
    mCurrentIndex = old_activity_index;
    return result;
}


bool FMStand::afterExecution(bool cancel)
{
    // check if an agent update is necessary
    unit()->agent()->type()->agentUpdateForStand(this, currentFlags().activity()->name(), -1);

    // is called after an activity has run
    int tmin = 10000000;
    int indexmin = -1;
    for (int i=0;i<mStandFlags.count(); ++i) {
        if (mStandFlags[i].isForcedNext()) {
            mStandFlags[i].setForceNext(false); // reset flag
            indexmin = i;
            break; // we "jump" to this activity
        }
    }

    if (indexmin == -1) {
        // check if a restart is needed
        // TODO: find a better way!!
        if (currentFlags().isFinalHarvest()) {
            // we have reached the last activity
            for (int i=0;i<mStandFlags.count(); ++i)
                if (mStandFlags[i].enabled())
                    mStandFlags[i].setActive(true); // set all activities as active which are enabled
            newRotatation();
            reload();
        }

        // look for the next (enabled) activity.
        for (int i=0;i<mStandFlags.count(); ++i) {
            if ( mStandFlags[i].enabled() && mStandFlags[i].active() && !mStandFlags[i].isRepeating())
                if (mStandFlags[i].activity()->earliestSchedule(U()) < tmin) {
                    tmin =  mStandFlags[i].activity()->earliestSchedule(U());
                    indexmin = i;
                }
        }
    }

    if (!cancel)
        currentActivity()->events().run(QStringLiteral("onExecuted"),this);
    else
        currentActivity()->events().run(QStringLiteral("onCancel"),this);

    if (indexmin != mCurrentIndex) {
        // call events:
        currentActivity()->events().run(QStringLiteral("onExit"), this);
        if (indexmin>-1 && indexmin<mStandFlags.count())
            mStandFlags[indexmin].activity()->events().run(QStringLiteral("onEnter"), this);

    }
    mLastExecutedIndex = mCurrentIndex;

    mCurrentIndex = indexmin;
    if (mCurrentIndex>-1) {
        int to_sleep = tmin - static_cast<int>(absoluteAge()) - 1;
        if (to_sleep>0)
            sleep(to_sleep);
    } else {
        // no next activity found...
        if (FMSTP::verbose())
            qCDebug(abe) << context() << "no activity found to execute next!";
    }
    mScheduledHarvest = 0.; // reset

    mLastExecution = ForestManagementEngine::instance()->currentYear();

    return mCurrentIndex > -1;
}

void FMStand::notifyTreeRemoval(Tree *tree, int reason)
{
    double removed_volume = tree->volume();
    mVolume -= removed_volume/area();

    // for MAI calculations: store removal regardless of the reason
    mRemovedVolumeDecade+=removed_volume / area();
    mRemovedVolumeTotal+=removed_volume / area();

    Tree::TreeRemovalType r = Tree::TreeRemovalType (reason);
    if (r == Tree::TreeDeath)
        return; // do nothing atm
    if (r==Tree::TreeHarvest) {
        // regular harvest
        if (currentActivity()) {
            if (currentFlags().isFinalHarvest())
                mFinalHarvested += removed_volume;
            else
                mThinningHarvest += removed_volume;
        }
    }
    if (r==Tree::TreeDisturbance) {
        // if we have an active salvage activity, then store
        mDisturbed += removed_volume;
        // check if we have an (active) salvage activity; both the activity flags and the stand flags need to be "enabled"
        if (mSTP->salvageActivity() && mSTP->salvageActivity()->standFlags().enabled() && mSTP->salvageActivity()->standFlags(this).enabled() ) {
            if (mSTP->salvageActivity()->evaluateRemove(tree)) {
                mSalvaged += removed_volume;
                tree->setIsHarvested(); // set the flag that the tree is removed from the forest
                // the last executed activity is the salvage activity...
                mLastExecutedIndex = mSTP->salvageActivity()->index();
                mLastExecution = ForestManagementEngine::instance()->currentYear();
            }
        }

    }
}

bool FMStand::notifyBarkBeetleAttack(double generations, int infested_px_per_ha)
{
    // check if we have an (active) salvage activity; both the activity flags and the stand flags need to be "enabled"
    if (mSTP->salvageActivity() && mSTP->salvageActivity()->standFlags().enabled() && mSTP->salvageActivity()->standFlags(this).enabled()) {
        return mSTP->salvageActivity()->barkbeetleAttack(this, generations, infested_px_per_ha);
    }
    return false;
}


void FMStand::sleep(int years_to_sleep)
{
    mYearsToWait = qMax(mYearsToWait, qMax(years_to_sleep,0));
}


double FMStand::calculateMAI()
{
    // MAI: delta standing volume + removed volume, per year
    // removed volume: mortality, management, disturbances
    mMAIdecade = ((mVolume - mLastMAIVolume) + mRemovedVolumeDecade) / 10.;
    if (absoluteAge()>0)
        mMAItotal = (mVolume + mRemovedVolumeTotal) / absoluteAge();

    mLastMAIVolume = mVolume;
    // reset counters
    mRemovedVolumeDecade = 0.;
    return meanAnnualIncrementTotal();
}

double FMStand::basalArea(const QString &species_id) const
{
    foreach (const SSpeciesStand &sd, mSpeciesData)
        if (sd.species->id()==species_id)
            return sd.basalArea;
    return 0.;
}

double FMStand::relBasalArea(const QString &species_id) const
{
    foreach (const SSpeciesStand &sd, mSpeciesData)
        if (sd.species->id()==species_id)
            return sd.relBasalArea;
    return 0.;
}

void FMStand::setAbsoluteAge(const double age)
{
    mRotationStartYear = ForestManagementEngine::instance()->currentYear() - age;
    mAge = age;
}

int FMStand::setToLatestForcedActivity()
{
    // the stand is "outside" the time frames provided by the activities.
    // set the last activity with "force" = true as the active
    int i;
    for (i=mStandFlags.count()-1;i>=0; --i)
        if (mStandFlags[i].enabled() && mStandFlags[i].activity()->schedule().force_execution==true) {
            mCurrentIndex = i;
            break;
        }
    if (mCurrentIndex<0)
        qCDebug(abe) << context() << "Warning: setToLatestForcedActivity(): no valid activity found!";
    return i;

}

// storage for properties (static)
QHash<const FMStand*, QHash<QString, QJSValue> > FMStand::mStandPropertyStorage;


void FMStand::setProperty(const QString &name, QJSValue value)
{
    // save a property value for the current stand
    mStandPropertyStorage[this][name] = value;
}

QJSValue FMStand::property(const QString &name) const
{
    // check if values are already stored for the current stand
    if (!mStandPropertyStorage.contains(this))
        return QJSValue();
    // check if something is stored for the property name (return a undefined value if not)
    if (!mStandPropertyStorage[this].contains(name))
        return QJSValue();
    return mStandPropertyStorage[this][name];
}

QStringList FMStand::info()
{
    QStringList lines;
    lines << QString("id: %1").arg(id())
          << QString("unit: %1").arg(unit()->id());
    lines  << "-" << unit()->info() << "/-"; // sub sections
    if (currentActivity()) {
        lines << QString("activity: %1").arg(currentActivity()->name()) << "-" << currentActivity()->info();
        // activity properties
        lines << QString("active: %1").arg(currentFlags().active());
        lines << QString("enabled: %1").arg(currentFlags().enabled());
        lines << QString("simulate: %1").arg(currentFlags().isDoSimulate());
        lines << QString("execute immediate: %1").arg(currentFlags().isExecuteImmediate());
        lines << QString("final harvest: %1").arg(currentFlags().isFinalHarvest());
        lines << QString("use scheduler: %1").arg(currentFlags().isScheduled());
        lines << QString("in scheduler: %1").arg(currentFlags().isPending());
        lines <<  "/-";
    }
    lines << QString("agent: %1").arg(unit()->agent()->type()->name());
    lines << QString("STP: %1").arg(stp()?stp()->name():QStringLiteral("-"));
    lines << QString("U (yrs): %1").arg(U());
    lines << QString("thinning int.: %1").arg(thinningIntensity());
    lines << QString("last update: %1").arg(lastUpdate());
    lines << QString("sleep (years): %1").arg(sleepYears());
    lines << QString("scheduled harvest: %1").arg(scheduledHarvest());
    lines << QString("basal area: %1").arg(basalArea());
    lines << QString("volume: %1").arg(volume());
    lines << QString("age: %1").arg(age());
    lines << QString("absolute age: %1").arg(absoluteAge());
    lines << QString("N/ha: %1").arg(stems());
    lines << QString("MAI (decadal) m3/ha*yr: %1").arg(meanAnnualIncrement());
    lines << "Basal area per species";
    for (int i=0;i<nspecies();++i) {
        lines << QString("%1: %2").arg(speciesData(i).species->id()).arg(speciesData(i).basalArea);
    }

    lines  << "All activities" << "-";
    for (QVector<ActivityFlags>::const_iterator it = mStandFlags.constBegin(); it!=mStandFlags.constEnd(); ++it) {
        const ActivityFlags &a = *it;
        lines << QString("%1 (active): %2").arg(a.activity()->name()).arg(a.active())
                 << QString("%1 (enabled): %2").arg(a.activity()->name()).arg(a.enabled());
    }
    lines << "/-";


    // stand properties
    if (mStandPropertyStorage.contains(this)) {
        QHash<QString, QJSValue> &props = mStandPropertyStorage[this];
        lines << QString("properties: %1").arg(props.size()) << "-";
        QHash<QString, QJSValue>::const_iterator i = props.constBegin();
        while (i != props.constEnd()) {
            lines << QString("%1: %2").arg(i.key()).arg(i.value().toString());
            ++i;
        }
        lines << "/-";
    }

    // scheduler info
    lines  << "Scheduler" << "-";
    lines << unit()->constScheduler()->info(id());
    lines << "/-";

    return lines;
}

void FMStand::newRotatation()
{
    mLastRotationAge = absoluteAge();
    mRotationStartYear = ForestManagementEngine::instance()->currentYear(); // reset stand age to 0.
    mRemovedVolumeTotal = 0.;
    mRemovedVolumeDecade = 0.;
    mLastMAIVolume = 0.;
    mMAIdecade = 0.;
    mMAItotal = 0.;
    // use default values
    setU( unit()->U() );
    setThinningIntensity( unit()->thinningIntensity() );
    unit()->agent()->type()->agentUpdateForStand(this, QString(), 0); // update at age 0? maybe switch to new STP?

}

SSpeciesStand &FMStand::speciesData(const Species *species)
{
    for (int i=0;i<mSpeciesData.count(); ++i)
        if (mSpeciesData[i].species == species)
            return mSpeciesData[i];

    mSpeciesData.append(SSpeciesStand());
    mSpeciesData.last().species = species;
    return mSpeciesData.last();
}


} // namespace
