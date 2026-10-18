
/**
  The global variable `fmengine` (of type `FMEngine`) variable lets you access the ABE core engine.

  Use `fmengine` to register agents and stand treatment programmes. `fmengine` also provides some additional functionalities, such as logging (`log`)
  or executing functions/activities in the ABE context.

  @class FMEngine
  */
var fmengine= {
/**
  `log` writes a log message. Each message is prefixed with a code for identifying the current stand and the current year of the simulation.
  The format of the prefix is: 'S_standid_Y_year_:'.

        fmengine.log('log message for stand ' + stand.id);
        // produces (in year 0 and for stand 7)
        abe: "S7Y0:" log message for stand 7

  @method log
  @param {string} message The message to be printed.
  */
    log: function(x){},

/**
  Calling `abort` stops the execution of ABE and prints the error message `message` to the console (and shows the error). Note that `abort` does not
  interrupt execution of JavaScript code immediately. However, the ABE core engine cancels all further activities, when the `abort` method has been called.


            if (some_error_condition) {
                fmengine.abort('This is really not expected!!');
                return; // required; otherwise do_some_stuff() would be executed.
            }
            do_some_stuff();


  @method abort
  @param {string} message The error message.
  */
    abort: function(x),

/**
    `runActivity` executes an {{#crossLink "Activity"}}{{/crossLink}} for stand given by `standId`. This bypasses the normal scheduling (useful for debugging/testing).
    This function calls the main execution function of the activity.

  @method runActivity
  @param {int} standId the (integer) id of the stand in which context the activity should be executed.
  @param {string} activity the name of the activity that should be executed.
  @return {boolean} returns false if the stand or activity were not found.

  */
    runActivity: function(id, activity),

/**
  `runActivityEvaluate` executes an {{#crossLink "Activity"}}{{/crossLink}} for stand given by `standId`. This bypasses the normal scheduling (useful for debugging/testing).
  'runActivityEvaluate' invokes the evaluation code of scheduled activites (e.g., ActThinning, ActScheduled).

  @method runActivity
  @param {int} standId the (integer) id of the stand in which context the activity should be executed.
  @param {string} activity the name of the activity that should be executed.
  @return {boolean} returns false if the stand or activity were not found.

  */
    runActivityEvaluate: function(id, activity),

/**
  adds a management program (STP) whose definition is provided by the Javascript object `program`. The `name` of the program is used internally.

  See also: {{#crossLink "Activity"}}{{/crossLink}}

  @method addManagement
  @param {object} program The javascript object that defines the STP.
  @param {string} name The name that ABE should be use for this STP.
  @return {boolean} true on success.
  */
     addManatgement: function(obj, name),

/**
  add an agent definition (Javascript) and gives the agent the name `name`.

  @method addAgent
  @param {object} program The javascript object that defines the {{#crossLink "Agent"}}{{/crossLink}}.
  @param {string} name The name that ABE should be use for this {{#crossLink "Agent"}}{{/crossLink}}.
  @return {boolean} true on success.

  @example
        // define a simple agent
        var fire_agent = {
            // scheduler options:
            scheduler: { enabled: false },
            // stp is a list of STPs available for the agent
            stp: {  'fire': 'fire', 'timber':'timber', 'default': 'fire'},
        };

        fmengine.addAgent(fire_agent, 'fire'); // add the agent

  */
    addAgent: function(obj, name),

/**
  add an agent by specifying the name of an agent type and gives the agent the name `name`.

    See also: {{#crossLink "addAgentType:method"}}{{/crossLink}}

  @method addAgent
  @param {string} agent_type_name The name of the agent type {{#crossLink "Agent"}}{{/crossLink}}.
  @param {string} name The name that ABE should be use for this {{#crossLink "addAgentType:method"}}{{/crossLink}}.
  @return {boolean} true on success.
  */
    addAgent: function(agent_type_name, name),

/**
  add an agent type (and not a specific agent!) by specifying the name of an agent type. The agent type
  will get the name `name`. Using an agent type (instead of single agents) allow the creation of multiple
  agents programmatically (e.g. small landowners). Properties of the generated agents can be refined
  using the `newAgent()` handler function of the agent.

  @example


  See also: {{#crossLink "addAgentType:method"}}{{/crossLink}}

  @method addAgentType
  @param {object} program The javascript object that defines the {{#crossLink "Agent"}}{{/crossLink}}.
  @param {string} name The name that ABE should be use for this agent type
  @return {boolean} true on success.


  @example

        // define a agent type template
        var fire_template = {
            // scheduler options:
            scheduler: { enabled: true,
                         harvestIntensity: 1},
            // stp is a list of STPs available for the agent
            stp: {  'fire': 'fire', 'timber':'timber', 'default': 'fire'},
            newAgent: function() {
                // this is called whenever a new agent is generated
                var new_ag = {
                scheduler: this.scheduler };
                new_ag.harvestIntensity = 1 + Math.random()*0.5; // sample between 1 and 1.5
                return new_ag;
            }
        };
  */
    addAgentType: function(agent_type_name, name),
/**
  checks if a given `stand_id` is valid (i.e., part of the currently simulated area).

  @method isValidStand
  @param {int} stand_id The id of the stand to check.
  @return {boolean} true, if 'stand_id' is a valid stand in the current setup.
  */
    isValidStand: function(id),

/**
  returns a list of valid stand-ids within the model.
  See also: {{#crossLink "standId:property"}}{{/crossLink}}

        for (var s of fmengine.standIds) {
           fmengine.standId = s; // set s to the current stand
           console.log('Stand ' + s + ': ' + stand.flag('wui')); // print value of the attribute wui for all stands
        }

  @method standIds
  @return {array} Array of valid stand-ids.
  */
    standIds: function(),

/**
  Runs a planting activity (without the context of stand treatment programmes). This is especially useful for
  setting up initial stand conditions. The `planting` defines the activity according to the syntax of the planting activity.


        // global 'onInit' function is called during startup
        function onInit() {
          // run a planting activity for the stand 235 (30cm spruce trees on 90% of the pixels)
          fmengine.runPlanting( 235, { species: "piab", fraction: 0.9, height: 0.3 });
        }

  @method runPlanting
  @param {int} standId (integer) of the stand for which a planting activity should be executed.
  @param {object} planting A javascript object with the definition of the planting activity (see ABE documentation).
  */
    runPlanting: function(standid, obj),

/**
  ABE generates much more detailed log messages, if `verbose` is true. This is generally useful for debugging and
  should be turned off for productive use. Note, that `verbose` mode can switched on for specfic code sections:

        ....
        fmengine.verbose = true;
        ... // do some complicated stuff
        fmengine.verbose = false; // switch off verbose mode

  See also: {{#crossLink "Stand/trace:property"}}{{/crossLink}}

  @property verbose
  @type boolean
  @default false
  */
    verbose: false,
/**
  The `enabled` property can be used to enable/disable ABE. When `false` then all management activties are paused.

        // switch management off at a certain simulation year
        function run() {
           // run is called by ABE every year
           if (Globals.year==10)
              fmengine.enabled = false;
        }
        // run() is called from ABE (only if ABE is running). In order to enable ABE use e.g.:
        function onNewYear() {
            // is called by iLand at the beginning of every simulation year
            if (Globals.year==30)
                fmengine.enabled = true;
        }



  @property enabled
  @type boolean
  @default true
  */
    enabled: true,

/**
  `standId` is the numeric Id of the stand that is set as the current execution context, i.e. the `stand`, `site`, etc. variables
  that are available in the current context refer to the stand with this Id. The property is set automatically by ABE during the
  execution of forest management. It can, however, be changed by Javascript for debugging/testing purposes. Note that changing
  the `standId` might lead to unexpected behavior.


  See also: {{#crossLink "Stand/id:property"}}{{/crossLink}}

  @property standId
  @type int
  @default -1
  */
    standId: -1,
/**
  This function adds quicklinks that trigger Javascript functions to the user interface of iLand. Each name-value pair
  defines one function call (name) and its description (value).


        // call this anywhere (e.g. in the body of the main javascript)
        Globals.setUIShortcuts( { 'Globals.reloadABE()': 'full reload of ABE', 'test()': 'my dearest test function' } );


  @method setUIShortcuts
  @param {object} A object containt name/value pairs that are used in the iLand UI to create quicklinks for calling javascript functions.
  */
    setUIShortcuts: function(obj)

}






/**
* Access to properties of the current activity.
* The `activity` variable is available in the execution context of forest management and provides access to properties and functions
* of the current activity (linked to a `stand`) that is currently processed. In addition, other activities of the (currently active) programme
* can be accessed with {{#crossLink "Stand/activity:method"}}{{/crossLink}}.
*
* Note that the variable `activity` is provided through the C++ framework and does not need to be created separately.
*
* See also: Variables `stand` ({{#crossLink "Stand"}}{{/crossLink}})

*
@class Activity
*/
var activity: {

/**
  The `active` property indicates whether an activity __can__ still be executed during this rotation. After an activity is executed,
  the `active` flag is set to false, and the next active (and `enabled`) activity is selected. At the end of a rotation, the `active`
  flag of all activities of the programme are set back to true. By changing the value of `active`, activities can be either removed
  from the current rotation, or re-enabled.

        //re-enable the 'todo' activity.
        stand.activity("todo").active = true;


  See also: {{#crossLink "Activity/enabled:property"}}{{/crossLink}}

  @property active
  @type boolean
  @default false
  */
    active: false,

/**
  An activity can only be executed, if the `enabled` property is true. Activities can be switched on/ off using this flag.

  Note that an activity needs also to be `{{#crossLink "Activity/active:property"}}{{/crossLink}}` in order to get executed.

        // switch off a repeating activity, when a height threshold is surpassed.
        if (stand.hmean>12)
            stand.activity('repeater').enabled = false;


  See also: {{#crossLink "Activity/active:property"}}{{/crossLink}}

  @property enabled
  @type boolean
  @default false
  */
    enabled: false,

/**
  This flag indicates whether the {{#crossLink "Scheduler"}}{{/crossLink}} is used when this activity is run, i.e. if the
  exact date of execution is selected by the scheduling algorithm of ABE. The default value depends on the type of the activity,
  e.g., default is false for activities of type __general__, but true for __scheduled__ activities.

  @property scheduled
  @type boolean
  @default depends on activity type.
  */
    scheduled: false,

/**
  An activity with `finalHarvest`=true ends a rotation when (successfully) executed. This resets the `{{#crossLink "Stand/age:property"}}{{/crossLink}}`
  counter and sets all activites `{{#crossLink "Activity/active:property"}}{{/crossLink}}`.

  @property finalHarvest
  @type boolean
  @default depends on activity type.
  */
    finalHarvest: false,


/**
  The name of the activity as provided in the definition of the activity.

  See also: {{#crossLink "FMEngine/addManagement:method"}}fmengine.addManagement{{/crossLink}}

  @property name
  @type string
  @readonly
  */
    name: ''
}




/**
* Access to properties of the current stand treatment program.
* The `stp` variable is available in the execution context of forest management and provides access to properties STP
* activity (linked to a `stand`).
*
* Note that the variable `stp` is provided through the C++ framework and does not need to be created separately.
*
* See also: Variables `stand` ({{#crossLink "Stand"}}{{/crossLink}})

*
@class STP
*/
var stp: {
    /**
      The name of the stand treatment program as provided when called the respecitve 'fmengine' functions.

      See also: {{#crossLink "FMEngine/addManagement:method"}}fmengine.addManagement{{/crossLink}}

      @property name
      @type string
      @readonly
      */
        name: ''
    },
    /**
      * The 'options' property provides a link to the 'options' property of the STP *definition*, i.e., this is a mechanism
      * that can be used to pass purely javascript options/values to other javascript code (e.g., when an activity within the
      * stand context is executed). Note that the C++ part of ABE does not interfere with these options.

                // definition of the STP
                var stp: { U: [110, 130, 150], // define the rotation period
                    clearcut: a_clearcut, // some activities
                    options: { dbh: 5, someOtherParameter: 50}
                }

                // definition of activities… here the ‘dbh’ options is used
                // in the Javascript code to select trees to harvest.
                var a_clearcut: {
                   type: "scheduled", … ,
                   onEvaluate: function() {
                            trees.loadAll();
                            trees.harvest("dbh>" + stp.options.dbh );}
                         }
                };



      @property options
      @type object
        */
        options: {}


}
