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
#include "actgeneral.h"

#include "fmstp.h"
#include "fmstand.h"
#include "fomescript.h"

namespace ABE {

/** @class ActGeneral
    @ingroup abe
    The ActGeneral class is an all-purpose activity and implements no specific forest management activity.

  */


QStringList ActGeneral::info()
{
    QStringList lines = Activity::info();
    lines << "this is the 'general' activity; the activity is not scheduled. Use the action-slot to define what should happen.";
    return lines;
}

void ActGeneral::setup(QJSValue value)
{
    Activity::setup(value);
    // specific
    mAction = FMSTP::valueFromJs(value, "action", "", "Activity of type 'general'.");
    if (!mAction.isCallable())
        throw IException("'general' activity has not a callable javascript 'action'.");
}

bool ActGeneral::execute(FMStand *stand)
{
    FomeScript::setExecutionContext(stand);
    if (FMSTP::verbose() || stand->trace())
        qCDebug(abe) << stand->context() << "activity 'general': execute of" << name();

    QJSValue result = mAction.call();
    if (result.isError()) {
        throw IException(QString("%1 Javascript error in 'general' activity '%3': %2").arg(stand->context()).arg(result.toString()).arg(name()));
    }
    return result.toBool();
}


} // namespace
