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
#include "actscheduled.h"

#include "fmstp.h"
#include "fmstand.h"
#include "fomescript.h"
#include "fmtreelist.h"

namespace ABE {

/** @class ActScheduled
    @ingroup abe
    The ActScheduled class is an all-purpose activity (similar to ActGeneral). The execution time of ActScheduled-activities, however,
    is defined by the ABE Scheduler.

  */



ActScheduled::ActScheduled(FMSTP *parent): Activity(parent)
{
    mBaseActivity.setIsScheduled(true); // use the scheduler
    mBaseActivity.setDoSimulate(true); // simulate per default
}

void ActScheduled::setup(QJSValue value)
{
    Activity::setup(value);
    events().setup(value, FomeScript::bridge()->activityJS(), QStringList() << "onEvaluate");

    if (!events().hasEvent(QStringLiteral("onEvaluate")))
        throw IException("activity %1 (of type 'scheduled') requires to have the 'onEvaluate' event.");

}

bool ActScheduled::execute(FMStand *stand)
{
    if (events().hasEvent(QStringLiteral("onExecute"))) {
        // switch off simulation mode
        stand->currentFlags().setDoSimulate(false);
        // execute this event
        bool result =  Activity::execute(stand);
        stand->currentFlags().setDoSimulate(true);
        return result;
    } else {
        // default behavior: process all marked trees (harvest / cut)
        if (stand->trace()) qCDebug(abe) << stand->context() << "activity" << name() << "remove all marked trees.";
        FMTreeList trees(stand);
        trees.removeMarkedTrees();
        return true;
    }
}

bool ActScheduled::evaluate(FMStand *stand)
{
    // this is called when it should be tested
    stand->currentFlags().setDoSimulate(true);
    QJSValue result = events().run(QStringLiteral("onEvaluate"), stand);
    if (stand->trace())
        qCDebug(abe) << stand->context() << "executed onEvaluate event of" << name() << "with result:" << FomeScript::JStoString(result);

    if (result.isNumber()) {
        double harvest = result.toNumber();

        // the return value is interpreted as scheduled harvest; if this value is 0, then no
        if (harvest==0.)
            return false;
        stand->addScheduledHarvest(harvest);
        if (stand->trace())
            qCDebug(abe) << stand->context() << "scheduled harvest is now" << stand->scheduledHarvest();
        return true;
    }
    bool bool_result = result.toBool();
    return bool_result;
}

QStringList ActScheduled::info()
{
    QStringList lines = Activity::info();
    //lines << "this is an activity of type 'scheduled'.";
    return lines;
}


} // namespace
