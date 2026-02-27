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
#include "globalsettings.h"


#include "forestmanagementengine.h"
#include "activity.h"
#include "fmunit.h"
#include "fmstand.h"
#include "fmstp.h"
#include "agent.h"
#include "agenttype.h"
#include "fomescript.h"
#include "scriptglobal.h"
#include "fomescript.h"
#include "scheduler.h"

#include "unitout.h"
#include "abestandout.h"
#include "abestandremovalout.h"

#include "debugtimer.h"

// general iLand stuff
#include "xmlhelper.h"
#include "csvfile.h"
#include "model.h"
#include "mapgrid.h"
#include "helper.h"
#include "threadrunner.h"
#include "outputmanager.h"

#include "tree.h"
#include "resourceunit.h"



Q_LOGGING_CATEGORY(abe, "abe")

Q_LOGGING_CATEGORY(abeSetup, "abe.setup")

namespace ABE {

/** @defgroup abe iLand agent based forest management engine (ABE)
  ABE is the Agent Based management Engine that allows the simulation of both forest management activties (e.g., harvesting of trees)
  and forest managers (e.g., deciding when and where to execute an activity).
  The ABE framework relies heavily on a blend of C++ (for low-level management activties) and Javascript (for higher level definition of
  management programs).

  The smallest spatial entity is a forest stand (FMStand), which may be grouped into forest management unit (FMUnit). Forest managers (Agent) can select
  stand treatment programs (FMSTP) for a unit. The management activities derive from a basic activity (Activity); specialized code exists
  for various activities such as planting or thinning. A scheduler (Scheduler) keeps track of where and when to execute activities following
  guidelines given by the management agent (Agent). Agents represent individual foresters that may be grouped into AgentTypes (e.g., farmers).


 */


/** @class ForestManagementEngine
 * @ingroup abe
*/

ForestManagementEngine *ForestManagementEngine::singleton_fome_engine = nullptr;
int ForestManagementEngine::mMaxStandId = -1;
ForestManagementEngine::ForestManagementEngine()
{
    mScriptBridge = nullptr;
    singleton_fome_engine = this;
    mCancel = false;
    mEnabled = true;
    setupOutputs(); // add ABE output definitions
    FMStand::clearAllProperties();
}

ForestManagementEngine::~ForestManagementEngine()
{
    clear();
    // script bridge: script ownership?
    //if (mScriptBridge)
    //    delete mScriptBridge;
    singleton_fome_engine = nullptr;
}

const MapGrid *ForestManagementEngine::standGrid()
{
    return GlobalSettings::instance()->model()->standGrid();
}


void ForestManagementEngine::setupScripting()
{
    // setup the ABE system
    const XmlHelper &xml = GlobalSettings::instance()->settings();

    ScriptGlobal::setupGlobalScripting(); // general iLand scripting helper functions and such

    // the link between the scripting and the C++ side of ABE
    if (mScriptBridge)
        delete mScriptBridge;
    mScriptBridge = new FomeScript;
    mScriptBridge->setupScriptEnvironment();

    QString file_name = GlobalSettings::instance()->path(xml.value("model.management.abe.file"));
    QString code = Helper::loadTextFile(file_name);
    if (code.isEmpty())
        throw IException("Loading of ABE script file '"  + file_name + "'failed; file missing or empty.");
    qCDebug(abeSetup) << "Loading script file" << file_name;
    QJSValue result = GlobalSettings::instance()->scriptEngine()->evaluate(code,file_name);
    if (result.isError()) {
        int lineno = result.property("lineNumber").toInt();
        QStringList code_lines = code.replace('\r', "").split('\n'); // remove CR, split by LF
        QString code_part;
        for (int i=std::max(0, lineno - 5); i<std::min(lineno+5, static_cast<int>(code_lines.count())); ++i)
            code_part.append(QString("%1: %2 %3\n").arg(i).arg(code_lines[i]).arg(i==lineno?"  <---- [ERROR]":""));
        qCCritical(abeSetup) << "Javascript Error in file" << result.property("fileName").toString() << ":" << result.property("lineNumber").toInt() << ":" << result.toString() << ":\n" << code_part;
        QString error_message = "Abe Error in Javascript (Please check also the logfile): " + result.toString()+ "\nIn:\n" + code_part + "\n" + result.property("stack").toString();
        Helper::msg(error_message);
        ScriptGlobal::throwError(error_message);

    }
}

void ForestManagementEngine::prepareRun()
{
    mStandLayoutChanged = false; // can be changed by salvage operations / stand polygon changes
}

void ForestManagementEngine::finalizeRun()
{
    // empty the harvest counter; it will be filled again
    // during the (next) year.

    foreach (FMStand *stand, mStands) {
        stand->resetHarvestCounter();
    }

    //
    if (mStandLayoutChanged) {
        DebugTimer timer("ABE:stand_layout_update");
        // renew the internal stand grid
        FMStand **fm = mFMStandGrid.begin();
        for (int *p = standGrid()->grid().begin(); p!=standGrid()->grid().end(); ++p, ++fm)
            *fm = *p<0?nullptr:mStandHash[*p];
        // renew neigborhood information in the stand grid
        const_cast<MapGrid*>(standGrid())->updateNeighborList();
        // renew the spatial indices
        const_cast<MapGrid*>(standGrid())->createIndex();
        mStandLayoutChanged = false;

        // now check the stands
        for (QVector<FMStand*>::iterator it=mStands.begin(); it!=mStands.end(); ++it) {
            // renew area
            (*it)->checkArea();
            // initial activity (if missing)
            if (!(*it)->currentActivity()) {
                (*it)->initialize();
            }
        }
    }

}

void ForestManagementEngine::setupOutputs()
{
    if (GlobalSettings::instance()->outputManager()->find("abeUnit"))
        return; // already set up
    GlobalSettings::instance()->outputManager()->addOutput(new UnitOut);
    GlobalSettings::instance()->outputManager()->addOutput(new ABEStandOut);
    GlobalSettings::instance()->outputManager()->addOutput(new ABEStandDetailsOut);
    GlobalSettings::instance()->outputManager()->addOutput(new ABEStandRemovalOut);
}

void ForestManagementEngine::runJavascript(bool after_processing)
{
    if (after_processing) {
        QJSValue handler = scriptEngine()->globalObject().property("runFinalize");
        if (handler.isCallable()) {
            scriptBridge()->setExecutionContext(nullptr, false);
            QJSValue result = handler.call(QJSValueList() << mCurrentYear);
            if (FMSTP::verbose())
                qCDebug(abe) << "executing 'runFinalize' function for year" << mCurrentYear << ", result:" << result.toString();
        }
        return;

    }
    QJSValue handler = scriptEngine()->globalObject().property("run");
    if (handler.isCallable()) {
        scriptBridge()->setExecutionContext(nullptr, false);
        QJSValue result = handler.call(QJSValueList() << mCurrentYear);
        if (FMSTP::verbose())
            qCDebug(abe) << "executing 'run' function for year" << mCurrentYear << ", result:" << result.toString();
    }

    handler = scriptEngine()->globalObject().property("runStand");
    if (handler.isCallable()) {
        qCDebug(abe) << "running the 'runStand' javascript function for" << mStands.size() << "stands.";
        foreach (FMStand *stand, mStands) {
            scriptBridge()->setExecutionContext(stand, true);
            handler.call(QJSValueList() << mCurrentYear);
        }
    }
}

void ForestManagementEngine::runRepeatedItems(int stand_id)
{
    // anything to do?
    auto it = mRepeatStore.find(stand_id);
    if (it == mRepeatStore.end())
        return;


    // Use two buffers to avoid concurrent modification issues
    QList<QPair< int, SRepeatItem> > buffer1;
    QList<QPair< int, SRepeatItem> > buffer2;
    QList<QPair< int, SRepeatItem> >* currentBuffer = &buffer1;
    QList<QPair< int, SRepeatItem> >* nextBuffer = &buffer2;

    // Move initial items from mRepeatStore to buffer1
    while (it != mRepeatStore.end() && it.key() == stand_id) {
        buffer1.push_back(QPair< int, SRepeatItem>(stand_id, it.value()));
        it = mRepeatStore.erase(it);
    }

    // process elements
    int iteration_depth = 0;
    while (!currentBuffer->empty()) {
        mRepeatStoreBuffer = nextBuffer; // Allow adding new items to the next buffer

        // Iterate through the items in the current buffer
        for (auto &p : *currentBuffer) {
            bool do_erase = runSingleRepeatedItem(stand_id, p.second);
            if (!do_erase) {
                mRepeatStore.insert(p.first, p.second);
            }
        }

        currentBuffer->clear(); // Clear the buffer (all elements are processed)

        std::swap(currentBuffer, nextBuffer); // Swap the buffers
        mRepeatStoreBuffer = nullptr; // Disable adding while swapping the buffers

        if (++iteration_depth > 99) {
            throw IException("ABE: Signal handling: infinite loop detected!");
        }
    }
}

bool ForestManagementEngine::runSingleRepeatedItem(int stand_id, SRepeatItem &item) {
    if (--item.waitYears <= 0) {
        // run the item
        item.N++; // number of invocation, 1,2,3,...


        FMStand* stnd = stand(stand_id);
        if (!stnd)
            throw IException(QString("Invalid stand-id for repeating activity: '%1'").arg(stand_id));
        FomeScript::setExecutionContext(stnd);

        if (item.activity) {
            // run the activity
            //bool res = stnd->executeActivity(it->activity);
            int old_index = stnd->currentActivityIndex();
            stnd->setSignalParameter(item.parameter);
            stnd->setActivityIndex( item.activity->index() );
            bool res = item.activity->execute(stnd);
            // special case final harvest: if the activcity is a final harvest, we
            // need to reset the rotation (onExecuted is called as well)
            if (stnd->currentFlags().isFinalHarvest()) {
                stnd->afterExecution(!res);
            } else {
                item.activity->runEvent(QStringLiteral("onExecuted"),stnd);
                stnd->setActivityIndex( old_index );
            }
            stnd->setSignalParameter(QJSValue());
            qCDebug(abe) << "executed activity (repeated): " << item.activity->name() << ". Result: " << res;
        } else {
            // run javascript function
            QJSValueList params = { item.parameter };
            QJSValue result;


            if (item.jsobj.isUndefined())
                result = item.callback.call(params);
            else
                result = item.callback.callWithInstance(item.jsobj, params);

            if (result.isError())
                FomeScript::bridge()->abort(result);

            qCDebug(abe) << "executed repeated op for stand" << stand_id << ", result:" << result.toString();
        }

        // reset
        item.waitYears = item.interval; // start again the countdown

        if (item.N >= item.times) {
            /*if (item.activity && item.times * item.interval > 1) {
                // call also the offboarding code for activities when the repeating activity
                // was active for a longer time (make sure we do not miss activities)
                int old_index = stnd->currentActivityIndex();
                stnd->setActivityIndex( item.activity->index() );
                stnd->afterExecution();
                //stnd->setActivityIndex( old_index );
            }*/
            return true;
        }
    }
    return false;

}

AgentType *ForestManagementEngine::agentType(const QString &name)
{
    for (int i=0;i<mAgentTypes.count();++i)
        if (mAgentTypes[i]->name()==name)
            return mAgentTypes[i];
    return nullptr;
}

Agent *ForestManagementEngine::agent(const QString &name)
{
    for (int i=0;i<mAgents.count();++i)
        if (mAgents[i]->name()==name)
            return mAgents[i];
    return nullptr;
}


/*---------------------------------------------------------------------
 * multithreaded execution routines
---------------------------------------------------------------------*/

FMUnit *nc_execute_unit(FMUnit *unit)
{
    if (ForestManagementEngine::instance()->isCancel())
        return unit;
    try {
        //qDebug() << "called for unit" << unit;
        const QMultiMap<FMUnit*, FMStand*> &stand_map = ForestManagementEngine::instance()->stands();
        QMultiMap<FMUnit*, FMStand*>::const_iterator it = stand_map.constFind(unit);
        int executed = 0;
        int total = 0;
        while (it!=stand_map.constEnd() && it.key()==unit) {
            // execute repeating activities for the stand
            if (it.value()->stp())
                it.value()->stp()->executeRepeatingActivities(it.value());
            ForestManagementEngine::instance()->runRepeatedItems(it.value()->id());

            // run the "normal" management for the stand
            if (it.value()->execute())
                ++executed;

            if (ForestManagementEngine::instance()->isCancel())
                break;

            ++it;
            ++total;
        }
        if (ForestManagementEngine::instance()->isCancel())
            return unit;

        if (FMSTP::verbose())
            qCDebug(abe) << "execute unit'" << unit->id() << "', ran" << executed << "of" << total;

        // now run the scheduler
        unit->scheduler()->run();
    } catch (const IException &e) {
        // thread-safe error message
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }



    return unit;
}

FMUnit *nc_plan_update_unit(FMUnit *unit)
{
    if (ForestManagementEngine::instance()->isCancel())
        return unit;

    if (ForestManagementEngine::instance()->currentYear() % 10 == 0 || unit->forceUpdateManagementPlan()) {
        qCDebug(abe) << "*** execute decadal plan update ***";
        unit->managementPlanUpdate();
        unit->runAgent();
    }


    // first update happens *after* a full year of running ABE.
    if (ForestManagementEngine::instance()->currentYear()>1)
        unit->updatePlanOfCurrentYear();

    return unit;
}



void ForestManagementEngine::setup()
{
    QString enable_debug = logLevelDebug() ? "true" : "false";
    QLoggingCategory::setFilterRules(QString("abe.debug=%1\n" \
                                             "abe.setup.debug=true").arg(enable_debug) ); // enable *all*

    DebugTimer time_setup("ABE:setupScripting");
    clear();

    // (1) setup the scripting environment and load all the javascript code
    setupScripting();
    if (isCancel()) {
        throw IException(QString("ABE-Error (setup): %1").arg(mLastErrorMessage));
    }

    if (!GlobalSettings::instance()->model())
        throw IException("No model created.... invalid operation.");

    // (2) spatial data (stands, units, ...)
    const MapGrid *stand_grid = GlobalSettings::instance()->model()->standGrid();

    if (stand_grid==nullptr || stand_grid->isValid()==false)
        throw IException("The ABE management model requires a valid stand grid.");

    const XmlHelper &xml = GlobalSettings::instance()->settings();

    QString data_file_name = GlobalSettings::instance()->path(xml.value("model.management.abe.agentDataFile"));
    qCDebug(abeSetup) << "loading ABE agentDataFile" << data_file_name << "...";
    CSVFile data_file(data_file_name);
    if (data_file.isEmpty())
        throw IException(QString("Stand-Initialization: the standDataFile file %1 is empty or missing!").arg(data_file_name));
    QStringList forbiddenExtraColums=QStringList() << "id" << "unit" << "agent" << "agentType" << "stp" << "speciesComposition" << "thinningIntensity" << "U" << "MAI" << "harvestMode" << "age";
    QStringList extraColumns;
    for (int i=0;i<data_file.colCount();++i) {
        if ( !forbiddenExtraColums.contains(data_file.columnName(i)) )
            extraColumns.push_back(data_file.columnName(i));
    }
    if (extraColumns.length()>0)
        qCDebug(abeSetup) << "Input file contains extra properties which will be stored as stand-properties: " << extraColumns;

    int ikey = data_file.columnIndex("id");
    int iunit = data_file.columnIndex("unit");
    int iagent = data_file.columnIndex("agent");
    int iagent_type = data_file.columnIndex("agentType");
    int istp = data_file.columnIndex("stp");
    // unit properties
    int ispeciescomp = data_file.columnIndex("speciesComposition");
    int ithinning = data_file.columnIndex("thinningIntensity");
    int irotation = data_file.columnIndex("U");
    int iage = data_file.columnIndex("age");
    int iMAI = data_file.columnIndex("MAI");
    int iharvest_mode = data_file.columnIndex("harvestMode");


    if (ikey<0)
        throw IException("setup ABE agentDataFile: the required column 'id' not available.");
    //if (iagent<0 && iagent_type<0)
    //    throw IException("setup ABE agentDataFile: the columns 'agent' or 'agentType' are not available. You have to include at least one of the columns.");


    QList<QString> unit_codes;
    QHash<FMStand*, QString> initial_stps;
    for (int i=0;i<data_file.rowCount();++i) {
        int stand_id = data_file.value(i,ikey).toInt();
        if (!stand_grid->isValid(stand_id))
            continue; // skip stands that are not in the map (e.g. when a smaller extent is simulated)
        if (FMSTP::verbose())
            qCDebug(abeSetup) << "setting up stand" << stand_id;

        // check agents
        QString agent_code = iagent>-1 ? data_file.value(i, iagent).toString() : QString();
        QString agent_type_code = iagent_type>-1 ? data_file.value(i, iagent_type).toString() : QString();
        QString unit_id = iunit >-1 ? data_file.value(i, iunit).toString() : "_default";

        Agent *ag=agent("_default");
        AgentType *at=(ag ? ag->type() : nullptr);
        if (agent_code.isEmpty() && agent_type_code.isEmpty())
            qCDebug(abeSetup) << "setup ABE agentDataFile row" << i << " : no code for columns 'agent' and 'agentType'. Using '_default' agent.";

        if (!agent_code.isEmpty()) {
            // search for a specific agent
            ag = agent(agent_code);
            if (!ag)
                throw IException(QString("Agent '%1' is not available (referenced in row '%2')! Use 'addAgent()' JS function to create agents.\nYou might have a Javascript error - check the logfile.").arg(agent_code).arg(i));
            at = ag->type();

        }

        if (!agent_type_code.isEmpty()) {
            // look up the agent type and create the agent on the fly
            // create the agent / agent type
            at = agentType(agent_type_code);
            if (!at)
                throw IException(QString("Agent type '%1' is not set up (row '%2')! Use the 'addAgentType()' JS function to add agent-type definitions.").arg(agent_type_code).arg(i));

            if (!unit_codes.contains(unit_id)) {
                // we create an agent for the unit only once (per unit)
                ag = at->createAgent();
            }
        }

        if (!ag)
            throw IException("ABE setup: no agent defined and no default agent available.");


        // check units
        FMUnit *unit = nullptr;
        if (!unit_codes.contains(unit_id)) {
            // create the unit
            unit = new FMUnit(ag);
            unit->setId(unit_id);
            if (iharvest_mode>-1)
                unit->setHarvestMode( data_file.value(i, iharvest_mode).toString());
            if (ithinning>-1)
                unit->setThinningIntensity( data_file.value(i, ithinning).toInt() );
            if (irotation>-1)
                unit->setU( data_file.value(i, irotation).toDouble() );
            if (iMAI>-1)
                unit->setAverageMAI(data_file.value(i, iMAI).toDouble());
            if (ispeciescomp>-1) {
                int index;
                index = at->speciesCompositionIndex( data_file.value(i, ispeciescomp).toString() );
                if (index==-1)
                    throw IException(QString("The species composition '%1' for unit '%2' is not a valid composition type (agent type: '%3').").arg(data_file.value(i, ispeciescomp).toString()).arg(unit->id()).arg(at->name()));
                unit->setTargetSpeciesCompositionIndex( index );
            }
            mUnits.append(unit);
            unit_codes.append(unit_id);
            if (ag)
                ag->addUnit(unit); // add the unit to the list of managed units of the agent
        } else {
            // get unit by id ... in this case we have the same order of appending values
            unit = mUnits[unit_codes.indexOf(unit_id)];
        }

        // create stand
        FMStand *stand = new FMStand(unit,stand_id);
        // set properties
        for (int p=0;p<extraColumns.size();++p)
            stand->setProperty(extraColumns[p], data_file.jsValue(i, extraColumns[p]));

        if (irotation>-1)
            stand->setU( data_file.value(i, irotation).toDouble() );

        if (istp>-1) {
            QString stp = data_file.value(i, istp).toString();
            initial_stps[stand] = stp;
        }

        if (iage>-1)
            stand->setAbsoluteAge(data_file.value(i, iage).toDouble());

        mMaxStandId = qMax(mMaxStandId, stand_id);

        mUnitStandMap.insert(unit,stand);
        mStands.append(stand);

    }

    // count the number of stands within each unit
    foreach(FMUnit *unit, mUnits)
        unit->setNumberOfStands( mUnitStandMap.count(unit) );

    // set up the stand grid (visualizations)...
    // set up a hash for helping to establish stand-id <-> fmstand-link
    mStandHash.clear();
    for (int i=0;i<mStands.size(); ++i) {
        mStandHash[mStands[i]->id()] = mStands[i];
    }

    mFMStandGrid.setup(standGrid()->grid().metricRect(), standGrid()->grid().cellsize());
    mFMStandGrid.initialize(nullptr);
    FMStand **fm = mFMStandGrid.begin();
    for (int *p = standGrid()->grid().begin(); p!=standGrid()->grid().end(); ++p, ++fm)
        *fm = *p<0?nullptr:mStandHash[*p];

    mStandLayers.setGrid(mFMStandGrid);
    mStandLayers.clearClasses();
    mStandLayers.registerLayers();

    // now initialize STPs (if they are defined in the init file)
    for (QHash<FMStand*,QString>::iterator it=initial_stps.begin(); it!=initial_stps.end(); ++it) {
        FMStand *s = it.key();
        FMSTP* stp = s->unit()->agent()->type()->stpByName(it.value());
        if (stp) {
            s->setSTP(stp);
            // if U is not set (by a U column in the CSV), then use the STP default
            if (s->U()==0.)
                s->setU(stp->rotationLengthOfType(2));
        } else {
            qCDebug(abeSetup) << "Warning while procssing the CSV setup file: for stand '" << s->id() << "' tried to set STP to '" << it.value() << "', but this STP is not available for Agenttype: " << s->unit()->agent()->type()->name();
        }
    }
    qCDebug(abeSetup) << "ABE setup completed.";
}

void ForestManagementEngine::initialize()
{

    if (isCancel()) {
        qCDebug(abeSetup) << "ABE setup stopped due to errors.";
        return;
    }
    DebugTimer time_setup("ABE:setup");

    foreach (FMStand* stand, mStands) {
        if (stand->stp()) {

            if (stand->U()==0.)
                stand->setU( stand->unit()->U() );
            stand->setThinningIntensity( stand->unit()->thinningIntensity() );
            stand->setTargetSpeciesIndex( stand->unit()->targetSpeciesIndex() );

            stand->initialize();
            if (isCancel()) {
                throw IException(QString("ABE-Error: init of stand %2: %1").arg(mLastErrorMessage).arg(stand->id()));
            }
        }
    }

    // now initialize the agents....
    foreach(Agent *ag, mAgents) {
        ag->setup();
        if (isCancel()) {
            throw IException(QString("ABE-Error: setup of agent '%2': %1").arg(mLastErrorMessage).arg(ag->name()));
        }
    }

    // run the initial planning unit setup
    GlobalSettings::instance()->model()->threadExec().run(nc_plan_update_unit, mUnits);


    qCDebug(abeSetup) << "ABE setup complete." << mUnitStandMap.size() << "stands on" << mUnits.count() << "units, managed by" << mAgents.size() << "agents.";

}

void ForestManagementEngine::clear()
{
    qDeleteAll(mStands); // delete the stands
    mStands.clear();
    qDeleteAll(mUnits); // deletes the units
    mUnits.clear();
    mUnitStandMap.clear();

    qDeleteAll(mAgents);
    mAgents.clear();
    qDeleteAll(mAgentTypes);
    mAgentTypes.clear();
    qDeleteAll(mSTP);
    mSTP.clear();
    mCurrentYear = 0;
    mCancel = false;
    mLastErrorMessage = QString();
}

void ForestManagementEngine::abortExecution(const QString &message)
{
    mLastErrorMessage = message;
    mCancel = true;
}

void ForestManagementEngine::runOnInit(bool before_init)
{
    QString handler = before_init ? QStringLiteral("onInit") : QStringLiteral("onAfterInit");
    if (GlobalSettings::instance()->scriptEngine()->globalObject().hasProperty(handler)) {
        QJSValue result = GlobalSettings::instance()->scriptEngine()->evaluate(QString("%1()").arg(handler));
        if (result.isError())
            qCDebug(abeSetup) << "Javascript Error in global"<< handler << "-Handler:" << result.toString();

    }
}




/// this is the main function of the forest management engine.
/// the function is called every year.
void ForestManagementEngine::run(int debug_year)
{


    if (debug_year>-1) {
        mCurrentYear++;
    } else {
        mCurrentYear = GlobalSettings::instance()->currentYear();
    }
    mCancel=false; // reset
    // now re-evaluate stands
    if (FMSTP::verbose()) qCDebug(abe) << "ForestManagementEngine: run year" << mCurrentYear;


    prepareRun();
    ExprExceptionAsScriptError no_expression;

    // execute an event handler before invoking the ABE core
    runJavascript(false);

    if (enabled()) {
        {
            // launch the planning unit level update (annual and thorough analysis every ten years)
            DebugTimer plu("ABE:planUpdate");
            GlobalSettings::instance()->model()->threadExec().run(nc_plan_update_unit, mUnits, true);
        }

        // run the actual forest management (incl. scheduler)
        GlobalSettings::instance()->model()->threadExec().run(nc_execute_unit, mUnits, true); // force single thread operation for now
        if (isCancel()) {
            throw IException(QString("ABE-Error: %1").arg(mLastErrorMessage));
        }
        // reset salvage and disturbance (from *last* year) of each stand (after all activites are processed)
        // salvaged timber will be filled during disturbances, which run *after* run() and *before* yearEnd().
        foreach (FMStand *stand, mStands) {
            stand->resetDisturbanceSalvage();
        }


    } else {
        qCDebug(abe) << "ForestManagementEngine: ABE is currently disabled.";
    }




}

void ForestManagementEngine::yearEnd()
{
    // reset the salvage harvest counter (salvage from *last* year) from the scheduler,
    // and collect on unit level all harvests that happened during (a) the execution of ABE, and (b) the salvaging operations
    const QMultiMap<FMUnit*, FMStand*> &stand_map = ForestManagementEngine::instance()->stands();

    foreach (FMUnit *unit, mUnits) {
        unit->resetHarvestCounter();
        // collect the harvests
        QMultiMap<FMUnit*, FMStand*>::const_iterator it = stand_map.constFind(unit);
        while (it!=stand_map.constEnd() && it.key()==unit) {
            unit->addRealizedHarvest(it.value()->totalHarvest());
            ++it;
        }
    }

    // sum up salvage occured during the *current* year (disturbances) -> salvage shows up in the unit outputs of the current year
    // the scheduler will use the sum of the current year in the scheduling of operations next year
    foreach (FMStand *stand, mStands) {
        if (stand->salvagedTimber()>0.) {
            const_cast<FMUnit*>(stand->unit())->scheduler()->addExtraHarvest(stand, stand->salvagedTimber(), Scheduler::Salvage);
        }
    }

    // create outputs

    DebugTimer plu("ABE:outputs");
    if (enabled()) {
        GlobalSettings::instance()->outputManager()->execute("abeUnit");
        GlobalSettings::instance()->outputManager()->execute("abeStandDetail");
    }
    // run ABEStand and StandRemoval in any case.
    GlobalSettings::instance()->outputManager()->execute("abeStand");
    GlobalSettings::instance()->outputManager()->execute("abeStandRemoval");


    finalizeRun();
    // execute an event handler after invoking the ABE core (runFinalize())
    runJavascript(true);

}




void ForestManagementEngine::test()
{
    // test code
    try {
        //Activity::setVerbose(true);
        // setup the activities and the javascript environment...
        GlobalSettings::instance()->resetScriptEngine(); // clear the script
        ScriptGlobal::setupGlobalScripting(); // general iLand scripting helper functions and such
        if (mScriptBridge)
            delete mScriptBridge;
        mScriptBridge = new FomeScript;
        mScriptBridge->setupScriptEnvironment();

        //setup();

    } catch (const IException &e) {
        qDebug() << "An error occured:" << e.message();
    }
    QString file_name = "E:/Daten/iLand/modeling/abm/knowledge_base/test/test_stp.js";
    QString code = Helper::loadTextFile(file_name);
    QJSValue result = GlobalSettings::instance()->scriptEngine()->evaluate(code,file_name);
    if (result.isError()) {
        int lineno = result.property("lineNumber").toInt();
        QStringList code_lines = code.replace('\r', "").split('\n'); // remove CR, split by LF
        QString code_part;
        for (int i=std::max(0, lineno - 5); i<std::min(lineno+5, static_cast<int>(code_lines.count())); ++i)
            code_part.append(QString("%1: %2 %3\n").arg(i).arg(code_lines[i]).arg(i==lineno?"  <---- [ERROR]":""));
        qDebug() << "Javascript Error in file" << result.property("fileName").toString() << ":" << result.property("lineNumber").toInt() << ":" << result.toString() << ":\n" << code_part;
    }


//    try {
//        qDebug() << "*** test 1 ***";
//        FMSTP stp;
//        stp.setVerbose(true);
//        stp.setup(GlobalSettings::instance()->scriptEngine()->globalObject().property("stp"), "stp");
//        stp.dumpInfo();

//    } catch (const IException &e) {
//        qDebug() << "An error occured:" << e.message();
//    }
//    try {
//        qDebug() << "*** test 2 ***";
//        FMSTP stp2;
//        stp2.setVerbose(true);
//        stp2.setup(GlobalSettings::instance()->scriptEngine()->globalObject().property("degenerated"), "degenerated");
//        stp2.dumpInfo();
//    } catch (const IException &e) {
//        qDebug() << "An error occured:" << e.message();
//    }

    // dump all objects:
    foreach(FMSTP *stp, mSTP)
        qDebug() << stp->info();

    setup();
    qDebug() << "finished";

}

QStringList ForestManagementEngine::evaluateClick(const QPointF coord, const QString &grid_name)
{
    Q_UNUSED(grid_name); // for the moment
    // find the stand at coord.
    FMStand *stand = mFMStandGrid.constValueAt(coord);
    if (stand)
        return stand->info();
    return QStringList();
}

QJSEngine *ForestManagementEngine::scriptEngine()
{
    // use global engine from iLand
    return GlobalSettings::instance()->scriptEngine();
}

FMSTP *ForestManagementEngine::stp(QString stp_name) const
{
    if (stp_name == "default" && mSTP.size()==1)
        return mSTP[0];
    for (QVector<FMSTP*>::const_iterator it = mSTP.constBegin(); it!=mSTP.constEnd(); ++it)
        if ( (*it)->name() == stp_name )
            return *it;
    return nullptr;
}

FMStand *ForestManagementEngine::stand(int stand_id) const
{
    if (mStandHash.contains(stand_id))
        return mStandHash[stand_id];

    // exhaustive search... should not happen
    qCDebug(abe) << "ForestManagementEngine::stand() fallback to exhaustive search.";
    for (QVector<FMStand*>::const_iterator it=mStands.constBegin(); it!=mStands.constEnd(); ++it)
        if ( (*it)->id() == stand_id)
            return *it;
    return nullptr;
}

QVariantList ForestManagementEngine::standIds() const
{
    QVariantList standids;
    foreach(FMStand *s, mStands)
        standids.push_back(s->id());
    return standids;
}

void ForestManagementEngine::addRepeatJS(int stand_id, QJSValue obj, QJSValue callback, int repeatInterval, int repeatTimes)
{
    if (mRepeatStoreBuffer)
        mRepeatStoreBuffer->push_back({stand_id, SRepeatItem(repeatInterval, repeatTimes, obj, callback)}); // to avoid problems with invalid iterators
    else
        mRepeatStore.insert(stand_id, SRepeatItem(repeatInterval, repeatTimes, obj, callback));
}

void ForestManagementEngine::addRepeatActivity(int stand_id, Activity *act, int repeatInterval, int repeatTimes, QJSValue parameter)
{
    if (mRepeatStoreBuffer)
        mRepeatStoreBuffer->push_back({stand_id, SRepeatItem(repeatInterval, repeatTimes, act, parameter)});
    else
        mRepeatStore.insert(stand_id, SRepeatItem(repeatInterval, repeatTimes, act, parameter));
}

void ForestManagementEngine::stopRepeat(int stand_id, QJSValue obj)
{
    auto it = mRepeatStore.find(stand_id);
    while (it != mRepeatStore.end() && it.key() == stand_id) {
        if (it->jsobj.equals(obj))
            it = mRepeatStore.erase(it);
        else
            ++it;
    }

}

void ForestManagementEngine::notifyTreeRemoval(Tree *tree, int reason)
{
    //if (!enabled())
    //    return;
    // we use an 'int' instead of Tree:TreeRemovalType because it does not work
    // with forward declaration (and I dont want to include the tree.h header in this class header).

    // test for emptyness: this handles the case when ABE is not yet properly set up
    if (mFMStandGrid.isEmpty())
        return;

    FMStand *stand = mFMStandGrid[tree->position()];
    if (stand)
        stand->notifyTreeRemoval(tree, reason);
    else
        if (FMSTP::verbose())
            qDebug() << "ForestManagementEngine::notifyTreeRemoval(): tree not on a stand (unmanaged?) at (metric coords): " << tree->position() << "ID:" << tree->id();
}

bool ForestManagementEngine::notifyBarkbeetleAttack(const ResourceUnit *ru, const double generations, int n_infested_px)
{
    if (!enabled())
        return false;

    // find out which stands are within the resource unit
    GridRunner<FMStand*> gr(mFMStandGrid, ru->boundingBox());
    QHash<FMStand*, bool> processed_items;
    bool forest_changed = false;
    while (FMStand **s=gr.next()) {
        if (*s && !processed_items.contains(*s)) {
            processed_items[*s] = true;
            forest_changed |=  (*s)->notifyBarkBeetleAttack(generations, n_infested_px);
        }
    }
    return forest_changed;
}

static QMutex protect_split;
FMStand *ForestManagementEngine::splitExistingStand(FMStand *stand)
{
    // get a new stand-id
    // make sure that the Id is only used once.
    QMutexLocker protector(&protect_split);
    int new_stand_id = ++mMaxStandId;

    FMUnit *unit = const_cast<FMUnit*> (stand->unit());
    FMStand *new_stand = new FMStand(unit,new_stand_id);

    mUnitStandMap.insert(unit,new_stand);
    mStands.append(new_stand);
    mStandHash[new_stand_id] = new_stand;

    unit->setNumberOfStands( mUnitStandMap.count(unit) );

    mStandLayoutChanged = true;

    return new_stand;
}



} // namespace
