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
#ifndef FORESTMANAGEMENTENGINE_H
#define FORESTMANAGEMENTENGINE_H
#include <QMultiMap>
#include <QVector>
#include <QJSValue>

#include "abegrid.h"

class QJSEngine; // forward
class MapGrid; // forward
class ResourceUnit; // forward
class Tree; // forward


namespace ABE {

class FMUnit; // forward
class FMStand; // forward
class FMSTP; // forward
class Agent; // forward
class AgentType; // forward
class FomeScript; // forward
class Activity; // forward

/// the ForestManagementEngine is the container for the agent based forest management engine.
class ForestManagementEngine
{
public:
    // life cycle
    ForestManagementEngine();
    ~ForestManagementEngine();
    // engine instance (singleton)
    static ForestManagementEngine *instance() { if (singleton_fome_engine) return singleton_fome_engine; singleton_fome_engine = new ForestManagementEngine; return singleton_fome_engine; }
    /// link to stand grid
    static const MapGrid *standGrid();

    // setup
    void setup(); ///< setup data structures
    void initialize(); ///< run initial stp
    void clear(); ///< delete all objects and free memory
    void abortExecution(const QString &message);
    bool isCancel() const { return mCancel; }
    void runOnInit(bool before_init); ///< run javascript code that can be used to initialize forest stands

    // main function
    void run(int debug_year=-1);
    /// write outputs, finalize the current year
    /// should be called at the end of the year (after all disturbances, ...)
    void yearEnd();

    // properties
    int currentYear() { return mCurrentYear; }
    bool enabled() const { return mEnabled; }
    void setEnabled(bool enable) {mEnabled=enable; }
    /// access to the "global" Javascript engine
    static QJSEngine *scriptEngine();
    FomeScript *scriptBridge() const {return mScriptBridge; }

    // setting up agents and stps
    /// add a stand treatment programme to the list of programs.
    void addSTP(FMSTP* stp) { mSTP.push_back(stp);}
    /// add an agent type (called from JS)
    void addAgentType(AgentType* at) { mAgentTypes.append(at);}
    /// add an agent (called from JS)
    void addAgent(Agent *agent) { mAgents.append(agent);}
    /// return the agent type with the name 'name' or NULL
    AgentType *agentType(const QString &name);
    /// return the Agent with the name 'name' or NULL
    Agent *agent(const QString &name);


    /// retrieve pointer to stand treatment programme. return 0-pointer if not available.
    FMSTP *stp(QString stp_name) const;
    /// get stand with id 'stand_id'. Return 0 if not found.
    FMStand *stand(int stand_id) const;
    //QVector<FMStand*> stands() const {return mStands; }
    const QMultiMap<FMUnit*, FMStand*> &stands() const {return mUnitStandMap; }
    const QVector<FMUnit*> &units() const { return mUnits; }
    const QVector<FMSTP*> &stps() const { return mSTP; }
    QVariantList standIds() const;

    FMStand *standAt(QPointF coord) const { return mFMStandGrid.constValueAt(coord); }
    // functions

    void addRepeatJS(int stand_id, QJSValue obj, QJSValue callback, int repeatInterval=1, int repeatTimes=-1);
    void addRepeatActivity(int stand_id, Activity *act, int repeatInterval=1, int repeatTimes=-1, QJSValue parameter=QJSValue());
    void stopRepeat(int stand_id, QJSValue obj);
    // run advanced repeated operations
    void runRepeatedItems(int stand_id);


    /// called by iLand for every tree that is removed/harvested/died due to disturbance.
    void notifyTreeRemoval(Tree* tree, int reason);
    /// called when bark beetle are likely going to spread
    bool notifyBarkbeetleAttack(const ResourceUnit *ru, const double generations, int n_infested_px);

    ///
    FMStand *splitExistingStand(FMStand *stand);

    /// evalaute forest management activities and select fitting activities for each forest stand
    void test();
    QStringList evaluateClick(const QPointF coord, const QString &grid_name);


private:
    static int mMaxStandId;
    void setupScripting();
    void prepareRun();
    void finalizeRun();
    void setupOutputs();
    void runJavascript(bool after_processing);



    static ForestManagementEngine *singleton_fome_engine;
    int mCurrentYear; ///< current year of the simulation (=year of the model)

    QVector<FMSTP*> mSTP;

    // scripting bridge (accessing model properties from javascript)
    FomeScript *mScriptBridge;

    // forest management units
    QVector<FMUnit*> mUnits; ///< container for forest management units
    // mapping of stands to units
    QMultiMap<FMUnit*, FMStand*> mUnitStandMap;
    QVector<FMStand*> mStands;
    QHash<int, FMStand*> mStandHash;

    // advanced repeating operations
    struct SRepeatItem {
        SRepeatItem(): interval(1), times(-1), N(0), waitYears(1),activity(nullptr) {}
        SRepeatItem(int ainterval, int atimes, QJSValue &aobj, QJSValue &acallback): interval(ainterval), times(atimes),
            N(0), waitYears(ainterval), jsobj(aobj), callback(acallback), activity(nullptr) {}
        SRepeatItem(int ainterval, int atimes, Activity *act, QJSValue aparam=QJSValue()): interval(ainterval), times(atimes),
            N(0), waitYears(ainterval), parameter(aparam), activity(act) {}

        int interval;
        int times;
        int N; // times already repeated
        int waitYears; // years until next execution
        QJSValue jsobj; // this object (for JS calls)
        QJSValue callback; // callback function (for JS)
        QJSValue parameter; // signal parameter
        Activity *activity; // activity to execute
    };
    bool runSingleRepeatedItem(int stand_id, SRepeatItem &item);

    QMultiHash<int, SRepeatItem> mRepeatStore; ///< store multiple repeat-items per stand (int)
    QList<QPair< int, SRepeatItem> >* mRepeatStoreBuffer {nullptr}; // used to store elements while iterating

    // agents
    QVector<AgentType*> mAgentTypes; ///< collection of agent types
    QVector<Agent*> mAgents; ///< collection of all agents (individuals)

    // grids, visuals, etc.
    Grid<FMStand*> mFMStandGrid;
    ABELayers mStandLayers;

    bool mCancel;
    bool mEnabled; ///< active or paused
    bool mStandLayoutChanged;
    QString mLastErrorMessage;

    //
    friend class UnitOut;
};


} // namespace
#endif // FORESTMANAGEMENTENGINE_H
