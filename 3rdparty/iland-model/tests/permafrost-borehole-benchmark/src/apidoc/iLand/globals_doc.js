/**
 * iLand Javascript API

Overview
========

iLand - the invidiual based landscape and disturbance model - is capable of simulating the development of forests on landscapes
of thousands of hectares. The model is built in C++ (using the [Qt-framework](http://qt.io) and utilizes the the built-in
V8 Javascript engine. The model exposes a number of (C++)-objects to the Javascript context, that allow accessing various iLand functions from Javascript.

See [iLand scripting](https://iland-model.org/iLand+scripting) for an overview.


 *
 * @module iLand
*/



/**
The Globals object exposes a series of auxiliary functions to the scripting context. An instance of "Globals" is always available in the global context, so no explicit creation is necessary.
The provided function cover inter alia:

- handling of directories and simple text files
- access to XML settings
- access and control over Outputs
- miscellaneous functions (e.g., for making screeshots)

 @class Globals
 */

/**
The current year of the simulation.
@property year
@type {integer}
@readOnly
*/

/**
The number of resource units in the current simulation.
@property resourceUnitCount
@type integer
@default 1
@readOnly */


/**
print the contents of the message to the log. In iLand GUI the message is also printed to the screen even if a logfile is specified.
In addition to the iLand `print()` function also the built-in Javascript object `console` can be used (e.g. `console.log()`; see https://developer.mozilla.org/de/docs/Web/API/Console).

Note: The global function `print()` is a short-cut to `Globals.print()`.

@method print
@param {string} message message to print as a string
*/

/**
shows a message box to the user (only available with iLand GUI) and halts execution until the user clicks OK.

Note: The global function `alert()` is a short-cut to `Globals.alert()`.

@method alert
@param {string} message message to show as a string
*/

/**
Includes / runs a Javascript file. The file is loaded and evaluated in the current Javascript context. `include` is useful for loading additional
code (e.g., library style scripts), or several management programs in ABE, or several agents in BITE.

Note: The global function `include()` is a short-cut to `Globals.include()`.

@Example
    // File 1
    include('scripts/file2.js');
    print(a); // prints "defined in file2"

    // file2.js:
    var a = "defined in file2";
    // some other stuff



@method include
@param {string} name of the Javascript file to include (path relative to the project root)
*/

/**
Completely reloads the ABE (agent based management engine) sub module. This includes loading of the (static) stand description file,
and the javascript source code of ABE.
@method reloadABE
*/

/**
Read a value from the [project file](https://iland-model.org/project+file). The `key` is the full path to the
requested node using a '.'-notation.

See also: {{#crossLink "Globals/set:method"}}{{/crossLink}}

@Example
    // global 'onInit' function is called during startup
    var width = Globals.setting('model.world.width'); // the horizontal extent of the project area
    var filename = Globals.setting('user.targetdir') + "my_file.txt"; // a user-defined key in the project file

@method setting
@param {string} key fully qualified key within the project file
@return The value of the setting, or _undefined_ if the setting is not present
*/


/**
Set a setting in the [project file](https://iland-model.org/project+file) with the key `key` to a value of `value`.
The `key` is the full path to the requested node using a '.'-notation.

See also: {{#crossLink "Globals/setting:method"}}{{/crossLink}}

@method set
@param {string} key fully qualified key within the project file
@param  value new value of the setting `key`.
@return {boolean} _true_ on success.
*/


/**
Get directory with of the given category `category`. See [filenames and paths](https://iland-model.org/filenames+and+paths) for available categories.
Using this defaultDirectory() avoids absolute file paths in scripts.

See also: {{#crossLink "Globals/currentDir:property"}}{{/crossLink}}

@method defaultDirectory
@param category {string} folder category
@return {string} The requested file path (without a trailing slash)
  */


/**
Use `path()` to construct a path relative to the project directory. Useful for avoiding absolute file paths in Javascript.

Example (Project-file: "c:\iland\test\test.xml"): Globals.path('temp/filexy.txt') -> "c:\iland\test\temp.filexy.txt"

See also: `defaultDirectory`, `currentDirectory`

@method path
@param filename {string} filename that should be extended
@return {string} fully qualified file path
*/


/**
gets or sets the current directory of iLand. Relative paths are resolved relative to the `currentDir`.

Example (Project-file: "c:\iland\test\test.xml"): Globals.path('temp/filexy.txt') -> "c:\iland\test\temp.filexy.txt"

See also: {{#crossLink "Globals/path:method"}}{{/crossLink}}
@property currentDir
@type {string}
@Example
    Globals.currentDir="c:/temp";
    Globals.loadTextFile("i_am_in_temp.txt"); // accesses c:/temp
*/

/**
extent of the world (without buffer) in meters (x-direction).

See also: {{#crossLink "Globals/worldY:property"}}{{/crossLink}}
@property worldX
@type {integer}
*/

/**
extent of the world (without buffer) in meters (y-direction).

See also: {{#crossLink "Globals/worldX:property"}}{{/crossLink}}
@property worldY
@type {integer}
*/
/**
The milliseconds since the start of the day.

@property msec
@type {integer}
@Example
    // simple timer functions:
    var _elapsed=-1;
    function start_timer() { _elapsed = Globals.msec; }
    function elapsed(thingi)
    {
        var elapsed=Globals.msec - _elapsed;
        _elapsed = Globals.msec;
        console.log("Time: " + thingi + ": " + elapsed + "ms");
    }

    // use timers:
    start_timer();
    // do some lengthy operation x
    elapsed("x"); // prints time elapsed since start_timer()
    // some other lenghty operation y
    elapsed("y"); // print duration of 'y' in ms
*/


/**
starts the output `table_name`. The table name for each output can be found on the [output](https://iland-model.org/output) wiki page.
Starting [debug outputs](https://iland-model.org/debug+outputs) is also possible - the `table_name` has to have the format _debug_XXX_, with _XXX_ one of the following:

+ treeNPP
+ treePartition
+ treeGrowth
+ waterCycle
+ dailyResponse
+ establishment
+ carbonCycle
+ performance

See also: {{#crossLink "Globals/stopOutput:method"}}{{/crossLink}}

@method startOutput
@param table_name {string} Output that should be started
@return {boolean} true on succes, an error message is printed in case of failure.
@Example
    Globals.startOutput("debug_performance");
*/
/**
stops the output `table_name`. The table name for each output can be found on the [output](https://iland-model.org/output) wiki page.
Starting (debug outputs) is also possible - see {{#crossLink "Globals/startOutput:method"}}{{/crossLink}}.

See also: {{#crossLink "Globals/startOutput:method"}}{{/crossLink}}

@method stopOutput
@param table_name {string} Output that should be stopped
@return {boolean} true on succes, an error message is printed in case of failure.
*/

/**
can be used to limit the amount of debug output that is generated on resource unit level. If the
function is called with a list of resource unit indices, then only RUs within the list produce debug output.
Output is deactivated for all other resource units (a call with an empty list effectively disables the output).

Effective for debug outputs on resource unit level and partiucularly useful for
outputs with daily resolution (e.g., daiyle water cycle, daily species response).

See also: {{#crossLink "Globals/saveDebugOutputs:method"}}{{/crossLink}}

@method debugOutputFilter
@param RUs {array} array of resource unit indices
@Example
    // limit output to a sample of the resource units
    Globals.debugOutputFilter([106,235,414,543]);
*/

/**
forces writing of debug outputs from the internal buffer of iLand to disk.


See also: {{#crossLink "Globals/startOutput:method"}}{{/crossLink}}, https://iland-model.org/Debug+Outputs

@method saveDebugOutputs
@param do_clear {boolean} if `true`, debug output buffer is cleared

*/


/**
creates a snapshot from the current state of the model. `file_name` is the path to the target database, which is created if the database file does not exist
(paths relative to the _home_ directory). The [wiki](https://iland-model.org/initialize+trees) provides details about snapshots.

See also: {{#crossLink "Globals/loadModelSnapshot:method"}}{{/crossLink}}

@method saveModelSnapshot
@param file_name {string} filename of the output database.
@return {boolean} true on succes.
*/

/**
loads a snapshot database (created with a previous call to {{#crossLink "Globals/saveModelSnapshot:method"}}{{/crossLink}}).
The model must be already created (i.e. resource units, ...);
exisiting trees are removed and replaced by the trees from the database.

See also: {{#crossLink "Globals/saveModelSnapshot:method"}}{{/crossLink}}

@method loadModelSnapshot
@param file_name {string} filename of the input database.
@return {boolean} true on succes.
*/

/**
  `viewOptions` allow some control over the visualization of the landscape in the iLand GUI. The `viewOptions` is an object with the following elements:

  + `minValue` and `maxValue`: the value range of the visualization; if not present, the value range of the ruler is automatically scaled
  + `type`: defines the type of visualization, and is one of the following:
       +  `lif`: the 'Light Influence Field' (2m)
       + `dom`: the dominant height (10m)
       + `regeneratation`: saplings/regeneration (2m)
       + `seed`: seed availability (20m)
       + `trees`: individual trees
       + `ru`: resource units (1ha)
  + `clip`: boolean ; if _true_, grids are clipped to the height grid (i.e. white for out-of-project-areas) (not supported for all visualizations)
  + `transparent`: boolean; if true, trees are drawn semi-transparent
  + `species`
       + boolean value in single tree mode: if true, draw trees using species colors (true)
       + a species short name (e.g. '_piab_'): select this species, e.g. for seed availability per species, or species shares
  + `grid`: draw one of the dynamic grids layers from active sub modules (check the possible names either in iLand or in the wiki)
  + `expression`: provides a expression that can be evaluated for trees or for resource unit (depending on the `type`)
  + `filter`: expression that is used to filter trees (useful for showing only a subset of trees)

  This option is not available in the iLand console version.

  @property viewOptions
  @type {object}
  @Example
        Globals.viewOptions = { type: 'trees', species: true };
        var vo = Globals.viewOptions;
        vo.filter = 'dbh<10';
        Globals.viewOptions = vo;
        Globals.repaint();
  */


/**
Load the content of a text file into a string.

@method loadTextFile
@param file_name {string} filename to load
@return {string} the content of the file or an empty text if file does not exist/is empty.

*/
/**
Save the content of a string `content` to a file.

@method saveTextFile
@param file_name {string} filename to save to
@param content {string} content that should be written
@return {boolean} true on success.
*/
/**
Check if the file `file_name` already exists.

@method fileExists
@param file_name {string} file to check
@return {boolean} true if the file already exists
*/
/**
Execute a system command (e.g., for copying files). Commands are operating system dependent; For windows,
`cmd.exe /C` executes a command without a separate command window (see example below). Output of the executed
command is redirected to the iLand log (stdout, stderr).

@method systemCmd
@param command {string} command to execute
@Example
    // helper function for windows: fix slashes and add 'cmd.exe /C'
    function winnify(s) {
         // replace forward with backward slashes
         s = s.replace(/\//g, '\\');
         s = "cmd.exe /C " + s;
         return s;
    }

    // onYearEnd: is called automtically from iLand at the end of a year
    function onYearEnd()
    {
        v = Globals.setting('user.v'); // 'version' is a user defined variable in the project file
        // create a folder for the simulation using 'v' at the end of the first simulation year
        if (Globals.year==1)
            Globals.systemCmd(winnify('mkdir ' + Globals.path('output/v' + v)));
    }
*/




/**
Add single trees on a specific resource unit with the 0-based index `resourceIndex`.
The tree list is in the string `content` and follows the
single-tree syntax described in [the wiki](https://iland-model.org/initialize+trees).


@method addSingleTrees
@param resourceIndex {integer} 0-based resource unit index
@param content {string} line (or lines) of an init-file to initialize
@return {integer} the number of added trees.
*/
/**
Add trees distribution on a specific resource unit with the 0-based index `resourceIndex`.
The tree list is in the string `content` and follows the distribution-tree syntax described in [the wiki](https://iland-model.org/initialize+trees).

@method addTrees
@param resourceIndex {integer} 0-based resource unit index
@param content {string} line (or lines) of an init-file to initialize
@return {integer} the number of added trees.
*/
/**
Add trees distribution on a specific stand described by the `standID`.
The stand is defined in the global stand grid. The tree list is in the string `content` and follows the distribution-tree
syntax described in [the wiki](https://iland-model.org/initialize+trees).

@method addTreesOnMap
@param standID {integer} ID of the stand in the stand grid
@param content {string} line (or lines) of an init-file to initialize
@return {integer} the number of added trees.
*/


/**
make a screenshot from the central viewing widget (as if pressing Ctrl+P in the iLand viewer) and stores the image to the provided file `file_name`.
The image type depends on the extension provided with `file_name`. Default path is the home directory.

@method screenshot
@param {string} file_name file name that
@return {boolean} true on success.
@Example
    // make a screenshot every 5 years
    function screenshot() {
       if (Globals.year % 5 == 0)
          Globals.screenshot( Globals.defaultDirectory('temp') + 'image_' + Globals.year + '.png' );
    }
*/

/**
force a repaint of the GUI main visualization area.

@method repaint
*/

/**
Creates a [ESRI style](http://en.wikipedia.org/wiki/Esri_grid) grid file from an iLand grid.
Specify the target file name with `file_name` and the type of the source grid with the string `grid_type`.

The avaialable `grid_type`s are:
* lif: the basic LIF grid of iLand (2m resolution)
* height: the height grid (10m resolution) of iLand (top tree heights)

See also: {{#crossLink "Globals/grid:method"}}{{/crossLink}}

@method gridToFile
@param {string} grid_type select the type of grid to export
@param {string} file_name target file path (relative to the home directory)
@return {boolean} true on success.
@Example
    Globals.gridToFile('height', 'temp/heightgrid.txt'); // store in project_folder/temp
*/

/**
Creates a [ESRI style](http://en.wikipedia.org/wiki/Esri_grid) grid file from the seed map for a given species.
The functions causes the creation of a raster file for 'species' the next time seed dispersal is calculated. `species` is the species-id (e.g., 'piab', 'fasy'),
and `file_name` the destination location of the grid file [ESRI ASCII raster](http://en.wikipedia.org/wiki/Esri_grid).
The saved seed map contains the seed distribution on 20m resolution.
Seed maps are only saved once, i.e. if a time series of seed maps is needed, `seedMapToFile()` need to be called periodically.

@method seedMapToFile
@param {string} species species code to export (e.g., 'piab')
@param {string} file_name target file path (relative to the home directory)
@return {boolean} true on success.
@Example
    // save maps for scots pine and beech
    Globals.seedMapToFile('pisy', 'temp/map_pisy.asc');
    Globals.seedMapToFile('fasy', 'temp/map_fasy.asc');
    // now run the model, at least for one year
    // files are created during model execution
*/

/**
extract a grid of type `type` from iLand. The extracted grid is a floating point grid (double precision) and
a copy of the current state in iLand (memory is freed automatically during Javascript garbage collection).

The grid has a cell size of 10m or 100m (depending on the type) and covers the full extent of the model.

The available grid types with 10m resolution are:
+ `height`: dominant tree height (m)
+ `count`: number of living trees (>4m height) on each pixel
+ `standgrid` the internal standgrid of iLand (see https://iland-model.org/landscape+setup#Setting_up_the_stand_grid)
+ `valid`: pixels inside the project area get a value of 1, pixels non within the project area 0 (see [wiki page](https://iland-model.org/landscape+setup))
+ `forestoutside`: 1 if a pixel is out of project area and is considered to be forested (see [wiki page](https://iland-model.org/landscape+setup))

The available grid types with 100m resolution are:
+ `smallsaplingcover`: the fraction of the area which is covered by small saplings (<=1.3m) OR grass cover (0..1). (Note: RUs with non-stockable area have always a value <1).
+ `saplingcover`: the fraction of the area which is covered by saplings (with a height >1.3m) (0..1). (Note: RUs with non-stockable area have always a value <1).
+ `swc`: mean annual water content (mm) over the full year
+ `swc_gs`: mean annual water content (mm) during the growing season (fixed month April - September)
+ `swc_pot`: field water capacity (mm) of the resource unit (potential water content)


See also: {{#crossLink "Globals/gridToFile:method"}}{{/crossLink}}

@method grid
@param {string} type select the type of grid to return
@return {Grid} a Javascript object encapsulating the {{#crossLink "Grid"}}{{/crossLink}}

*/


/**
Return a grid with the basal area of the given `species` (resource unit resolution, i.e. 100m).

See also: {{#crossLink "Globals/grid:method"}}{{/crossLink}}

@method speciesShareGrid
@param {string} species species code (e.g., 'piab') of the species
@return {Grid} a Javascript {{#crossLink "Grid"}}{{/crossLink}}
*/

/**
Return a grid (resolution of resource units) with the result of an `expression`
([Expression](https://iland-model.org/Expression)) evaluated in the context of the resource unit
(see the wiki for a list of [available variables](https://iland-model.org/resource+unit+variables)).


@method resourceUnitGrid
@param {string} expression Expression to evaluate for each resource unit
@return {Grid} a Javascript {{#crossLink "Grid"}}{{/crossLink}}
*/

/**
Pause model execution for `milliseconds` ms. This can be useful to slow down animations.


See also: {{#crossLink "Globals/repaint:method"}}{{/crossLink}}

@method wait
@param {integer} milliseconds time to wait in milliseconds
*/

/**
This is a helper function that allows to add shortcut links to the 'Scripting' panel in the iLand Viewer user interface.
`shortcuts` is a object with name/value pairs, where the _value_ is the string displayed in iLand, and _name_ the
Javascript function call (as a string).


@method setUIshortcuts
@param {object} shortcuts Javascript object that defines named Javascript calls
@Example
    Globals.setUIshortcuts({ 'kyrill()': 'run the kyrill storm',
                     'emma_paula()': 'run the emma/paula storms' }) ;

*/
Globals = {
}
