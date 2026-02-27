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
#include "agent.h"
#include "agenttype.h"
#include "scheduler.h"
#include "forestmanagementengine.h"
#include "fmstand.h"
#include "fomescript.h"
#include "fmstp.h"
#include "fmunit.h"

namespace ABE {

/** @class Agent
    @ingroup abe
    The Agent class is a lightweight implementation of a single forest manager. Several agents (responsible for particular parts
    of the landscape) may be of the same AgentType and thus share certain characteristics.

  */

int Agent::mAgentsCreated = 0;

Agent::Agent(AgentType *type, QJSValue js)
{
    mType = type;
    mJSAgent = js;
    mAgentsCreated++;
    mName = QString("agent_%1").arg(mAgentsCreated);
}

void Agent::setName(const QString &name)
{
    mName = name;
    mJSAgent.setProperty("name", name);
}

double Agent::useSustainableHarvest() const
{
    return schedulerOptions().useSustainableHarvest;
}

void Agent::setup()
{
    QJSValue scheduler = jsAgent().property("scheduler");
    mSchedulerOptions.setup( scheduler );

    FMSTP *stp = type()->stpByName("default");
    if (!stp) {
        if (mName == "_default") // we do not care if the built-in default agent has an error
            return;
        throw IException("Agent::setup(): default-STP not defined. Could be due to a Javascript error.");
    }

    QJSValue onSelect_handler = type()->jsObject().property("onSelect");

    const QMultiMap<FMUnit*, FMStand*> &stand_map = ForestManagementEngine::instance()->stands();
    foreach (FMUnit *unit, mUnits) {
        QMultiMap<FMUnit*, FMStand*>::const_iterator it = stand_map.constFind(unit);
        unit->setU(stp->rotationLengthOfType(2)); // medium
        unit->scheduler()->setEnabled(mSchedulerOptions.useScheduler);
        while (it!=stand_map.constEnd() && it.key()==unit) {
            FMStand *stand = it.value();
            // check if STP is already assigned. If not, do it now.
            if (!stand->stp()) {
                stand->reload(); // fetch data from iLand ...
                if (onSelect_handler.isCallable()) {
                    FomeScript::setExecutionContext(stand);
                    //QJSValue mix = onSelect_handler.call();
                    QJSValue mix = onSelect_handler.callWithInstance(type()->jsObject());
                    QString mixture_type = mix.toString();
                    if (!type()->stpByName(mixture_type))
                        throw IException(QString("Agent::setup(): the selected mixture type '%1' for stand '%2' is not valid for agent '%3'.").arg(mixture_type).arg(stand->id()).arg(mName));
                    stand->setSTP(type()->stpByName(mixture_type));
                } else {
                    // todo.... some automatic stp selection
                    stand->setSTP(stp);
                }
                stand->setU( unit->U() );
                stand->setThinningIntensity( unit->thinningIntensity() );
                stand->setTargetSpeciesIndex( unit->targetSpeciesIndex() );
                stand->initialize(); // run initialization

            } else {
                // set values that could be altered by STP (such as U)
                if (stand->U()==0.)
                    stand->setU( unit->U() );
                stand->setThinningIntensity( unit->thinningIntensity() );
                stand->setTargetSpeciesIndex( unit->targetSpeciesIndex() );
            }
            ++it;
        }
    }
}

} // namespace
