/**
* Access to properties of the current stand.
* The `stand` variable is available in the execution context of forest management and provides access to properties and functions
* of the stand that is currently processed.
*
* Note that the variable `stand` is provided through the C++ framework and does not need to be created separately.
*
* Use the 'flag' and 'setFlag' methods to (persistently) modify / read user-specific properties for each stand. This is a means
* to pass stand-specific information between activities (or between different events within one activity).
*
*
@class Stand
*/
var stand = {
/**
  If `trace` is set to true, detailed log information is produced by ABE. This is useful for testing/ debugging.
  The trace-mode can be switched on/ off like this:

        // enable trace for stand 7
        fmengine.standId = 7; // set the current stand to the stand with Id 7
        stand.trace = true; // enable trace


  See also: {{#crossLink "FMEngine/verbose:property"}}{{/crossLink}}

  @property trace
  @type boolean
  @default false
  */

/**
  The id of the stand that is currently processed.

  See also: {{#crossLink "FMEngine/standId:property"}}{{/crossLink}} in `fmengine`.

  @property id
  @type integer
  @default -1
*/

/**
  The basal area / ha stocking on the stand (living trees, >4m).

  @property basalArea
  @type double
*/


/**
  The total standing timber volume / ha of the stand (living trees, >4m).

  @property volume
  @type double
*/

/**
  The mean height of the stand (meter). It is calculated as basal area weighted mean height of all trees on the stand (>4m).
  See also {{#crossLink "Stand/topHeight:property"}}{{/crossLink}}.

  @property height
  @type double
*/

/**
  The top height (in meters) is defined as the mean height of the 100 thickest trees per ha. For larger/ smaller stands, the number of trees is scaled accordingly.
  See also {{#crossLink "Stand/height:property"}}{{/crossLink}}.

  @property topHeight
  @type double
*/

/**
  The mean age of the stand (years). It is calculated as basal area weighted mean age of all trees on the stand (>4m).
  Note the difference to `absoluteAge`, which is the number of years since the rotation started.

  See also {{#crossLink "Stand/absoluteAge:property"}}{{/crossLink}}.

  @property age
  @type double
*/

/**
    The age of the stand given in years since the rotation started. At startup, the `absoluteAge` is estimated from
    the `age` of the stand (i.e. the mean age of the initialized trees). Later, the stand age counter is reset
    by management activities. Note that this property is writable.

    See also {{#crossLink "Stand/age:property"}}{{/crossLink}}.

  @property absoluteAge
  @type double
*/

/**
  The number of different tree species present on the stand (trees >4m). Use to iterate over the available species on the stand:

        // print the species id and the basal area for each available species.
        // note that the species are ordered by the basal area share.
        for (var i=0;i<stand.nspecies;++i)
            log(stand.speciesId(i) + ": " + stand.speciesBasalArea(i));

  @property nspecies
  @type int
*/

/**
  The total area of the stand in hectares.


  @property area
  @type double
*/
/**
  Retrieve the species id at position `index`.

  @method speciesId
  @param {integer} index The index of the species (valid between 0 and `nspecies`-1).
  @return {string} The unique id of the species.*/

/**
  Retrieve the basal area of the species at position `index`.

  @method speciesBasalArea
  @param {integer} index The index of the species (valid between 0 and `nspecies`-1).
  @return {double} The basal area (m2/ha) of the species.*/

/**
  Retrieve the basal area of the species with the species code 'speciescode'.
  Note that only trees with height > 4m are included.

  @method speciesBasalAreaOf
  @param {string} speciescode The code of the species (e.g., 'piab').
  @return {double} The basal area (m2/ha) of the species, or 0 if the species is not present.*/

 /**
  Retrieve the relative basal area of the species 'speciescode'.

  @method relSpeciesBasalAreaOf
  @param {string} speciescode The code of the species (e.g., 'piab').
  @return {double} The basal area (m2/ha) of the species, or 0 if the species is not present.*/

/**
*  Retrieve the basal area share (0..1) of the species at position `index`.
*
*      // get the share of the dominant species:
*      log( stand.relSpeciesBasalArea(0) * 100 + "%");
*
*  @method relSpeciesBasalArea
*  @param {integer} index The index of the species (valid between 0 and `nspecies`-1).
*  @return {double} The basal area share (0..1) of the species.*/

/**
  Force a reload of the stand data, i.e. fetch stand statistics (e.g. basal area, age)
  from the trees comprising the stand.

  Usually, this is done automatically by ABE, however, it might be useful in some rare circumstances.

  @method reload

*/

/**
  The ´sleep´ method suspends the activities on the stand for `years` years. Only after the specified has elapsed,
  ABE continues to examine the stand.

  @method sleep
  @param {integer} years The number of years that the stand should sleep.

*/

/**
  Use `activity` to retrieve an {{#crossLink "Activity"}}{{/crossLink}} object.

  Note: the global variable `activity` is a "short-cut" to access the currently active activity.

        stand.activity("my_thinning_2").enabled = false; // disable an activity
        var act = stand.activity("my_thinning_1"); // save a reference to the activity for later use

  See also: the global variable `{{#crossLink "Activity"}}activity{{/crossLink}}`

  @method activity
  @param {string} activity_name The name of the activity to be retrieved. Activity names are provided during activity definition (see {{#crossLink "FMEngine/addManagement:method"}}fmengine.addManagement{{/crossLink}})
  @return {Activity} the Activity, or `undefined` if not found.
*/

/**
  Retrieves the stand-specific property associated with the name 'name' for the stand of the current execution context.

        stand.setFlag('test', 3); // simple values
        stand.setFlag('my_goal', { s1: 10, s2: 20, s3: function(){return this.s1+this.s2;} } ); // complex objects (including functions are allowed)

        fmengine.log( stand.flag('my_goal').s3 + stand.flag('test') + stand.U  ); // -> 133 (if U=100 of the stand)

@method flag
@param {string} name The (user-defined) property name of the stored parameter.
@return {value} The associated value for the given 'name'. Returns 'undefined' if no value is assigned.*/

/**
  Sets 'value' as the stand-specific property associated with the name 'name' for the stand of the current execution context.


  @method setFlag
  @param {string} name The (user-defined) property name of the stored parameter.
  @param {value} value The value that should be stored for 'name'. 'value' can be any valid Javascript expression (including objects).
*/

/**
  The number of years since the execution of the last activity for the current stand. Value is -1 if no activity was executed previously.

  @property elapsed
  @type int
*/

/**
  The name of the last previously executed activity, or an empty string if no activity was executed before. The name can be used to access
  properties of the activity.


           if (stand.lastActivity == "thinning1")
                stand.activity( stand.lastActivity ).enabled = true; // re-enable last activity if it was 'thinning1'

  @property lastActivity
  @type string
*/

/**
  The rotation length of the current stand. The rotation length is defined by the stand treatment programme that is currently assigned to a given
  stand. The 'U' is frequently used for timing activites relative to the length of the period.

  @property U
  @type double
*/
}



