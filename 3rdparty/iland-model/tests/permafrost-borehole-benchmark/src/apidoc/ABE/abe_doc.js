// Javascript API documentation framework http://yui.github.io/yuidoc/
// create with "yuidoc ." in /apidoc directory (/build/apidocs must be available!) or with "yuidoc --server ." for interactive mode.
// see also the yuidoc.json for more options.
// http://127.0.0.1:3000/

// Installation of YUIDOC: http://yui.github.io/yuidoc/
// you need node.js -> download and install

// including a nice search box:
// https://github.com/jiannicello/yuidocsite
// run in 'apidocs' folder (the generated YUI docs are pushed to the 'docs' folder):
// yuidocsite --port 3000 --search_desc

// bootstrap theme: https://www.npmjs.com/package/yuidoc-bootstrap-theme
// npm install yuidoc-bootstrap-theme
//

/**
 * The agent based forest management engine.

Overview
========
The ABE forest management system is:
+ is a hybrid C++/Javascript system
+ follows a declarative paradigma - forest management strategies are described using Javascript-Objects (like JSON), but include also
imperative section and event handlers that allow a fine grained control

The concept is described in more details on the iLand wiki page: https://iland-model.org/ABE

building blocks
---------------

The main classes of ABE are:

+ **{{#crossLink "Agent"}}{{/crossLink}}** are conceptually forest managers that are responsible for a part or for the full simulated landscape.
+ **{{#crossLink "STP"}}{{/crossLink}}** a STP object encapsulates a stand treatment program, that includes one or many forest management activities.
+ **{{#crossLink "Activity"}}{{/crossLink}}** is a single management activity such as a thinning or a planting.


In addition, the API defines various helper objects:

+ **{{#crossLink "FMEngine"}}{{/crossLink}}** is the main class of ABE. It provides methods to define agents, stand treatment programs, etc and methods to "manually" execute forest mangement
+ **{{#crossLink "Stand"}}{{/crossLink}}** encapsulates a "stand" in the model. A landscape can have multiple stands (e.g., by providing a GIS layer).
+ **{{#crossLink "TreeList"}}{{/crossLink}}** a collection of individual trees that are e.g. fetched from a specific stand

 *
 * @module ABE
*/




/**
* Schedule object.
*
* @class Schedule
*/

var schedule= {
/**
  The 'dump' method prints the contents of the scheduler (i.e., the list of stands with the estimated harvests and execution dates to the console.
  @method dump
  */
    dump: function(){},
/**
  If 'verbose' is set to true, much more log messages are generated from the scheduling algorithms.
  @property verbose
  @type {bool}
  @default false
  */
    verbose: false,
    /**
      The 'harvestIntensity' is a multiplier for the "sustainable" harvest level; values > 1 cause the harvesting being
      above the level of sustainable yield (as it is estimated by the scheduler).
      @property harvestIntensity
      @type {bool}
      @default false
      */

    harvestIntensity: 0,
/**
'useSustainableHarvest' is a scaling factor (0..1) that allows gradually switching between bottom-up harvest planning (i.e., stands are always processed at their optimal dates),
and a top-down approach (i.e, the scheduling algorithm decides when a stand should be processed). A value of 1 means
 that the scheduler (assigned to the agent) is used exclusively, and 0 means a strict bottom up approach. Between 0 and 1 the harvest is scaled in between.

 still used????

  @property useSustainableHarvest
  @type {bool}
  @default false
  */
    useSustainableHarvest: false,
/**
  The 'maxHarvestLevel' property is a multiplier to define the maximum overshoot over the planned volume that the scheduler is willing to take (e.g. 1.2 -> 20% max. overshoot).
  @property maxHarvestLevel
  @type {double}
  @default 1
  */
    maxHarvestLevel: 1.1,

/**
  The 'maxAbsoluteHarvest' is a cutoff for the annual harvest. If a a value is given (default is no cap), then
  the scheduler stops when the cap is reached.

  Notes:
    * the value is only effective when the increment based scheduling is off ('enabled: false')
    * to calculate the running sum of harvest the scheduler uses the value provided as the return value
      of the onEvaluate() handler of the activities. This defaults to the volume (per ha) of planned harvests
      but can be used differently. For example, an area limit can be established by using
      stand area as return value in the handler.

  @type {double}
  @property maxAbsoluteHarvest
  @default 0
  */
    maxAbsoluteHarvest: 1.1,
}


/**
* Constraints object.
*
* @class Constraint
*/

/**
* Definition of an Agent.
* The main part of the agent defintion is a list of assignments of stand treatment programmes (STP) to mixture types.
* In addition, logic can be added for selecting mixture types from forest attributes.
*
*

        var x = { stp: { "fasy": ["beech_default_1", "beech_default_2", "beech_default3"],
                         "piab": "spruce_default" }
                  onSelect: function(){ return "fasy"; }
                 }
*
@class Agent
*/

/** Description of stp
@property stp
@type object
*/
/**
  onSelect allows to provide custom code to select the mixture type for a given stand.
  @example
       onSelect: function() { if (stand.share('piab') > 50)
                                return 'piab';
                            return 'default'; }
  @event onSelect
  @type String
  */



