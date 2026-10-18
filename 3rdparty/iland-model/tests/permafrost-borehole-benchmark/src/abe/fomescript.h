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

#ifndef FOMESCRIPT_H
#define FOMESCRIPT_H

#include <QObject>
#include <QStringList>
#include <QVariantList>

#include "fmstand.h"
#include "fmunit.h"
#include "scripttree.h"

namespace ABE {

class StandObj;
class UnitObj;
class SimulationObj;
class SchedulerObj;
class STPObj;
class FMTreeList; // forward
class FMSTP; // forward
class ActivityObj;

/// FomeScript provides general helping functions for the Javascript world.
/// the object is known as 'fmengine'.
class FomeScript : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool verbose READ verbose WRITE setVerbose)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(int standId READ standId WRITE setStandId)
    Q_PROPERTY(QString standVisualization READ standVisualization WRITE setStandVisualization)
    Q_PROPERTY(QVariantList standIds READ standIds)
public:
    explicit FomeScript(QObject *parent = nullptr);
    ~FomeScript();
    // prepare scripting features
    void setupScriptEnvironment();
    // functions
    /// prepares the context for executing javascript functions
    /// by setting up all internal structures for the forest stand 'stand'.
    static void setExecutionContext(FMStand *stand, bool add_agent=false);

    /// special function for setting context without a valid stand
    static void setActivity(Activity *act);

    /// static accessor function for the responsible script bridge
    static FomeScript *bridge();
    /// returns a string for debug/trace messages
    const QString &context() const { return mStand?mStand->context():mInvalidContext; }

    /// convert a javascript object to a string (for debug output)
    static QString JStoString(QJSValue value);


    StandObj *standObj() const { return mStandObj; }
    UnitObj *siteObj() const { return mUnitObj; }
    FMTreeList *treesObj() const { return mTrees; }
    ActivityObj *activityObj() const { return mActivityObj; }
    /// get a JS-Object referencing a single tree
    QJSValue treeRef(Tree *tree);

    // Properties
    /// verbose: when true, the logging intensity is increased significantly.
    bool verbose() const;
    void setVerbose(bool arg);

    int standId() const;
    void setStandId(int new_stand_id);

    bool enabled() const;
    void setEnabled(bool enable);


    /// a list of all stand-ids that are currently in the landscape
    QVariantList standIds();

signals:

public slots:
    /// logging function (which includes exeuction context)
    void log(QJSValue value);
    /// abort execution
    void abort(QJSValue message);
    /// adds a management program (STP) that is provided as the Javascript object 'program'. 'name' is used internally.
    bool addManagement(QJSValue program, QString name);
    /// set the STP with the name 'name' to the (new) 'program'. This reloads the STP definition (and all activities).
    /// if 'name' is not present, nothing happens.
    bool updateManagement(QJSValue program, QString name);
    /// add a certain stp (given by 'name') to the agent 'agentname'. Returns false if either stp or agent were not found.
    bool addManagementToAgentType(QString name, QString agentname);
    /// add an agent definition (Javascript). 'name' is used internally. Returns true on success.
    bool addAgentType(QJSValue program, QString name);
    /// create an agent of type 'agent_type' (the name of an agent type) and give the name 'agent_name'. 'agent_name' needs to be unique.
    QJSValue addAgent(QString agent_type, QString agent_name);

    /// create an agent with name 'name' and implicitly an AgentType with the same name
    QJSValue addAgent(QJSValue program, QString name);
    /// executes an activity for stand 'stand_id'. This bypasses the normal scheduling (useful for debugging/testing).
    /// returns false if activity could not be found for the stand.
    bool runActivity(int stand_id, QString activity);
    /// executes an the "evaluate" part of the activity for stand 'stand_id'. This bypasses the normal scheduling (useful for debugging/testing).
    /// returns false if activity could not be found for the stand.
    bool runActivityEvaluate(int stand_id, QString activity);

    /// execute 'function' of the agent for the given stand; this is primarily aimed at testing/debugging.
    bool runAgent(int stand_id, QString function);
    // special functions
    bool isValidStand(int stand_id);

    QJSValue activity(QString stp_name, QString activity_name);

    void runPlanting(int stand_id, QJSValue planting_item);

    // just for testing
    QJSValue test(QJSValue val);
public:
    static int levelIndex(const QString &level_label);
    static const QString levelLabel(const int level_index);
    QString standVisualization() const { return mStandVisualization; }
    void setStandVisualization(QString vis) { mStandVisualization = vis; }

private:
    static QString mInvalidContext;
    const FMStand *mStand;
    StandObj *mStandObj;
    UnitObj *mUnitObj;
    SimulationObj *mSimulationObj;
    ActivityObj *mActivityObj;
    FMTreeList *mTrees;
    SchedulerObj *mSchedulerObj;
    STPObj *mSTPObj;
    QString mLastErrorMessage;
    QString mStandVisualization;
    QJSValue mTreeValue;
    ScriptTree mTree;


};

/// StandObj is the bridge to stand variables from the Javascript world
class StandObj: public QObject
{
    Q_OBJECT
    Q_PROPERTY (bool trace READ trace WRITE setTrace)
    Q_PROPERTY (QJSValue agent READ agent)
    Q_PROPERTY (double basalArea READ basalArea)
    Q_PROPERTY (double age READ age)
    Q_PROPERTY (double absoluteAge READ absoluteAge WRITE setAbsoluteAge)
    Q_PROPERTY (double volume READ volume)
    Q_PROPERTY (double height READ height)
    Q_PROPERTY (double topHeight READ topHeight)
    Q_PROPERTY (int id READ id)
    Q_PROPERTY (int nspecies READ nspecies)
    Q_PROPERTY (double area READ area)
    Q_PROPERTY (QRectF rectangle READ rectangle)
    Q_PROPERTY (int elapsed READ timeSinceLastExecution)
    Q_PROPERTY (QString lastActivity READ lastActivity)

    Q_PROPERTY (double U READ rotationLength WRITE setRotationLength)
    Q_PROPERTY(QString speciesComposition READ speciesComposition )
    Q_PROPERTY(QString thinningIntensity READ thinningIntensity )

    Q_PROPERTY(QString stp READ stp WRITE setStp)


/*    basalArea: 0, // total basal area/ha of the stand
    volume: 100, // total volume/ha of the stand
    speciesCount: 3, // number of species present in the stand with trees > 4m
    age: 100, // "age" of the stand (in relation to "U")
    flags: {}*/
public slots:
    /// basal area of a given species (m2/ha) given by Id.
    double speciesBasalAreaOf(QString species_id) const {return mStand->basalArea(species_id); }
    double relSpeciesBasalAreaOf(QString species_id) const {return mStand->relBasalArea(species_id); }
    double speciesBasalArea(int index) const { if (index>=0 && index<nspecies()) return mStand->speciesData(index).basalArea; else return 0.; }
    double relSpeciesBasalArea(int index) const { if (index>=0 && index<nspecies()) return mStand->speciesData(index).relBasalArea; else return 0.; }
    QString speciesId(int index) const;

    // set and get standspecific data (persistent!)
    void setFlag(const QString &name, QJSValue value){ const_cast<FMStand*>(mStand)->setProperty(name, value);}
    QJSValue flag(const QString &name) { return const_cast<FMStand*>(mStand)->property(name); }
    QJSValue activity(QString name);
    QJSValue agent();

    // actions
    /// force a reload of the stand data.
    void reload() { if (mStand) mStand->reload(true); }
    void sleep(int years) { if (mStand) mStand->sleep(years); }

    void setAbsoluteAge(double arg);
    /// start the management program again (initialize the stand)
    void reset();

public:
    explicit StandObj(QObject *parent = 0): QObject(parent), mStand(0) {}
    // system stuff
    void setStand(FMStand* stand) { mStand = stand; }
    bool trace() const;
    void setTrace(bool do_trace);

    // properties of the forest
    double basalArea() const { if (mStand)return mStand->basalArea(); throwError("basalArea"); return -1.;}
    double height() const { if (mStand)return mStand->height(); throwError("height"); return -1.;}
    double topHeight() const { if (mStand)return mStand->topHeight(); throwError("topHeight"); return -1.;}
    double age() const {if (mStand)return mStand->age(); throwError("age"); return -1.;}
    double absoluteAge() const {if (mStand)return mStand->absoluteAge(); throwError("absoluteAge"); return -1.;  }
    double volume() const {if (mStand) return mStand->volume(); throwError("volume"); return -1.; }
    int id() const { if (mStand) return mStand->id(); throwError("id"); return -1; }
    int nspecies() const {if (mStand) return mStand->nspecies();  throwError("id"); return -1;}
    double area() const {if (mStand) return mStand->area();  throwError("area"); return -1;}
    QRectF rectangle() const;
    int timeSinceLastExecution() const;
    QString lastActivity() const;
    double rotationLength() const;
    void setRotationLength(int new_length);
    QString speciesComposition() const;
    QString thinningIntensity() const;
    QString stp() const;
    void setStp(QString stp_name);




private:
    void throwError(QString msg) const;
    FMStand *mStand;
};

/** @brief The UnitObj class is the Javascript object known as 'unit' in JS and represents
 * a management unit.
*/
class UnitObj: public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString harvestMode READ harvestMode)
    Q_PROPERTY(QString speciesComposition READ speciesComposition )
    Q_PROPERTY(double U READ U )
    Q_PROPERTY(QString thinningIntensity READ thinningIntensity )
    // performance indicators
    Q_PROPERTY(double MAIChange READ MAIChange )
    Q_PROPERTY(double MAILevel READ MAILevel )
    Q_PROPERTY(double landscapeMAI READ landscapeMAI )
    Q_PROPERTY(double mortalityChange READ mortalityChange )
    Q_PROPERTY(double mortalityLevel READ mortalityLevel )
    Q_PROPERTY(double regenerationChange READ regenerationChange )
    Q_PROPERTY(double regenerationLevel READ regenerationLevel )

public slots:
    /// main function to provide agent decisions to the engine
    bool agentUpdate(QString what, QString how, QString when);
    /// force an out-of-schedule update of the management plan
    void updateManagementPlan();
public:
    explicit UnitObj(QObject *parent = 0): QObject(parent) {}
    void setStand(const FMStand* stand) { mStand = stand; }
    QString harvestMode() const;
    QString speciesComposition() const;
    double U() const;
    QString thinningIntensity() const;

    // performance indicators
    double MAIChange() const;
    double MAILevel() const;
    double landscapeMAI() const;
    double mortalityChange() const;
    double mortalityLevel() const;
    double regenerationChange() const;
    double regenerationLevel() const;


private:
    const FMStand *mStand;

};

/** @brief The SimulationObj encapsulates the 'simulation' object in JS. The 'simulation' object
 * is used for global scenarios (e.g., changes in timber price).
*/
class SimulationObj: public QObject
{
    Q_OBJECT
    Q_PROPERTY (double timberPriceIndex READ timberPriceIndex)
public:
    explicit SimulationObj(QObject *parent = 0): QObject(parent) {}
    double timberPriceIndex() const { return 1.010101; } // dummy
private:

};

/** @brief The STPObj encapsulates the 'stp' object in JS. The 'stp' object
 * is provides a link to the currently active stand treatment programme.
*/
class STPObj: public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name)
    Q_PROPERTY (QJSValue options READ options)
public:
    void setSTP(FMStand *stand);
    explicit STPObj(QObject *parent = 0): QObject(parent) { mSTP = 0;}
    QJSValue options() { if (mOptions) return *mOptions;  else return QJSValue(); }
    QString name();
private:
    FMSTP *mSTP;
    QJSValue *mOptions; ///< options of the current STP


};
/**
 * @brief The ActivityObj class encapsulates the 'activity' object in JS. The 'activity'
 * can be used to fine-tune the management activities (e.g., set the enable/disable flags).
 */
class ActivityObj : public QObject
{
    Q_OBJECT
    Q_PROPERTY (bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(bool active READ active WRITE setActive)
    Q_PROPERTY(bool finalHarvest READ finalHarvest WRITE setFinalHarvest)
    Q_PROPERTY(bool scheduled READ scheduled WRITE setScheduled)
    Q_PROPERTY(QString name READ name)
public:
    explicit ActivityObj(QObject *parent = 0): QObject(parent) { mActivityIndex=-1; mStand=0; mActivity=0; }
    // used to construct a link to a given activty (with an index that could be not the currently active index!)
    ActivityObj(FMStand *stand, Activity *act, int index ): QObject(0) { mActivityIndex=index; mStand=stand; mActivity=act; }
    /// default-case: set a forest stand as the context.
    void setStand(FMStand *stand) { mStand = stand; mActivity=0; mActivityIndex=-1;}
    /// set an activity context (without a stand) to access base properties of activities
    void setActivity(Activity *act) { mStand = 0; mActivity=act; mActivityIndex=-1;}
    /// set an activity that is not the current activity of the stand
    void setActivityIndex(const int index) { mActivityIndex = index; }

    // properties

    QString name() const;
    bool enabled() const;
    void setEnabled(bool do_enable);

    bool active() const { return flags().active(); }
    void setActive(bool activate) { flags().setActive(activate);}

    bool finalHarvest() const { return flags().isFinalHarvest(); }
    void setFinalHarvest(bool isfinal) { flags().setFinalHarvest(isfinal);}

    bool scheduled() const { return flags().isScheduled(); }
    void setScheduled(bool issched) { flags().setIsScheduled(issched);}
public slots:
private:
    ActivityFlags &flags() const; // get (depending on the linked objects) the right flags
    static ActivityFlags mEmptyFlags;
    int mActivityIndex; // link to base activity
    Activity *mActivity; // pointer
    FMStand *mStand; // and to the forest stand....

};

/**
 * @brief The SchedulerObj class is accessible via 'scheduler' in Javascript.
 */
class SchedulerObj : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(double harvestIntensity READ harvestIntensity WRITE setHarvestIntensity)
    Q_PROPERTY(double useSustainableHarvest READ useSustainableHarvest WRITE setUseSustainableHarvest)
    Q_PROPERTY(double maxHarvestLevel READ maxHarvestLevel WRITE setMaxHarvestLevel)
    Q_PROPERTY(double minScheduleHarvest READ minScheduleHarvest WRITE setMinScheduleHarvest)
    Q_PROPERTY(double maxScheduleHarvest READ maxScheduleHarvest WRITE setMaxScheduleHarvest)
public slots:
    void dump() const; ///< write log to console
public:
    explicit SchedulerObj(QObject *parent = 0): QObject(parent) {mStand=0; }
    void setStand(FMStand *stand) { mStand = stand;}

    bool enabled();
    void setEnabled(bool is_enabled);
    double harvestIntensity();
    void setHarvestIntensity(double new_intensity);
    double useSustainableHarvest();
    void setUseSustainableHarvest(double new_level);
    double maxHarvestLevel();
    void setMaxHarvestLevel(double new_harvest_level);
    double minScheduleHarvest();
    double maxScheduleHarvest();
    void setMinScheduleHarvest(double new_level);
    void setMaxScheduleHarvest(double new_level);

private:
    FMStand *mStand; // link to the forest stand
};


} // namespace

#endif // FOMESCRIPT_H
