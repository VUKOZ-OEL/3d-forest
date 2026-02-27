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
#ifndef ACTIVITY_H
#define ACTIVITY_H
#include <QJSValue>
#include <QVector>
#include <QMap>

class Expression; // forward

namespace ABE {


class FMStand;
class FMSTP;


class Schedule {
public:
    // setup and life cycle
    Schedule()  {}
    Schedule(QJSValue &js_value) { clear(); setup(js_value); }
    void clear() { tmin=tmax=topt=-1; tminrel=tmaxrel=toptrel=-1.; force_execution=false; repeat_interval=-1; repeat_start=0; repeat=false; absolute=false; mSignalStr="", mSignalDelta=-1; }
    void setup(const QJSValue &js_value);
    QString dump() const;
    // functions
    /// value() evaluates the schedule for the given 'stand'.
    /// return 0..1 (0: no fit, 1: perfect time).
    /// Special value -1: expired (i.e. current time past the maximum schedule time).
    double value(const FMStand *stand, const int specific_year=-1);
    /// gives (fixed) earliest possible execution time
    double minValue(const double U=100.) const;
    /// returns the latest possible execution time
    double maxValue(const double U=100.) const;
    /// returns the "optimal" year, i.e. the first year when prob. to execute is highest.
    double optimalValue(const double U=100.) const;
    // signals
    bool listensToSignal(const QString &signalstr) { return signalstr == mSignalStr; }
    int signalExecutionDelay(const QString &signalstr) { Q_UNUSED(signalstr); return mSignalDelta; }
    // some stuffs
    int tmin; int tmax; int topt;
    double tminrel; double tmaxrel; double toptrel;
    bool force_execution;
    // repeating
    int repeat_interval;
    int repeat_start;
    bool repeat;
    bool absolute;
private:
    QString mSignalStr;
    int mSignalDelta;
};

class Events {
public:
    Events() {}
    /// clear the list of events
    void clear();
    /// setup events from the javascript object
    void setup(QJSValue &js_value, QJSValue &this_object, QStringList event_names);
    /// execute javascript event /if registered) in the context of the forest stand 'stand'.
    QJSValue run(const QString event, FMStand *stand, QJSValueList *params=0);
    /// returns true, if the event 'event' is available.
    bool hasEvent(const QString &event) const;
    QString dump(); ///< prints some debug info
private:
    QJSValue mInstance; ///< object holding the events
    QMap<QString, QJSValue> mEvents; ///< list of event names and javascript functions
};

/** DynamicExpression encapsulates an "expression" that can be either a iLand expression, a constant or a javascript function.
*/
struct DynamicExpression {
    DynamicExpression(): filter_type(ftInvalid), expr(0) {}
    ~DynamicExpression();
    void setup(const QJSValue &js_value);
    bool evaluate(FMStand *stand) const;
    bool isValid() const { return filter_type!=ftInvalid;}
    QString dump() const;
private:
    enum { ftInvalid, ftExpression, ftJavascript} filter_type;
    Expression *expr;
    QJSValue func;
};

class Constraints {
public:
    Constraints() {}
    void setup(QJSValue &js_value); ///< setup from javascript
    double evaluate(FMStand *stand); ///< run the constraints
    QStringList dump(); ///< prints some debug info
private:
    QList<DynamicExpression> mConstraints;
};

class Activity; //forward
/** Activity meta data (enabled, active, ...) that need to be stored
    per stand. */
class ActivityFlags
{
public:
    ActivityFlags(): mActivity(0), mFlags(0) {}
    ActivityFlags(Activity *act): mActivity(act), mFlags(0) {}
    Activity *activity() const {return mActivity; }

    bool active() const {return flag(Active); }
    bool enabled() const {return flag(Enabled);}
    bool isRepeating() const {return flag(Repeater);}
    bool isPending() const {return flag(Pending); }
    bool isForcedNext() const {return flag(ExecuteNext); }
    bool isFinalHarvest() const {return flag(FinalHarvest); }
    bool isExecuteImmediate() const {return flag(ExecuteImmediate); }
    bool isScheduled() const {return flag(IsScheduled);}
    bool isDoSimulate() const {return flag(DoSimulate);}
    bool isSalvage() const {return flag(IsSalvage);}
    bool manualExit() const {return flag(ManualExit);}


    void setActive(const bool active) { setFlag(Active, active); }
    void setEnabled(const bool enabled) { setFlag(Enabled, enabled); }
    void setIsRepeating(const bool repeat) { setFlag(Repeater, repeat); }
    void setIsPending(const bool pending) { setFlag(Pending, pending); }
    void setForceNext(const bool isnext) { setFlag(ExecuteNext, isnext); }
    void setFinalHarvest(const bool isfinal) { setFlag(FinalHarvest, isfinal); }
    void setExecuteImmediate(const bool doexec) { setFlag(ExecuteImmediate, doexec);}
    void setIsScheduled(const bool doschedule) {setFlag(IsScheduled, doschedule); }
    void setDoSimulate(const bool dosimulate) {setFlag(DoSimulate, dosimulate); }
    void setIsSalvage(const bool issalvage) {setFlag(IsSalvage, issalvage); }
    void setManualExit(const bool isterminate) {setFlag(ManualExit, isterminate); }

private:
    /// (binary coded)  flags
    enum Flags { Active=1,  // if false, the activity has already been executed
                 Enabled=2,  // if false, the activity can not be executed
                 Repeater=4, // if true, the activity is executed
                 ExecuteNext=8, // this activity should be executed next (kind of "goto"
                 ExecuteImmediate=16, // should be executed immediately by the scheduler (e.g. required sanitary cuttings)
                 Pending=32,  // the activity is currently in the scheduling algorithm
                 FinalHarvest=64,  // the management of the activity is a "endnutzung" (compared to "vornutzung")
                 IsScheduled=128, // the execution time of the activity is scheduled by the Scheduler component
                 DoSimulate=256,  // the default operation mode of harvests (simulate or not)
                 IsSalvage=512,   // the activity is triggered by tree mortality events
                 ManualExit=1024 // the activity does not exit activity automatically after execution
                 };
    bool flag(const ActivityFlags::Flags flag) const { return mFlags & flag; }
    void setFlag(const ActivityFlags::Flags flag, const bool value) { if (value) mFlags |= flag; else mFlags &= (flag ^ 0xffffff );}
    Activity *mActivity; ///< link to activity
    int mFlags;
};



/// Activity is the base class for management activities
class Activity
{
public:
    // life cycle
    Activity(const FMSTP *parent);
    virtual ~Activity();
    /// Activity factory - create activities for given 'type'
    static Activity *createActivity(const QString &type, FMSTP *stp);

    // properties
    enum Phase { Invalid, Tending, Thinning, Regeneration, All };
    const FMSTP *program() const { return mProgram; }
    virtual QString type() const;
    QString name() const {return mName; } ///< name of the activity as provided by JS
    QString description() const {return mDescription; } ///< additional description of the acitivity
    int index() const { return mIndex; } ///< index of the activity within the STP
    /// get earlist possible scheduled year (relative to rotation begin)
    int earliestSchedule(const double U=100.) const {return mSchedule.minValue(U); }
    /// get latest possible scheduled year (relative to rotation begin)
    int latestSchedule(const double U=100.) const { return mSchedule.maxValue(U); }
    /// get optimal scheduled year (relative to rotation begin)
    int optimalSchedule(const double U=100.) const { return mSchedule.optimalValue(U); }
    bool isRepeatingActivity() const { return mSchedule.repeat; }
    // main actions
    /// setup of the activity (events, schedule, constraints). additional setup in derived classes.
    virtual void setup(QJSValue value);
    /// returns a value > 0 if the activity coult be scheduled now
    virtual double scheduleProbability(FMStand *stand, const int specific_year=-1);
    /// returns a probability for the activity to be executed (ie all constraints are fulfilled)
    /// return value is 0 if the activity can not be executed (maximum result is 1)
    virtual double execeuteProbability(FMStand *stand);
    /// executes the action (usually defined in derived classes) using the context of 'stand'.
    virtual bool execute(FMStand *stand);
    /// executes the evaluation of the forest stand.
    /// returns true, when the stand should enter the scheduler.
    virtual bool evaluate(FMStand *stand);
    /// function that evaluates "bound" dynamic expressions
    virtual void evaluateDyanamicExpressions(FMStand *stand);
    /// run an event
    void runEvent(const QString &event_name, FMStand *stand);
    /// dumps some information for debugging
    virtual QStringList info();

    /// general purpose and user-defined JS object of an activity
    QJSValue &JSobj() { return mJSObj; }

protected:
    Schedule &schedule()  { return mSchedule; }
    Constraints &constraints()  { return mConstraints; }
    Events &events()  { return mEvents; }
    ActivityFlags &standFlags(FMStand *stand=0);
    ActivityFlags mBaseActivity; // base properties of the activity (that can be changed for each stand)
    static QStringList mAllowedProperties; // list of properties (e.g. 'schedule') that are parsed by the base activity
private:
    void setIndex(const int index) { mIndex = index; } // used during setup
    void setName(const QString &name) { mName = name; }
    int mIndex; ///< index of the activity within the STP
    QString mName; ///< the name of the activity;
    QString mDescription; ///< a more detailed descrption of the activity
    const FMSTP *mProgram; // link to the management programme the activity is part of
    Schedule mSchedule; // timing of activity
    Constraints mConstraints; // constraining factors
    Events mEvents; // action handlers such as "onExecute"
    DynamicExpression mEnabledIf; // enabledIf property (dynamically evaluated)
    QJSValue mJSObj; // general purpose javascript object
    friend class FMSTP; // allow access of STP class to internals
    friend class FMStand; // allow access of the activity class (e.g for events)
    friend class ActivityObj; // allow access to scripting function
};

} // namespace

Q_DECLARE_TYPEINFO(ABE::ActivityFlags, Q_PRIMITIVE_TYPE); // declare as POD structure to allow more efficient copying

#endif // ACTIVITY_H
