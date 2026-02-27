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

#include "global.h"
#include "abe_global.h"
#include "fomescript.h"

#include <QQmlEngine>

#include "forestmanagementengine.h"
#include "fmstp.h"
#include "agenttype.h"
#include "agent.h"
#include "fmtreelist.h"
#include "scheduler.h"
#include "fmstp.h"
#include "fmunit.h"
#include "patches.h"

#include "actplanting.h"

// iLand main includes
#include "species.h"
#include "mapgrid.h"

namespace ABE {

/** @class FomeScript
    @ingroup abe
    The FomeScript class is visible to Javascript via the 'fmengine' object. The main functions of ABE are available through this class.


  */


QString FomeScript::mInvalidContext = "S---";
ActivityFlags ActivityObj::mEmptyFlags;

FomeScript::FomeScript(QObject *parent) :
    QObject(parent)
{
    mStandObj = 0;
    mUnitObj = 0;
    mSimulationObj = 0;
    mActivityObj = 0;
    mSchedulerObj = 0;
    mTrees = 0;
    mStand = 0;
    mTree.setParent(this); // avoid JavaScriptOwnership
    mTreeValue = ForestManagementEngine::scriptEngine()->newQObject(&mTree);
}

FomeScript::~FomeScript()
{
    // all objects have script ownership - do not delete here.
//    if (mStandObj) {
//        delete mStandObj;
//        delete mSiteObj;
//        delete mSimulationObj;
//        delete mTrees;
//    }
}

void FomeScript::setupScriptEnvironment()
{
    // create javascript objects in the script engine
    // these objects can be accessed from Javascript code representing forest management activities
    // or agents.

    // stand variables
    mStandObj = new StandObj;
    QJSValue stand_value = ForestManagementEngine::scriptEngine()->newQObject(mStandObj);
    ForestManagementEngine::scriptEngine()->globalObject().setProperty("stand", stand_value);

    // unit level
    mUnitObj = new UnitObj;
    //QJSValue site_value = ForestManagementEngine::scriptEngine()->newQObject(mUnitObj);
    //ForestManagementEngine::scriptEngine()->globalObject().setProperty("unit", site_value);

    // general simulation variables (mainly scenariolevel)
    mSimulationObj = new SimulationObj;
    QJSValue simulation_value = ForestManagementEngine::scriptEngine()->newQObject(mSimulationObj);
    ForestManagementEngine::scriptEngine()->globalObject().setProperty("simulation", simulation_value);

    //access to the current activity
    mActivityObj = new ActivityObj;
    mActivityJS = ForestManagementEngine::scriptEngine()->newQObject(mActivityObj);
    QQmlEngine::setObjectOwnership(mActivityObj, QQmlEngine::CppOwnership);


    //QJSValue activity_value = ForestManagementEngine::scriptEngine()->newQObject(mActivityObj);
    //ForestManagementEngine::scriptEngine()->globalObject().setProperty("activity", activity_value);

    // general simulation variables (mainly scenariolevel)
    mTrees = new FMTreeList;
    //QJSValue treelist_value = ForestManagementEngine::scriptEngine()->newQObject(mTrees);
    //ForestManagementEngine::scriptEngine()->globalObject().setProperty("trees", treelist_value);

    // options of the STP
    mSTPObj = new STPObj;
    mSTPJS = ForestManagementEngine::scriptEngine()->newQObject(mSTPObj);
    QQmlEngine::setObjectOwnership(mSTPObj, QQmlEngine::CppOwnership);
    //QJSValue stp_value = ForestManagementEngine::scriptEngine()->newQObject(mSTPObj);
    //ForestManagementEngine::scriptEngine()->globalObject().setProperty("stp", stp_value);

    // scheduler options
    mSchedulerObj = new SchedulerObj;
    QJSValue scheduler_value = ForestManagementEngine::scriptEngine()->newQObject(mSchedulerObj);
    ForestManagementEngine::scriptEngine()->globalObject().setProperty("scheduler", scheduler_value);

    // the script object itself
    QJSValue script_value = ForestManagementEngine::scriptEngine()->newQObject(this);
    ForestManagementEngine::scriptEngine()->globalObject().setProperty("fmengine", script_value);

    // default agent
    ForestManagementEngine::scriptEngine()->evaluate(
    "fmengine.addAgent({ scheduler: {enabled: false}, " \
        "stp: { 'default': '_default'},"\
        "run: function() {}  }, '_default');"
        );

}

void FomeScript::setExecutionContext(FMStand *stand, bool add_agent)
{
    FomeScript *br = bridge();
    br->mStand = stand;
    br->mStandObj->setStand(stand);
    br->mTrees->setStand(stand);
    br->mUnitObj->setStand(stand);
    br->mActivityObj->setStand(stand, stand ? stand->currentActivity() : nullptr);
    br->mSchedulerObj->setStand(stand);
    br->mSTPObj->setFromStand(stand);
    if (stand && stand->trace())
        qCDebug(abe) << br->context() << "Prepared execution context (thread" << QThread::currentThread() << ").";
    if (add_agent) {
        const Agent *ag = stand->unit()->agent();
        ForestManagementEngine::instance()->scriptEngine()->globalObject().setProperty("agent", ag->jsAgent());
    }

}

void FomeScript::setActivity(Activity *act)
{
    FomeScript *br = bridge();
    setExecutionContext(nullptr);
    br->mActivityObj->setActivity(act);
}

FomeScript *FomeScript::bridge()
{
    // get the right bridge object (for the right thread??)
    return ForestManagementEngine::instance()->scriptBridge();
}

QString FomeScript::JStoString(QJSValue value)
{
    if (value.isArray() || value.isObject()) {
        QJSValue fun = ForestManagementEngine::scriptEngine()->evaluate("(function(a) { return JSON.stringify(a); })");
        QJSValue result = fun.call(QJSValueList() << value);
        return result.toString();
    } else
        return value.toString();

}

QStringList FomeScript::stpNames() const
{
    QStringList names;
    foreach (const FMSTP* stp, ForestManagementEngine::instance()->stps())
        names.push_back(stp->name());
    return names;

}

QJSValue FomeScript::treeRef(Tree *tree)
{
    mTree.setTree(tree);
    return mTreeValue;

}

bool FomeScript::verbose() const
{
    return FMSTP::verbose();
}

void FomeScript::setVerbose(bool arg)
{
    FMSTP::setVerbose(arg);
    qCDebug(abe) << "setting verbose property of ABE to" << arg;
}

int FomeScript::standId() const
{
    if (mStand)
        return mStand->id();
    return -1;
}

void FomeScript::setStandId(int new_stand_id)
{
    FMStand *stand = ForestManagementEngine::instance()->stand(new_stand_id);
    if (!stand) {
        qCDebug(abe) << bridge()->context() << "invalid stand id" << new_stand_id;
        return;
    }
    setExecutionContext(stand);
}

bool FomeScript::enabled() const
{
    return ForestManagementEngine::instance()->enabled();
}

void FomeScript::setEnabled(bool enable)
{
    ForestManagementEngine::instance()->setEnabled(enable);
}

void FomeScript::log(QJSValue value)
{
    QString msg = JStoString(value);
    qCDebug(abe) << bridge()->context() << msg;
}

void FomeScript::abort(QJSValue message)
{
    log(message);
    ForestManagementEngine::instance()->abortExecution(QString("%1: %2").arg(context()).arg(message.toString()));
}


bool FomeScript::addManagement(QJSValue program, QString name)
{
    try {
        FMSTP *stp = ForestManagementEngine::instance()->stp(name);
        if (stp) {
            ScriptGlobal::throwError(QString("Error in setting up STP '%1'. There is already a STP registered with that name.").arg(name));
            return false;
        }
        stp = new FMSTP();
        stp->setup(program, name);
        ForestManagementEngine::instance()->addSTP(stp);
        return true;
    } catch (const IException &e) {
        qCWarning(abe) << e.message();
        ForestManagementEngine::instance()->abortExecution(QString("Error in adding management.\n%1").arg(e.message()));
        return false;
    }
}

bool FomeScript::updateManagement(QJSValue program, QString name)
{
    try {
        FMSTP *stp = ForestManagementEngine::instance()->stp(name);
        if (!stp) {
            qCWarning(abe) << "updateManagement: STP" << name << "not found. No program updated.";
            return false;
        }
        stp->setup(program, name);
        // update associated stands (fix stand flags)
        QMultiMapIterator<FMUnit*, FMStand*> i(ForestManagementEngine::instance()->stands());
        while (i.hasNext()) {
            i.next();
            if (i.value()->stp() == stp)
                i.value()->initialize();
        }

        return true;
    } catch (const IException &e) {
        qCWarning(abe) << e.message();
        ForestManagementEngine::instance()->abortExecution(QString("Error in updating management '%2':\n%1").arg(e.message(), name));
        return false;
    }

}

bool FomeScript::addManagementToAgentType(QString name, QString agentname)
{
    FMSTP *stp = ForestManagementEngine::instance()->stp(name);
    if (!stp) {
        qCWarning(abe) << "addManagementToAgentType: STP" << name << "not found!";
        return false;
    }
    AgentType *at = ForestManagementEngine::instance()->agentType(agentname);
    if (!at) {
        qCWarning(abe) << "addManagementToAgentType: agenttype" << agentname << "not found!";
        return false;
    }
    at->addSTP(name);
    return true;

}

bool FomeScript::addAgentType(QJSValue program, QString name)
{
    try {
        AgentType *at = new AgentType();
        at->setupSTP(program, name);
        ForestManagementEngine::instance()->addAgentType(at);
        return true;
    } catch (const IException &e) {
        qCWarning(abe) << e.message();
        ForestManagementEngine::instance()->abortExecution(QString("Error in adding agent type definition.\n%1").arg(e.message()));
        return false;
    }

}

QJSValue FomeScript::addAgent(QString agent_type, QString agent_name)
{
    try {
    // find the agent type
    AgentType *at = ForestManagementEngine::instance()->agentType(agent_type);
    if (!at) {
        abort(QString("fmengine.addAgent: invalid 'agent_type': '%1'").arg(agent_type));
        return QJSValue();
    }
    Agent *ag = at->createAgent(agent_name);
    return ag->jsAgent();
    } catch (const IException &e) {
        qCWarning(abe) << e.message();
        ForestManagementEngine::instance()->abortExecution(QString("Error in adding agent definition.\n%1").arg(e.message()));
        return false;

    }
}

QJSValue FomeScript::addAgent(QJSValue program, QString name)
{
    // first create an agenttype
    if (!addAgentType(program, name))
        return false;
    return addAgent(name, name);
}

/// force execution of an activity (outside of the usual execution context, e.g. for debugging)
bool FomeScript::runActivity(int stand_id, QString activity)
{
    // find stand
    FMStand *stand = ForestManagementEngine::instance()->stand(stand_id);
    if (!stand)
        return false;
    if (!stand->stp())
        return false;
    Activity *act = stand->stp()->activity(activity);
    if (!act)
        return false;
    // run the activity....
    qCDebug(abe) << "running activity" << activity << "for stand" << stand_id;
    return act->execute(stand);
}

bool FomeScript::runActivityEvaluate(int stand_id, QString activity)
{
    // find stand
    FMStand *stand = ForestManagementEngine::instance()->stand(stand_id);
    if (!stand)
        return false;
    if (!stand->stp())
        return false;
    Activity *act = stand->stp()->activity(activity);
    if (!act)
        return false;
    // run the activity....
    qCDebug(abe) << "running evaluate of activity" << activity << "for stand" << stand_id;
    return act->evaluate(stand);

}

bool FomeScript::runAgent(int stand_id, QString function)
{
    // find stand
    FMStand *stand = ForestManagementEngine::instance()->stand(stand_id);
    if (!stand)
        return false;

    setExecutionContext(stand, true); // true: add also agent as 'agent'

    QJSValue val;
    QJSValue agent_type = stand->unit()->agent()->type()->jsObject();
    if (agent_type.property(function).isCallable()) {
        val = agent_type.property(function).callWithInstance(agent_type);
        qCDebug(abe) << "running agent-function" << function << "for stand" << stand_id << ":" << val.toString();
    } else {
        if (stand->trace())
            qCDebug(abe) << "function" << function << "is not a valid function of agent-type" << stand->unit()->agent()->type()->name();
    }

    return true;


}

bool FomeScript::isValidStand(int stand_id)
{
    FMStand *stand = ForestManagementEngine::instance()->stand(stand_id);
    if (stand)
        return true;

    return false;
}

QVariantList FomeScript::standIds()
{
    return ForestManagementEngine::instance()->standIds();
}

QJSValue FomeScript::activity(QString stp_name, QString activity_name)
{

    FMSTP *stp = ForestManagementEngine::instance()->stp(stp_name);
    if (!stp) {
        qCDebug(abe) << "fmengine.activity: invalid stp" << stp_name;
        return QJSValue();
    }

    Activity *act = stp->activity(activity_name);
    if (!act) {
        qCDebug(abe) << "fmengine.activity: activity" << activity_name << "not found in stp:" << stp_name;
        return QJSValue();
    }

    int idx = stp->activityIndex(act);
    ActivityObj *ao = new ActivityObj(nullptr, act, idx);
    QJSValue value = ForestManagementEngine::scriptEngine()->newQObject(ao);
    return value;

}

void FomeScript::runPlanting(int stand_id, QJSValue planting_item)
{
    FMStand *stand = ForestManagementEngine::instance()->stand(stand_id);
    if (!stand) {
        qCWarning(abe) << "runPlanting: stand not found" << stand_id;
        return;
    }

    ActPlanting::runSinglePlantingItem(stand, planting_item);


}

QJSValue FomeScript::stpByName(QString name)
{
    FMSTP *stp = ForestManagementEngine::instance()->stp(name);
    if (!stp) {
        ScriptGlobal::throwError(QString("stpByName(): No STP with name '%1'.").arg(name));
        return QJSValue();
    }
    STPObj *so = new STPObj();
    so->setSTP(stp);
    QJSValue value = ForestManagementEngine::scriptEngine()->newQObject(so);
    return value;

}

bool FomeScript::isValidStp(QString name)
{
    FMSTP *stp = ForestManagementEngine::instance()->stp(name);
    return stp != nullptr;
}

QJSValue FomeScript::test(QJSValue val)
{
    qDebug() << "value:"<<  val.toString();
    if (val.isNumber())
        qDebug() << "numeric: " << val.toNumber();
    return QJSValue();
}

int FomeScript::levelIndex(const QString &level_label)
{
    if (level_label=="low") return 1;
    if (level_label=="medium") return 2;
    if (level_label=="high") return 3;
    return -1;
}

const QString FomeScript::levelLabel(const int level_index)
{
    switch (level_index) {
    case 1: return QStringLiteral("low");
    case 2: return QStringLiteral("medium");
    case 3: return QStringLiteral("high");
    }
    return QStringLiteral("invalid");
}

QString StandObj::speciesId(int index) const
{
    if (index>=0 && index<nspecies()) return mStand->speciesData(index).species->id(); else return "error";
}

QJSValue StandObj::activityByName(QString name)
{
    Activity *act = mStand->stp()->activity(name);
    if (!act)
        return QJSValue();

    int idx = mStand->stp()->activityIndex(act);
    ActivityObj *ao = new ActivityObj(mStand, act, idx);
    QJSValue value = ForestManagementEngine::scriptEngine()->newQObject(ao);
    return value;

}

void StandObj::runNext(ActivityObj *next_act)
{
    if (!mStand || !mStand->stp()) return;

    if (!next_act->activity()) {
        ScriptGlobal::throwError("stand.runNext() called with an invalid activity.");
        return;
    }
    int index = mStand->stp()->activityIndex(next_act->activity());
    if (index < 0) {
        ScriptGlobal::throwError("stand.runNext() called with an activity that is not part of the current STP. Activity: " + next_act->name());
        return;
    }
    mStand->flags(index).setForceNext(true);
    mStand->setActivityIndex(index);
    if (mStand->sleepYears()>0){
        mStand->sleep(0, true);
    }
}

QJSValue StandObj::agent()
{
    if (mStand && mStand->unit()->agent())
        return mStand->unit()->agent()->jsAgent();
    else
        throwError("get agent of the stand failed.");
    return QJSValue();
}

UnitObj *StandObj::unit()
{
    if (!mStand) {
        throwError("stand not valid!"); return nullptr; }
    return FomeScript::bridge()->unitObj();
}

ActivityObj *StandObj::activity()
{
    if (!mStand) {
        throwError("stand not valid!"); return nullptr; }
    return FomeScript::bridge()->activityObj();

}

STPObj *StandObj::stp()
{
    if (!mStand) {
        throwError("stand not valid!"); return nullptr; }
    return FomeScript::bridge()->stpObj();

}

FMTreeList *StandObj::trees()
{
    if (!mStand) {
        throwError("stand not valid!"); return nullptr; }
    return FomeScript::bridge()->treesObj();

}

Patches *StandObj::patches()
{
    if (!mStand) {
        throwError("stand not valid!"); return nullptr; }
    return mStand->patches();
}

void StandObj::setAbsoluteAge(double arg)
{
    if (!mStand) {
        throwError("set absolute age"); return; }
    mStand->setAbsoluteAge(arg);
}

void StandObj::reset()
{
    if (!mStand) {
        throwError("reset"); return; }
    mStand->initialize();
}

bool StandObj::trace() const
{
    if (!mStand) { throwError("trace"); return false; }
    return mStand->trace();
}

void StandObj::setTrace(bool do_trace)
{
    if (!mStand) { throwError("trace"); return; }
    mStand->setProperty("trace", QJSValue(do_trace));
}

QRectF StandObj::rectangle() const
{

    if (!mStand)
        return QRectF();
        //return QVector<double>();
    QRectF box = ForestManagementEngine::instance()->standGrid()->boundingBox(mStand->id());
    return box;
}

int StandObj::timeSinceLastExecution() const
{
    if (mStand)
        return ForestManagementEngine::instance()->currentYear() - mStand->lastExecution();
    throwError("timeSinceLastExecution");
    return -1;
}

QString StandObj::lastActivity() const
{
    if (mStand && mStand->lastExecutedActivity())
        return mStand->lastExecutedActivity()->name();
    return QString();
}

double StandObj::rotationLength() const
{
    if (mStand)
        return mStand->U();
    throwError("U");
    return -1.;
}

void StandObj::setRotationLength(int new_length)
{
    if (mStand)
        mStand->setU(new_length);
}

QString StandObj::speciesComposition() const
{
    if (!mStand) return QString("Invalid");
    int index = mStand->targetSpeciesIndex();
    return mStand->unit()->agent()->type()->speciesCompositionName(index);

}

QString StandObj::thinningIntensity() const
{
    if (!mStand) return QString("Invalid");
    int t = mStand->thinningIntensity();
    return FomeScript::levelLabel(t);

}


void StandObj::setSTP(QString stp_name)
{
    if (mStand && mStand->unit() && mStand->unit()->agent() && mStand->unit()->agent()->type()) {
        QString old_stp = mStand->stp() ? mStand->stp()->name() : "none";
        FMSTP *stp = mStand->unit()->agent()->type()->stpByName(stp_name);
        if (!stp) {
            throwError(QString("The stp '%1' is not valid, and cannot be set for stand %2.").arg(stp_name).arg(mStand->id()));
            return;
        }
        try {
        int u = stp->rotationLengthOfType(mStand->thinningIntensity());
        if (u>0)
            mStand->setU( u );
        if (mStand->unit()->constScheduler())
            const_cast<FMUnit*>(mStand->unit())->scheduler()->clearItemsOfStand(mStand);
        mStand->setSTP(stp);
        mStand->initialize();
        qCDebug(abe) << mStand->context() << "switched STP from" << old_stp << "to" << stp_name;
        } catch(const IException &e) {
            throwError(QString("Error in setting stp to '%1' for stand '%2': \n %3").arg(stp_name).arg(mStand->id()).arg(e.message()));
        }
        return;

    }
    throwError(QString("The stp cannot be set, because the agent for stand %1 is not properly defined.").arg(mStand ? mStand->id() : -1));
}

void StandObj::repeat(QJSValue repeat_obj, QJSValue repeat_fun, int repeat_interval, int repeat_count)
{
    if (!repeat_fun.isCallable()) {
        throwError(QString("Stand::repeat: the 'what' to repeat needs to be a callable JavaScript object. It is: %1").arg(repeat_fun.toString()));
        return;
    }
    ForestManagementEngine::instance()->addRepeatJS(mStand->id(),
                                                  repeat_obj,
                                                  repeat_fun,
                                                  repeat_interval,
                                                  repeat_count);
}

void StandObj::throwError(QString msg) const
{
    FomeScript::bridge()->abort(QString("Error while accessing 'stand': no valid execution context. Message: %1").arg(msg));
}

/* ******************** */

bool ActivityObj::enabled() const
{
    return flags().enabled();
}

QString ActivityObj::name() const
{
    return mActivity? mActivity->name() : QStringLiteral("undefined");
}

QString ActivityObj::description() const
{
    return mActivity? mActivity->description() : QStringLiteral("undefined");
}

void ActivityObj::setEnabled(bool do_enable)
{
    flags().setEnabled(do_enable);
    if (!do_enable && mStand && mActivity==mStand->currentActivity()) {
        // when the current activity is disabled, we need to look for the next possible activity
        QString old_activity = mStand->currentActivity()->name();
        mStand->afterExecution(true); // cancel=true
        if (!mStand->currentActivity()) {
            // disabling of the current activity lead to a situation when no activity is active anymore (e.g. older than then age of clearcut)
            // a "hacky" solution is to force the execution of the latest activity with force=true (usually a final harvest)
            mStand->setToLatestForcedActivity();
            qCDebug(abe) << mStand->context() << "No valid activity found - forced execution of the latest activity with force=true.";
        }
        qCDebug(abe) << mStand->context() << "disabled currently active activity " << old_activity << ", new next activity:" << (mStand->currentActivity() ? mStand->currentActivity()->name() : QStringLiteral("*** no activity ***"));
    }
}


ActivityFlags &ActivityObj::flags() const
{
    // refer to a specific  activity of the stand (as returned by stand.activityByName("xxx") )
    if (mStand && mActivityIndex>-1)
        return mStand->flags(mActivityIndex);
//    if (mStand && !mActivity && mActivityIndex == -1)
//        return mEmptyFlags; // in this case also the currentFlags of the stand would be missing
    // during setup of activites (onCreate-handler)
    if (!mStand && mActivity)
        return mActivity->mBaseActivity;
    // refer to the *current* activity (the "activity" variable)
    if (mStand && mStand->currentActivityIndex()>-1)
        return mStand->currentFlags();


    qCDebug(abe) << "ActivityObj:flags: invalid access of flags! stand: " << mStand << "activity-index:" << mActivityIndex;
    return mEmptyFlags;
}

bool UnitObj::agentUpdate(QString what, QString how, QString when)
{
    AgentUpdate::UpdateType type = AgentUpdate::label(what);
    if (type == AgentUpdate::UpdateInvalid)
        qCDebug(abe) << "unit.agentUpdate: invalid 'what':" << what;

    AgentUpdate update;
    update.setType( type );

    // how
    int idx = FomeScript::levelIndex(how);
    if (idx>-1)
        update.setValue( QString::number(idx));
    else
        update.setValue( how );

    // when
    bool ok;
    int age = when.toInt(&ok);
    if (ok)
        update.setTimeAge(age);
    else
        update.setTimeActivity(when);

    mStand->unit()->agent()->type()->addAgentUpdate( update, const_cast<FMUnit*>(mStand->unit()) );
    qCDebug(abe) << "Unit::agentUpdate:" << update.dump();
    return true;
}

void UnitObj::updateManagementPlan()
{
    if (mStand && mStand->unit())
        const_cast<FMUnit*>(mStand->unit())->setForceUpdateManagementPlan();
}

QString UnitObj::harvestMode() const
{
    if (!mStand || mStand->unit()) return QString("invalid");
    return mStand->unit()->harvestMode();
}

QString UnitObj::speciesComposition() const
{
    if (!mStand || mStand->unit()) return QString("invalid");
    int index = mStand->unit()->targetSpeciesIndex();
    return mStand->unit()->agent()->type()->speciesCompositionName(index);
}

double UnitObj::U() const
{
    if (!mStand || mStand->unit()) return 0.;
    return mStand->U();
}

QString UnitObj::thinningIntensity() const
{
    if (!mStand || mStand->unit()) return QString("invalid");
    int t = mStand->unit()->thinningIntensity();
    return FomeScript::levelLabel(t);
}

double UnitObj::MAIChange() const
{
    // todo
    if (!mStand || mStand->unit()) return 0.;

    return mStand->unit()->annualIncrement();
}

double UnitObj::MAILevel() const
{
    if (!mStand || mStand->unit()) return 0.;
    return mStand->unit()->averageMAI();
}

double UnitObj::landscapeMAI() const
{
    // hacky way of getting a MAI on landscape level
    double total_area = 0.;
    double total_mai = 0.;
    const QVector<FMUnit*> &units = ForestManagementEngine::instance()->units();
    for (int i=0;i<units.size();++i) {
        total_area += units[i]->area();
        total_mai += units[i]->annualIncrement()*units[i]->area();
    }
    if (total_area>0.)
        return total_mai / total_area;
    else
        return 0.;
}

double UnitObj::mortalityChange() const
{
    return 1; // todo
}

double UnitObj::mortalityLevel() const
{
    return 1; // todo

}

double UnitObj::regenerationChange() const
{
    return 1; // todo

}

double UnitObj::regenerationLevel() const
{
    return 1; // todo

}

void SchedulerObj::dump() const
{
    if (!mStand || !mStand->unit() || !mStand->unit()->constScheduler())
        return;
    mStand->unit()->constScheduler()->dump();
}

bool SchedulerObj::enabled()
{
    if (!mStand) return false;
    const SchedulerOptions &opt = mStand->unit()->agent()->schedulerOptions();
    return opt.useScheduler;
}

void SchedulerObj::setEnabled(bool is_enabled)
{
    if (!mStand)
        return;
    SchedulerOptions &opt = const_cast<SchedulerOptions &>(mStand->unit()->agent()->schedulerOptions());
    opt.useScheduler = is_enabled;

}

double SchedulerObj::harvestIntensity()
{
    if (!mStand) return false;
    const SchedulerOptions &opt = mStand->unit()->agent()->schedulerOptions();
    return opt.harvestIntensity;
}

void SchedulerObj::setHarvestIntensity(double new_intensity)
{
    if (!mStand)
        return;
    SchedulerOptions &opt = const_cast<SchedulerOptions &>(mStand->unit()->agent()->schedulerOptions());
    opt.harvestIntensity = new_intensity;

}

double SchedulerObj::useSustainableHarvest()
{
    if (!mStand) return false;
    const SchedulerOptions &opt = mStand->unit()->agent()->schedulerOptions();
    return opt.useSustainableHarvest;
}

void SchedulerObj::setUseSustainableHarvest(double new_level)
{
    if (!mStand)
        return;
    SchedulerOptions &opt = const_cast<SchedulerOptions &>(mStand->unit()->agent()->schedulerOptions());
    opt.useSustainableHarvest = new_level;

}

double SchedulerObj::maxHarvestLevel()
{
    if (!mStand) return false;
    const SchedulerOptions &opt = mStand->unit()->agent()->schedulerOptions();
    return opt.maxHarvestLevel;
}

void SchedulerObj::setMaxHarvestLevel(double new_harvest_level)
{
    if (!mStand)
        return;
    SchedulerOptions &opt = const_cast<SchedulerOptions &>(mStand->unit()->agent()->schedulerOptions());
    opt.maxHarvestLevel = new_harvest_level;
}

double SchedulerObj::minScheduleHarvest()
{
    if (!mStand) return false;
    const SchedulerOptions &opt = mStand->unit()->agent()->schedulerOptions();
    return opt.minScheduleHarvest;

}

double SchedulerObj::maxScheduleHarvest()
{
    if (!mStand) return false;
    const SchedulerOptions &opt = mStand->unit()->agent()->schedulerOptions();
    return opt.maxScheduleHarvest;

}

void SchedulerObj::setMinScheduleHarvest(double new_level)
{
    if (!mStand)
        return;
    SchedulerOptions &opt = const_cast<SchedulerOptions &>(mStand->unit()->agent()->schedulerOptions());
    opt.minScheduleHarvest = new_level;
}

void SchedulerObj::setMaxScheduleHarvest(double new_level)
{
    if (!mStand)
        return;
    SchedulerOptions &opt = const_cast<SchedulerOptions &>(mStand->unit()->agent()->schedulerOptions());
    opt.maxScheduleHarvest = new_level;
}

void STPObj::setFromStand(FMStand *stand)
{
    if (stand && stand->stp()) {
        mSTP = stand->stp();
        mOptions = mSTP->JSoptions();
    } else {
        mOptions = 0;
        mSTP = 0;
    }
}

void STPObj::setSTP(FMSTP *stp)
{
    mSTP = stp;
    mOptions = mSTP->JSoptions();
}

QString STPObj::name()
{
    if (mSTP)
        return mSTP->name();
    else
        return "undefined";
}

int STPObj::activityCount() const
{
    if (!mSTP) {
        ScriptGlobal::throwError("stp not valid!"); return -1; }
    return mSTP->activities().size();
}

QStringList STPObj::activityNames()
{
    if (!mSTP) {
        ScriptGlobal::throwError("stp not valid!"); return QStringList(); }
    QStringList names;
    for (int i=0;i<mSTP->activities().size();++i)
        names.push_back(mSTP->activities()[i]->name());
    return names;

}

bool STPObj::signal(QString signalname, QJSValue parameter)
{
    if (!mSTP) {
        ScriptGlobal::throwError("stp not valid!"); return false; }
    if (FomeScript::bridge()->standId() < 0) {
        ScriptGlobal::throwError("STP::signal: no valid stand id!"); return false; }

    return mSTP->signal(signalname, FomeScript::bridge()->standObj()->stand(), parameter);
}


QString STPObj::info()
{
    if (mSTP) {
        return mSTP->info();
    }
    return "invalid";
}


} // namespace
