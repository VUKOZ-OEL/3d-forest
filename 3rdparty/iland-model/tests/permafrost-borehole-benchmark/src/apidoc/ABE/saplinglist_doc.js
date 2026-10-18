// Javascript API documentation for the FMSaplingList class
/**
The SaplingList class represents a list of saplings (=cohort of trees <4m height) that can be manipulated (e.g., removed) with functions of the class.

## Overview
### initializing the list
not implemented (but available in Bite)
### manipulating the content of the list
The list of sapling cohorts may be manipulated using functions such as `filter()`. There are functions helping to get aggregate values
(`sum()`).
### manipulationg saplings
The class provides function to kill cohorts (`kill()`) or to modify cohorts (e.g. `browse()`).

### Variables
A list of available variables for saplings:  https://iland-model.org/sapling+variables

### general notes
The sapling list heavily used the expression engine of iLand https://iland-model.org/Expression. Expressions are provided as strings to the
respective Javascript functions (e.g.,`filter`) and evaluated by iLand. Note that tree species names are treated as a special case (https://iland-model.org/Expression#Constants).

## Example
    var saplings = biteAgent.cell(0,0).saplings; // currently, only available via Bite
    // load of saplings in Bite: bitecell.reloadSaplings();

    saplings.filter('height > 2'); // keep only larger cohorts
    trees.sum('foliagemass*nrep'); // calculate total leaf mass of all trees in a cohort
    saplings.browse(); // affect saplings (browsing: reduced height growth)

@class SaplingList
***/

/**
the number of cohorts that are currently loaded in the list.
@property count
@type Integer
@readonly
*/

/**
Kill (i.e., cut down and do not remove from the forest) the saplings in the list, filtered with `filter`. If `filter` is omitted, all saplings are killed.


@method kill
@param {String} filter A valid filter Expression.
@return {Integer} the number of trees that have been removed.
@Example
    saplings.kill('age<5'); // kill young saplings
**/

/**
Affect all saplings in the list by browsing; this reduces the height growth in the current year to 0.

@method browse
@param {Boolean} do_browse if `true`, the cohorts are marked as browsed (`false` unmarks cohorts)
@return {Integer} the number of trees that have been removed.
@Example
    saplings.browse(); // browse saplings
**/

/**
Apply a filter on the list. Only saplings for which the filter condition `filter` evaluates to true, remain in the list.

@method filter
@param {String} filter A valid filter Expression.
@return {Integer} the number of trees that remain in the list.
@Example
    saplings.filter('species=fasy'); // remove all trees that are not Beech
    trees.filter('incsum(foliagemass*nrep)<1'); // max foliage mass after filtering 1kg

**/

/**
Calculates the sum of a given `expr` that can contain any of the available sapling variables. The optional `filter` is another expression
that filters the trees that are processed. Returns the sum of `expr`.

Note: counting saplings that fulfill the `filter` expression can be expressed as: `sum('1', '<filter-expr>')`.

@method sum
@param {string} expr a valid expression that should be processed
@param {string} filter only saplings that pass the filter are processed
@return {double} the sum over the value of `expr`of the population
@Example
    console.log('number of repr. trees: ' + saplings.sum('nrep') ); // just a count, same as saplings.length
    console.log('cohorts > 1m: ' + trees.sum('1', 'height>1') ); // note that '1' is a string (and parsed by the expression engine)
**/


