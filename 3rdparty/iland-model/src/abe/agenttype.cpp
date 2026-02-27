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
#include "agenttype.h"
#include "agent.h"
#include "fmstp.h"
#include "forestmanagementengine.h"
#include "fmunit.h"
#include "fmstand.h"
#include "fomescript.h"

#include <QJSEngine>
namespace ABE {

/** @class AgentType
    @ingroup abe
    AgentType implements an abstract agent type (e.g., farmer or forest company). The class defines basic behavior of
    agents.

  */

AgentType::AgentType()
{
    mAllSTPAvailable = false;
}

void AgentType::setupSTP(QJSValue agent_code, const QString agent_name)
{
    mName = agent_name;
    mSTP.clear();
    mJSObj = agent_code;
    if (!agent_code.isObject())
        throw IException(QString("ABE:AgentType:setup: the javascript object for agent '%1' could not be found.").arg(agent_name));
    QJSValue stps = agent_code.property("stp");
    if (!stps.isObject())
        throw IException(QString("ABE:AgentType:setup: the javascript definition of agent '%1' does not have a section for 'stp'.").arg(agent_name));
    QJSValueIterator it(stps);
    while (it.hasNext()) {
        it.next();
        if (it.value().toString() == "_default") {
            mAllSTPAvailable = true;
        } else {
            FMSTP *stp = ForestManagementEngine::instance()->stp(it.value().toString());
            if (!stp)
               throw IException(QString("ABE:AgentType:setup: definition of agent '%1': the STP for mixture type '%2': '%3' is not available.").arg(agent_name).arg(it.name()).arg(it.value().toString()));
            mSTP[it.name()] = stp;
        }
    }

    if (FMSTP::verbose())
        qCDebug(abeSetup) << "setup of agent" << agent_name << mSTP.size() << "links to STPs established.";


}

void AgentType::addSTP(QString stp_name)
{
    FMSTP *stp = ForestManagementEngine::instance()->stp(stp_name);
    if (!stp)
       throw IException(QString("AgentType::addSTP: definition of agent '%1': the STP  '%2' is not available.").arg(mName).arg(stp_name));
    mSTP[stp_name] = stp;

}


Agent *AgentType::createAgent(QString agent_name)
{
    // call the newAgent function in the javascript object assigned to this agent type
    QJSValue func = mJSObj.property("newAgent");
    QJSValue result;
    if (!func.isCallable()) {
        // add a function with minimal code
        mJSObj.setProperty("newAgent",
                           ForestManagementEngine::instance()->scriptEngine()->evaluate("(function() { return {scheduler: this.scheduler } })")
                           );
        func = mJSObj.property("newAgent");

    }
    if (func.isCallable()) {
        result = func.callWithInstance(mJSObj);
        if (result.isError())
            throw IException(QString("calling the 'newAgent' function of agent type '%1' returned with the following error: %2").arg(name()).arg(result.toString()));
    } else {
        // newAgent is not available, run minimal code
        throw IException(QString("The agent type '%1' does not have a valid 'newAgent' function.").arg(name()));
        //result = ForestManagementEngine::instance()->scriptEngine()->evaluate("{ scheduler: this.scheduler }");

    }

    Agent *agent = new Agent(this, result);
    if (!agent_name.isEmpty()) {
        agent->setName(agent_name);
    } else {
        if (result.property("name").isUndefined())
            result.setProperty("name", agent->name()); //  set the auto-generated name also for the JS world
        else
            agent->setName(result.property("name").toString()); // set the JS-name also internally
    }
    ForestManagementEngine::instance()->addAgent(agent);

    return agent;

}

void AgentType::addAgentUpdate(const AgentUpdate &update, FMUnit *unit)
{

    // clear agent updates...
    QMultiHash<const FMUnit*, AgentUpdate>::iterator hi= mAgentChanges.begin();
    while (hi != mAgentChanges.end()) {
        if (!hi.value().isValid())
            hi = mAgentChanges.erase(hi);
        else
            ++hi;
    }


    AgentUpdate &rUpdate = mAgentChanges.insert(unit, update).value();
    rUpdate.setCounter( unit->numberOfStands() );

    // set default unit value
    switch (update.type()) {
    case AgentUpdate::UpdateU: unit->setU( update.value().toInt() ); break;
    case AgentUpdate::UpdateThinning: unit->setThinningIntensity(update.value().toInt()); break;
    case AgentUpdate::UpdateSpecies: break;
    default: break;
    }

    if (update.age()==-1)
        return;

    // check stands that should be updated immediateley
    const QMultiMap<FMUnit*, FMStand*> &stands = ForestManagementEngine::instance()->stands();
    QMultiMap<FMUnit*, FMStand*>::const_iterator it = stands.constFind(const_cast<FMUnit*>(unit));
    while (it != stands.constEnd() && it.key()==unit) {
        FMStand *stand = it.value();
        if (stand->trace())
            qCDebug(abe) << stand->context() << "Agent-update: update if stand-age: " << stand->age() << " < update-age: " << update.age();
        if (stand->age() <= update.age())
            agentUpdateForStand(stand, QString(), stand->age());
        ++it;
    }

}

bool AgentType::agentUpdateForStand(FMStand *stand, QString after_activity, int age)
{
    //
    QMultiHash<const FMUnit*, AgentUpdate>::iterator uit = mAgentChanges.find(stand->unit());
    bool action = false;
    while (uit != mAgentChanges.end() && uit.key()==stand->unit()) {
        AgentUpdate &update = uit.value();

        if (!update.isValid()) {
            ++uit;
            continue;
        }
        // timing of update
        if (!after_activity.isEmpty() && update.afterActivity()==after_activity) {
            // do something
            action = true;
        }
        if (update.age()>-1 && age<update.age()) {
            // do something
            action = true;
        }

        // update the stand
        if (action) {
            update.decrease();
            switch (update.type()) {
            case AgentUpdate::UpdateU: {
                int current_u = stand->U(); // stand->stp()->rotationLengthType(stand->U());
                int new_u = update.value().toInt();
                if (current_u == new_u) {
                    if (stand->trace())
                        qCDebug(abe) << stand->context() << "AgentUpdate: update of U to" << new_u << " not done (value already set).";
                    break;
                }
                stand->setU( new_u );
                // stand->setU( stand->stp()->rotationLengthOfType(new_u) );
                qCDebug(abe) << stand->context() << "AgentUpdate: changed to U" << stand->U();
                // QML like dynamic expressions
                stand->stp()->evaluateDynamicExpressions(stand);
                break;
            }
            case AgentUpdate::UpdateThinning: {
                int current_th = stand->thinningIntensity();
                int new_th = update.value().toInt();
                if (current_th == new_th) {
                    if (stand->trace())
                        qCDebug(abe) << stand->context() << "AgentUpdate: update of thinningIntensity class to" << new_th << " not done (value already set).";
                    break;
                }
                stand->setThinningIntensity(new_th );
                qCDebug(abe) << stand->context() << "AgentUpdate: changed to thinningIntensity class:" << stand->thinningIntensity();
                stand->stp()->evaluateDynamicExpressions(stand);
                break;
            }
            default: break; // TODO: UpdateSpecies???

            }
        }

        ++uit;
    }
    return action;
}

FMSTP *AgentType::stpByName(const QString &name)
{
    if (mSTP.contains(name))
        return mSTP[name];
    if (mAllSTPAvailable) {
        // look globally
        FMSTP *stp = ForestManagementEngine::instance()->stp(name);
        return stp;
    }
    return nullptr; // not found
}

int AgentType::speciesCompositionIndex(const QString &key)
{
    for (int i=0;i<mSpeciesCompositions.size(); ++i)
        if (mSpeciesCompositions[i] == key)
            return i;
    return -1;
}

QString AgentType::speciesCompositionName(const int index)
{
    if (index>=0 && index < mSpeciesCompositions.count())
        return mSpeciesCompositions[index];
    return QString();
}

AgentUpdate::UpdateType AgentUpdate::label(const QString &name)
{
    if (name=="U") return UpdateU;
    if (name=="thinningIntensity") return UpdateThinning;
    if (name=="species") return UpdateSpecies;
    return UpdateInvalid;
}

QString AgentUpdate::dump()
{
    QString line;
    switch (type()) {
    case UpdateU:   line= QString("AgentUpdate: update U to '%1'.").arg(mValue); break;
    case UpdateThinning:   line= QString("AgentUpdate: update thinning interval to '%1'.").arg(mValue); break;
    case UpdateSpecies:   line= QString("AgentUpdate: update species composition to '%1'.").arg(mValue); break;
    default: break;
    }
    if (!mAfterActivity.isEmpty())
        return line + QString("Update after activity '%1'.").arg(mAfterActivity);
    return line + QString("Update (before) age '%1'.").arg(mAge);
}


} // namespace
