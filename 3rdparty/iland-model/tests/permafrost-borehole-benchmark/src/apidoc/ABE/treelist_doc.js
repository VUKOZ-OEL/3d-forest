// documentation for the FMTreeList API


/**
The TreeList class (**`trees`** variable) represents a list of trees that can be manipulated (e.g., harvested) with functions of the class.
When javascript code is executed in the context of an Activity, a variable 'trees' is available in the global context. This instance
of a TreeList is linked automatically to the forest stand that is currently managed.

## Overview
### initializing the list
The main function for loading (i.e. making available for manipulation) trees of a forest stand is `loadAll()` or `load()`. `loadAll()` can be used to load either
all trees, and `load()` to load only a subset of trees (based on filter criteria).
### manipulating the content of the list
The trees in a list (loaded by `load()`) may be manipulated using functions such as `filter()` or `sort()`. There are functions helping to get aggregate values
(`mean()`, `sum()`), or to get the value of a given `percentile()`.
### manipulationg trees
Trees present in the tree list may be harvested or simply cut down. Main functions are `kill()`, `harvest()`. If `simulate` is true, harvested/killed trees
are only marked for removal. At a later point in time, all marked trees can be removed using `removeMarkedTrees()`.
### general notes
The tree list heavily used the expression engine of iLand https://iland-model.org/Expression. Expressions are provided as strings to the
respective Javascript functions (e.g.,`filter`) and evaluated by iLand. Note that tree species names are treated as a special case (https://iland-model.org/Expression#Constants).


## Examples
    // 'trees' variable is always available.
    // this loads *all* trees of the current forest stand (i.e. trees>4m)
    trees.loadAll();
    trees.simulate = true; // enable simulation mode (this is the default)
    // keep only spruce trees with a dbh>20cm in the list:
    trees.filter("species = piab and dbh>20"); // note: piab is a constant and not a string
    // now harvest all trees (since simulate=true, the trees are only marked for harvesting)
    trees.harvest();
    ...
    // later: remove all trees from the system that are marked for removal (harvest/kill)
    trees.removeMarkedTrees();

@class TreeList
***/


/**
The `load()` method is the main means to get all the trees of a forest stand into the list.
Per default, all trees (i.e. trees>4m height) that are located on the forest stand are loaded. If `filter` is provided, only a subset
of the trees are loaded. `filter` can either be a probability (between 0..1) for selecting individual trees, or an Expression using tree
variables.

    trees.load(0.2); // load (randomly selected) 20% of all trees
    trees.load("rnd(0,1)<0.2)"); // the same as above, but using an Expression
    // load a subset of trees (for which the given Expression evalulates to true):
    trees.load("species=piab and dbh>20");

@method load
@param {String} filter optional filter criterion (see above).
@return {Integer} the number of trees loaded.
**/


/**
When `simulate` is true, harvest operations are not actually executed, but affected trees are marked as either for harvest or killing. Calling `removeMarkedTrees()` then
executes the harvest operation.
Note: tree variables `markharvest`, `markcut`, `markcrop` are available for use in `filter()` or for visualization in iLand.
@property simulate
@type Boolean
@default true
*/

/**
the ID of the currently active stand (or -1).
@property stand
@type Integer
@readonly
*/

/**
the number of trees that are currently loaded in the list.
@property count
@type Integer
@readonly
*/

/**
Load all trees (living trees, >4m height) of the current `stand` into the internal list.

@method loadAll
@return {Integer} the number of loaded harvested.
**/


/**
Check all trees of the stand and either kill or harvest those trees that are marked for that operation.

See also: {{#crossLink "TreeList/simulate:property"}}{{/crossLink}}, {{#crossLink "TreeList/harvest:method"}}{{/crossLink}}

@method removeMarkedTrees
@return {Integer} the number of trees that have been removed.
**/

/**
Kill (i.e., cut down and do not remove from the forest) the trees in the list, filtered with `filter`.

See also: {{#crossLink "TreeList/simulate:property"}}{{/crossLink}}

@method kill
@param {String} filter A valid filter Expression.
@return {Integer} the number of trees that have been removed.
@Example
    trees.loadAll();
    trees.kill('dbh<10'); // kill all trees with a dbh<10cm
**/


/**
Apply a filter on the list. Only trees for which the filter condition `filter` is true, remain in the list.

See also: {{#crossLink "TreeList/simulate:property"}}{{/crossLink}}

@method filter
@param {String} filter A valid filter Expression.
@return {Integer} the number of trees that remain in the list.
@Example
    trees.loadAll();
    trees.filter('dbh>10'); // only trees with dbh>10 remain in the list
    trees.filter('incsum(volume)<100'); // keep trees with a total of 100m3 in the list


**/

/**
Remove the `fraction` of all trees [0..1] for which `filter` evalulates to `true`. Return number of removed trees.
When trees are harvested, the biomass is removed from the system (compare kill/cut).
When `harvest()` is called without parameters, then all trees that are currently in the list are removed (see also `load()`).


See also: {{#crossLink "TreeList/simulate:property"}}{{/crossLink}}, {{#crossLink "TreeList/kill:method"}}{{/crossLink}}

@method harvest
@param {string} filter A valid filter Expression.
@param {double} fraction The fraction [0..1] of trees that should be harvested. Default value is 1.
@return {Integer} the number of trees that have been removed.
@Example
    trees.loadAll();
    trees.harvest('species=piab', 0.2); // remove 20% (selected randomly) from all spruce
    trees.harvest('dbh>30'); // harvest all trees with dbh>30cm
**/

/**
Return the `perc` th percentile (0..100) value from the list of trees. The function requires that a {{#crossLink "TreeList/sort:method"}}{{/crossLink}} operation
has been executed before this function is called. The `sort()` function assigns to each tree in the list a value (e.g., the `dbh`). The `percentile` function
accesses those value. Without a call to `sort` the function returns 0 for all values. The percentile 100 returns the maximum value within the list (i.e., the value last
attached to the last tree in the list).

See also:{{#crossLink "TreeList/sort:method"}}{{/crossLink}}

@method percentile
@param {int} perc a number between 0 and 100 that should be returned.
@return {double} the value at the given percentile (see details)
@Example
    trees.loadAll();
    trees.sort('dbh'); // smallest trees in front of the list
    console.log("IQR: " + trees.percentile(75) - trees.percentile(25) );
    console.log("min: " + trees.percentile(0) + ", median: " + trees.percentile(50) + ", max: " + trees.percentile(100) );
**/

/**
`sort()` sorts the tree list according to the numeric value of the expression `sort_expr`. The sort order is ascending, i.e., after sorting the tree with the smallest
value is at the beginning of the list. Descending order can be achieved by inverting the sign of the expression. A sorted list is useful
for extracting {{#crossLink "TreeList/percentile:method"}}{{/crossLink}} values or for using the `incsum` function of expressions [iland-model.org/Expression].

See also: {{#crossLink "TreeList/percentile:method"}}{{/crossLink}}

@method sort
@param {string} sort_expr a valid expression used for sorting
@Example
    trees.loadAll();
    trees.sort('volume'); // small trees in front of the list
    trees.filter('incsum(volume) < 100'); // keep 100m3 of the trees with the lowest volume
    trees.loadAll();
    trees.sort('-x'); // x-coordinate, descending, trees start from the 'right'
    trees.filter('incsum(1)<=100'); // filter 100 trees on the right edge of the area
**/

/**
Randomly shuffles the list of trees. Trees in a newly refreshed list (e.g., `loadAll()`) have a non-random spatial pattern. When
randomness is wanted (e.g., to select randomly N trees), then `randomize` should be used.

Note: `randomize` overwrites the values of a {{#crossLink "TreeList/sort:method"}}{{/crossLink}} call.

See also: {{#crossLink "TreeList/loadAll:method"}}{{/crossLink}}

@method randomize
@Example
    trees.loadAll();
    trees.sort('dbh');
    trees.filter('dbh<' + trees.percentile(50) ); // only the population with dbh < median(dbh) remains
    // to randomly select from this population:
    trees.randomize();
    trees.filter('incsum(1) < 100'); // select 100 trees from the population
**/

/**
Calculates the mean value of a given `expr` that can contain any of the available tree variables. The optional `filter` is another expression
that filters the trees that are processed.
Return the mean value.

See also: {{#crossLink "TreeList/sum:method"}}{{/crossLink}}

@method mean
@param {string} expr a valid expression that should be processed
@param {string} filter only trees that pass the filter are processed
@return {double} the mean value of the population
@Example
    trees.loadAll();
    console.log('mean dbh: ' + trees.mean('dbh') );
    var tdbh = trees.mean('height', 'dbh>=30'); // mean height of all trees >30cm
    var ldbh = trees.mean('height', 'dbh<30'); // mean height of all belwo 30cm - note that the list is not altered
**/

/**
Calculates the sum of a given `expr` that can contain any of the available tree variables. The optional `filter` is another expression
that filters the trees that are processed. Returns the sum of `expr`.

Note: counting trees that fulfill the `filter` expression can be expressed as: `sum('1', '<filter-expr>')`.

See also:{{#crossLink "TreeList/mean:method"}}{{/crossLink}}

@method sum
@param {string} expr a valid expression that should be processed
@param {string} filter only trees that pass the filter are processed
@return {double} the sum over the value of `expr`of the population
@Example
    trees.loadAll();
    console.log('total basal area: ' + trees.sum('basalarea') );
    console.log('N trees >30cm: ' + trees.sum('1', 'dbh>30') ); // note that '1' is a string (and parsed by the expression engine)
**/

/**
`killSaplings()` provides an access to the cohorts of the sapling layer in iLand https://iland-model.org/sapling+growth+and+competition .
https://iland-model.org/sapling+variables provides a list of available variables.
The function removes all sapling cohorts for which `expr` returns `true`.

Note: The interface to saplings currrently much simpler compared to the interface for trees >4m.

@method killSaplings
@param {string} filter a filter expression to select the saplings that should be removed
@return {int} the number of sapling cohorts that have been removed.
@Example
    trees.killSaplings('species=piab and age<5'); // kill spruce saplings younger than 5yrs
**/


/**
`spatialFilter()` filters the tree that are currently in the list based on their spatial location and the content
of the `grid`. The function checks for every tree the corresponding pixel of `grid` and evaluates the `filter` function. If the
function returns true, the tree is kept (if the tree is outside of the `grid` the tree is removed).

Note that when not otherwise altered, the default name of a grid is simply 'x', so filtering for a certain value could be achieved
by using 'x=1' as a `filter`.

See also: {{#crossLink "Grid/create:method"}}{{/crossLink}}, {{#crossLink "Grid/setOrigin:method"}}{{/crossLink}}

@method spatialFilter
@param {Grid} grid a {{#crossLink "Grid"}}{{/crossLink}}Grid object
@param {string} filter A expression that is evaluated for every pixel of the `grid`.
@return {int}  the number of trees still remaining in the list, or -1 if an error occurs.
@Example
    // create a grid
    var g = Factory.newGrid();
    g.create(10,10,5); // 50x50m, default name is 'x'
    g.setOrigin(30,20); // set lower left corner to 30/20
    // in the context of ABE:
    trees.loadAll(); // load all trees
    // filter so, that only trees that are located on a pixel in g with the value 1 remain:
    trees.spatialFilter(g, 'x=1');
    // do something, e.g.,
    trees.harvest();
**/

/**
`tree(index)` returns the tree with index `index` (index is 0-based) from the current list. The returned Javascript object
is a reference to the represented tree in iLand. See {{#crossLink "Tree"}}{{/crossLink}} for more details.

See also: {{#crossLink "TreeList/treeObject:method"}}{{/crossLink}}

@method tree
@return {Tree}  a reference to the tree with index `index` (see above). If the index is out of range, the `valid` property of the returned object is `false`.
@Example
    trees.loadAll(); // fill the tree list
    // loop over all trees
    for (var i=0;i<trees.count;++i)
       console.log( trees.tree(i).dbh );

**/

/**
`treeObject(index)` returns the tree with index `index` (index is 0-based) from the current list. A new Javascript object
is created for the tree. See {{#crossLink "Tree"}}{{/crossLink}} for more details.

See also: {{#crossLink "TreeList/tree:method"}}{{/crossLink}}

@method treeObject
@return {Tree}  a object representing the tree with index `index` (see above), or an invalid Tree object (if index is out of range).
@Example
    trees.loadAll(); // fill the tree list
    var x = trees.treeObject(7); // 8th tree in the list
    // access the tree
    if (x.valid == true)
       console.log(x.dbh);

**/
