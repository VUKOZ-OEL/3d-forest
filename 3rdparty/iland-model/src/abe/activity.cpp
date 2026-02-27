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
#include <QJSValueIterator>

#include "activity.h"
#include "fmstand.h"
#include "fmstp.h"
#include "fomescript.h"
#include "fomewrapper.h"
#include "forestmanagementengine.h"

#include "expression.h"
#include "debugtimer.h"


// include derived activity types
#include "actgeneral.h"
#include "actscheduled.h"
#include "actplanting.h"
#include "actthinning.h"

namespace ABE {

/** @class Activity
    @ingroup abe
    An activity is the basic silvicultural building block; it holds state information and defines basic capabilities
    of all activities (such as having a given Schedule, or Events).

  */

// statics
QStringList Activity::mAllowedProperties = QStringList() << "schedule" << "constraint" << "type"
                                                         << "onCreate" << "onSetup" << "onEnter" << "onExit" << "onExecute" << "onExecuted" << "onCancel";

/***************************************************************************/
/***************************   Schedule  ***********************************/
/***************************************************************************/


void Schedule::setup(const QJSValue &js_value)
{
    clear();
    if (js_value.isObject()) {
        const QStringList allowed = {"min", "max", "opt", "minRel", "maxRel", "optRel",
                               "repeatInterval", "repeatStart", "force", "absolute",
                               "signal", "repeat", "wait" };

        FMSTP::checkObjectProperties(js_value, allowed, "Schedule" );

        tmin = FMSTP::valueFromJs(js_value, "min", "-1").toInt();
        tmax = FMSTP::valueFromJs(js_value, "max", "-1").toInt();
        topt = FMSTP::valueFromJs(js_value, "opt", "-1").toInt();
        tminrel = FMSTP::valueFromJs(js_value, "minRel", "-1").toNumber();
        tmaxrel = FMSTP::valueFromJs(js_value, "maxRel", "-1").toNumber();
        toptrel = FMSTP::valueFromJs(js_value, "optRel", "-1").toNumber();
        repeat_interval = FMSTP::valueFromJs(js_value, "repeatInterval", "1").toInt();
        repeat_start = FMSTP::valueFromJs(js_value, "repeatStart", "0").toInt();
        // switches
        force_execution = FMSTP::boolValueFromJs(js_value, "force", false);
        repeat = FMSTP::boolValueFromJs(js_value, "repeat", false);
        absolute = FMSTP::boolValueFromJs(js_value, "absolute", false);
        // signals
        mSignalStr = FMSTP::valueFromJs(js_value, "signal").toString();
        if (mSignalStr == "undefined") mSignalStr.clear();
        if (!mSignalStr.isEmpty())
            mSignalDelta = FMSTP::valueFromJs(js_value, "wait","0").toInt();

        if (!repeat && mSignalStr.isEmpty()) {

            if (tmin>-1 && tmax>-1 && topt==-1)
                topt = (tmax+tmin) / 2;
            if (tmin>-1 && tmax>-1 && topt>-1 && (topt<tmin || topt>tmax))
                throw IException(QString("Error in setting up schedule: 'opt' either missing or out of range: %1").arg(FomeScript::JStoString(js_value)));
            if (tminrel>-1 && tmaxrel>-1 && toptrel>-1 && (toptrel<tminrel || toptrel>tmaxrel))
                throw IException(QString("Error in setting up schedule: 'opt' either missing or out of range: %1").arg(FomeScript::JStoString(js_value)));
            if (tminrel*tmaxrel < 0. || tmin*tmax<0.)
                throw IException(QString("Error in setting up schedule: min and max required: %1").arg(FomeScript::JStoString(js_value)));

            if (topt==-1 && toptrel==-1.)
                throw IException(QString("Error in setting up schedule: neither 'opt' nor 'optRel' point can be derived in: %1").arg(FomeScript::JStoString(js_value)));
        }

    } else if (js_value.isNumber()) {
        topt = js_value.toNumber();
    } else {
        throw IException(QString("Error in setting up schedule/timing. Invalid javascript object: %1").arg(FomeScript::JStoString(js_value)));
    }
}

QString Schedule::dump() const
{
    if (repeat)
        return QString("schedule: Repeating every %1 years.").arg(repeat_interval);
    else
        return QString("schedule: tmin/topt/tmax %1/%2/%3\nrelative: min/opt/max %4/%5/%6\nforce: %7").arg(tmin).arg(topt).arg(tmax)
                .arg(tminrel).arg(toptrel).arg(tmaxrel).arg(force_execution);
}

double Schedule::value(const FMStand *stand, const int specific_year)
{
    double U = stand->U();
    double current;
    double current_year = ForestManagementEngine::instance()->currentYear();
    if (specific_year>=0)
        current_year = specific_year;
    // absolute age: years since the start of the rotation
    current = stand->absoluteAge();

    if (absolute)
        current = current_year;

    double current_rel = current / U;
    if (repeat) {
        // handle special case of repeating activities.
        // we execute the repeating activity if repeatInterval years elapsed
        // since the last execution. The first execution is in year "repeat_start"
        if (int(current_year)>=repeat_start &&  (int(current_year) - repeat_start) % repeat_interval == 0)
            return 1.; // yes, execute this year
        else
            return 0.; // do not execute this year.

    }
    // force execution: if age already higher than max, then always evaluate to 1.
    if (tmax>-1. && current >= tmax && force_execution)
        return 1;
    if (tmaxrel>-1. && current_rel >= tmaxrel && force_execution)
        return 1;

    if (tmin>-1. && current < tmin) return 0.;
    if (tmax>-1. && current > tmax) return -1.; // expired
    if (tminrel>-1. && current_rel < tminrel) return 0.;
    if (tmaxrel>-1. && current_rel > tmaxrel) return -1.; // expired

    // optimal time
    if (topt > -1. && fabs(current-topt) <= 0.5)
        return 1;
    if (topt > -1. && current > topt) {
        if (force_execution)
            return 1.;
        else
            return -1.; // expired
    }

    if (tmin>-1. && tmax > -1.) {
        if (topt > -1.) {
            // linear interpolation
            if (current<=topt)
                return topt==tmin?1.:(current-tmin)/(topt-tmin);
            if (force_execution)
                return 1.; // keep the high probability.
            else
                return topt==tmax?1.:(tmax-current)/(tmax-topt); // decreasing probabilitiy again
        } else {
            return 1.; // no optimal time: everything between min and max is fine!
        }
    }
    // there is an optimal absoulte point in time defined, but not reached
    if (topt > -1)
        return 0.;

    // optimal time
    if (toptrel>-1. && fabs(current_rel-toptrel)*U <= 0.5)
        return 1.;

    // min/max relative time
    if (tminrel>-1. && tmaxrel>-1.) {
        if (toptrel > -1.) {
            // linear interpolation
            if (current_rel<=toptrel)
                return toptrel==tminrel?1.:(current_rel-tminrel)/(toptrel-tminrel);
            else
                return toptrel==tmaxrel?1.:(tmaxrel-current_rel)/(tmaxrel-toptrel);
        } else {
            return 1.; // no optimal time: everything between min and max is fine!
        }
    }
    // there is an optimal relative point in time defined, but not reached yet.
    if (toptrel>-1.)
        return 0.;

    qCDebug(abe) << "Schedule::value: unexpected combination. U" << U << "age" << current << ", schedule:" << this->dump();
    return 0.;
}

double Schedule::minValue(const double U) const
{
    if (absolute && tmin>-1) return tmin;
    if (repeat || !mSignalStr.isEmpty()) return 100000000.;
    if (tmin>-1) return tmin;
    if (tminrel>-1.) return tminrel * U; // assume a fixed U of 100yrs
    if (repeat) return -1.; // repeating executions are treated specially
    if (topt>-1) return topt;
    return toptrel * U;
}

double Schedule::maxValue(const double U) const
{
    if (!mSignalStr.isEmpty()) return 1000000000;
    if (absolute && tmax>-1) return tmax;
    if (tmax>-1) return tmax;
    if (tmaxrel>-1.) return tmaxrel * U; // assume a fixed U of 100yrs
    if (repeat) return -1.; // repeating executions are treated specially
    if (topt>-1) return topt;
    return toptrel * U;

}

double Schedule::optimalValue(const double U) const
{
    if (!mSignalStr.isEmpty()) return 1000000000;
    if (topt>-1) return topt;
    if (toptrel>-1) return toptrel*U;
    if (tmin>-1 && tmax>-1) return (tmax+tmin)/2.;
    if (tminrel>-1 && tmaxrel>-1) return (tmaxrel+tminrel)/2. * U;
    if (force_execution) return tmax;
    return toptrel*U;
}

/***************************************************************************/
/**************************     Events  ************************************/
/***************************************************************************/

void Events::clear()
{
    mEvents.clear();
}

void Events::setup(QJSValue &js_value, QJSValue &this_object, QStringList event_names)
{
    mInstance = this_object; //
    foreach (QString event, event_names) {
        QJSValue val = FMSTP::valueFromJs(js_value, event);
        if (val.isCallable()) {
            mEvents[event] = js_value; // save the event functions (and the name of the property that the function is assigned to)
        }
    }
}

QJSValue Events::run(const QString event, FMStand *stand, QJSValueList *params)
{
    if (mEvents.contains(event)) {
        if (stand)
            FomeScript::setExecutionContext(stand);
        QJSValue func = mEvents[event].property(event);
        QJSValue result;
        if (func.isCallable()) {
            DebugTimer t("ABE:JSEvents:run");

            if (params)
                result = func.callWithInstance(mInstance, *params);
            else
                result = func.callWithInstance(mInstance);
            if (FMSTP::verbose() || (stand && stand->trace()))
                qCDebug(abe) << (stand?stand->context():QString("<no stand>")) << "invoking javascript event" << event << " result: " << result.toString();
        }

        //qDebug() << "event called:" << event << "result:" << result.toString();
        if (result.isError()) {
            throw IException(QString("%3 Javascript error in event %1: %2. \n %4")
                                 .arg(event)
                                 .arg(result.toString())
                                 .arg(stand?stand->context():"----")
                                 .arg(ScriptGlobal::formattedErrorMessage(result)));
        }
        return result;
    }
    return QJSValue();
}

bool Events::hasEvent(const QString &event) const
{
    return mEvents.contains(event);
}

QString Events::dump()
{
    QString event_list = "Registered events: ";
    foreach (QString event, mEvents.keys())
        event_list.append(event).append(" ");
    return event_list;
}

/***************************************************************************/
/*************************  Constraints  ***********************************/
/***************************************************************************/

void Constraints::setup(QJSValue &js_value)
{
    mConstraints.clear();
    if ((js_value.isArray() || js_value.isObject()) && !js_value.isCallable()) {
        QJSValueIterator it(js_value);
        while (it.hasNext()) {
            it.next();
            if (it.name()==QStringLiteral("length"))
                continue;
            mConstraints.append(DynamicExpression());
            DynamicExpression &item = mConstraints.last();
            item.setup(it.value());
        }
    } else {
        mConstraints.append(DynamicExpression());
        DynamicExpression &item = mConstraints.last();
        item.setup(js_value);

    }
}

double Constraints::evaluate(FMStand *stand)
{
    if (mConstraints.isEmpty())
        return 1.; // no constraints to evaluate
    double p;
    double p_min = 1;
    for (int i=0;i<mConstraints.count();++i) {
        p = mConstraints.at(i).evaluate(stand);
        if (p == 0.) {
            if (stand->trace())
                qCDebug(abe) << stand->context() << "constraint" << mConstraints.at(i).dump() << "did not pass.";
            return 0.; // one constraint failed
        } else {
            // save the lowest value...
            p_min = std::min(p, p_min);
        }
    }
    return p_min; // all constraints passed, return the lowest returned value...
}

QStringList Constraints::dump()
{
    QStringList info;
    for (int i=0;i<mConstraints.count();++i){
        info << QString("constraint: %1").arg(mConstraints[i].dump());
    }
    return info;
}


DynamicExpression::~DynamicExpression()
{
    if (expr)
        delete expr;
}

void DynamicExpression::setup(const QJSValue &js_value)
{
    filter_type = ftInvalid;
    if (expr) delete expr;
    expr=0;

    if (js_value.isCallable()) {
        func = js_value;
        filter_type = ftJavascript;
        return;
    }
    if (js_value.isString()) {
        // we assume this is an expression

        QString exprstr = js_value.toString();
        // replace "." with "__" in variables (our Expression engine is
        // not able to cope with the "."-notation
        exprstr = exprstr.replace("activity.", "activity__");
        exprstr = exprstr.replace("stand.", "stand__");
        exprstr = exprstr.replace("site.", "site__");
        exprstr = exprstr.replace("unit.", "unit__");
        // add ....
        expr = new Expression(exprstr);
        filter_type = ftExpression;
        return;

    }
}

bool DynamicExpression::evaluate(FMStand *stand) const
{
    switch (filter_type) {
    case ftInvalid: return true; // message?
    case ftExpression: {
            FOMEWrapper wrapper(stand);
            double result;
            try {
                result = expr->execute(0, &wrapper); // using execute, we're in strict mode, i.e. wrong variables are reported.
                //result = expr->calculate(wrapper);
            } catch (IException &e) {
                // throw a nicely formatted error message
                e.add(QString("in filter (expr: '%2') for stand %1.").
                              arg(stand->id()).
                              arg(expr->expression()) );
                throw;
            }

            if (FMSTP::verbose())
                qCDebug(abe) << stand->context() << "evaluate constraint (expr:" << expr->expression() << ") for stand" << stand->id() << ":" << result;
            return result > 0.;

        }
    case ftJavascript: {
        // call javascript function
        // provide the execution context
        FomeScript::setExecutionContext(stand);
        QJSValue result = const_cast<QJSValue&>(func).call();
        if (result.isError()) {
            throw IException(QString("Erron in evaluating constraint  (JS) for stand %1: %2").
                             arg(stand->id()).
                             arg(result.toString()));
        }
        if (FMSTP::verbose())
            qCDebug(abe) << "evaluate constraint (JS) for stand" << stand->id() << ":" << result.toString();
        // convert boolean result to 1 - 0
        if (result.isBool())
            return result.toBool()==true?1.:0;
        else
            return result.toNumber();
    }

    }
    return true;
}

QString DynamicExpression::dump() const
{
    switch (filter_type){
    case ftInvalid: return "Invalid";
    case ftExpression: return expr->expression();
    case ftJavascript: return func.toString();
    default: return "invalid filter type!";
    }
}



/***************************************************************************/
/***************************  Activity  ************************************/
/***************************************************************************/

Activity::Activity(const FMSTP *parent)
{
    mProgram = parent;
    mIndex = 0;
    mBaseActivity = ActivityFlags(this);
    mBaseActivity.setActive(true);
    mBaseActivity.setEnabled(true);
}

Activity::~Activity()
{

}

Activity *Activity::createActivity(const QString &type, FMSTP *stp)
{
    Activity *act = 0;

    if (type=="general")
        act = new ActGeneral(stp);

    if (type=="scheduled")
        act = new ActScheduled(stp);

    if (type=="planting")
        act = new ActPlanting(stp);

    if (type=="salvage")
        act = new ActSalvage(stp);

    if (type=="thinning")
        act = new ActThinning(stp);

    if (!act) {
        throw IException(QString("Error: the activity type '%1' is not a valid type.").arg(type));
    }

    return act;
}

QString Activity::type() const
{
    return "base";
}

void Activity::setup(QJSValue value)
{
    mSchedule.setup(FMSTP::valueFromJs(value, "schedule", "", "setup activity"));
    if (FMSTP::verbose())
        qCDebug(abeSetup) << mSchedule.dump();

    if (isRepeatingActivity())
        mBaseActivity.setIsScheduled(false);

    // setup of events
    mEvents.clear();
    mEvents.setup(value, FomeScript::bridge()->activityJS(), QStringList() << "onCreate" << "onSetup" << "onEnter" << "onExit" << "onExecute" << "onExecuted" << "onCancel");
    if (FMSTP::verbose())
        qCDebug(abeSetup) << "Events: " << mEvents.dump();

    // setup of constraints
    QJSValue constraints = FMSTP::valueFromJs(value, "constraint");
    if (!constraints.isUndefined())
        mConstraints.setup(constraints);

    // enabledIf property
    QJSValue enabled_if = FMSTP::valueFromJs(value, "enabledIf");
    if (!enabled_if.isUndefined())
        mEnabledIf.setup(enabled_if);

    QJSValue description = FMSTP::valueFromJs(value, "description");
    mDescription = description.toString();

    // initial value for the general purpose JS object
    mJSObj = value.property("obj");
}

double Activity::scheduleProbability(FMStand *stand, const int specific_year)
{
    // return a value between 0 and 1; return -1 if the activity is expired.
    return schedule().value(stand, specific_year);
}

double Activity::execeuteProbability(FMStand *stand)
{
    // check the standard constraints and return true when all constraints are fulfilled (or no constraints set)
    return constraints().evaluate(stand);
}

bool Activity::execute(FMStand *stand)
{
    // execute the "onExecute" event
    events().run(QStringLiteral("onExecute"), stand);
    return true;
}

bool Activity::evaluate(FMStand *stand)
{
    // execute the "onEvaluate" event: the execution is canceled, if the function returns false.
    bool cancel = events().run(QStringLiteral("onEvaluate"), stand).toBool();
    return !cancel;
}

void Activity::evaluateDyanamicExpressions(FMStand *stand)
{
    // evaluate the enabled-if property and set the enabled flag of the stand (i.e. the ActivityFlags)
    if (mEnabledIf.isValid()) {
        bool result = mEnabledIf.evaluate(stand);
        stand->flags(mIndex).setEnabled(result);
    }
}

void Activity::runEvent(const QString &event_name, FMStand *stand)
{
    events().run(event_name, stand);
}

QStringList Activity::info()
{
    QStringList lines;
    lines << QString("Activity '%1': type '%2'").arg(name()).arg(type());
    if (!description().isEmpty())
        lines << "Details: " << description() << "/-";
    lines << "Events" << "-" << events().dump() << "/-";
    lines << "Schedule" << "-" << schedule().dump() << "/-";
    lines << "Constraints" << "-" << constraints().dump() << "/-";
    return lines;
}


ActivityFlags &Activity::standFlags(FMStand *stand)
{
    // use the base data item if no specific stand is provided
    if (!stand)
        return mBaseActivity;

    // return the flags associated with the specific stand
    return stand->flags(mIndex);
}


} // namespace

