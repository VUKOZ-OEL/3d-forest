
/**
The `SpatialAnalysis` class encapsulates special spatial analysis operations in iLand. For example, the class
contains methods to conduct patch size analysis, or calculate the Rumple index.

An instance of the object is available in the global Javascript context with the name `SpatialAnalysis`.

Example
-------
    SpatialAnalysis.saveRumpleGrid('temp/rumple.asc');

@module iLand
@class SpatialAnalysis
 */


/**
Retrieve the rumple index for the full landscape (i.e., one value).

See also: {{#crossLink "SpatialAnalysis/saveRumpleGrid:method"}}{{/crossLink}}

@property rumpleIndex
@type double
*/

/**
Generate a map of the crown cover ( at 2m resolution), and save an average (100m) to `filename` (ESRI ASCII format).
For each tree, the tree crown (as defined by the ['reader' stamp](https://iland-model.org/Lightroom), which defines for 2m pixels the share
of pixels that are covered by the crown) is additively plotted on a 2m grid.
If that sum is larger then 0.5 (i.e. 50%), a pixel is consiedered as 'covered'.
The resulting grid provides for each 100m cell a fraction of 'covered' pixels. Note that 2m cells that are out of the project area
are not counted (i.e., the resulting crown cover refers only to the stockable area of the resource unit).

@method saveCrownCoverGrid
@param {string} filename target filename (relative to project folder)
*/

/**
Generate a map of the crown cover ( at 2m resolution), and save an average to `filename` (ESRI ASCII format).
For each tree, the tree crown (as defined by the ['reader' stamp](https://iland-model.org/Lightroom), which defines for 2m pixels the share
of pixels that are covered by the crown) is additively plotted on a 2m grid.
If that sum is larger then 0.5 (i.e. 50%), a pixel is consiedered as 'covered'.
The `grid` is used as a mask: the resulting grid uses the cell size of `grid` and also the spatial extent.
The resulting grid provides for each 100m cell a fraction of 'covered' pixels.


@method saveCrownCoverGrid
@param {string} filename target filename (relative to project folder)
@param {Grid} mask (a {{#crossLink "Grid"}}{{/crossLink}}
@Example
    // create a grid:
    var mask = Factory.newGrid();
    mask.create(100, 60, 10); // cell-size 10m, 100 x 60 cells
    mask.setOrigin(100,1020); // define the origin, the grid covers (100/1020)-(1100/1620)
    SpatialAnalysis.saveCrownCoverGrid('temp/test.asc', mask); // save the crown cover grid
*/

/**
Calculate the Rumple-Index for the landscape with a spatial resolution of 10m (i.e., the cells of the iLand height grid)
and extract a 100m grid (average over the 10m cells). Store the grid to `filename` (ESRI ASCII format).

The Rumple-Index is a spatial index relating surface area to ground area.
In forestry, it is a indicator of vertical heterogeneity. In iLand, the Rumple Index is
the variability of the maximum tree height on 10m level (i.e. the "Height"-Grid).
The RumpleIndex is calculated for each resource unit and also for the full project area.

See also: {{#crossLink "SpatialAnalysis/rumpleIndex:property"}}{{/crossLink}}

@method saveRumpleGrid
@param {string} filename target filename (relative to project folder)
*/

/**
Perform a patch analysis on the input `grid` and return a {{#crossLink "Grid"}}{{/crossLink}} with unique patch IDs
assigned to each patch (starting with 1, 2, 3, ...). A 'patch' is a number of adjacent pixels with a value > 0. In other words,
all connected non-zero areas are flagged with unique Ids in the output grid.
Internally, a simple flood-fill algorithm searching in the Moore-neighborhood (8 neighbors) is used and applied repeatedly.

If a patch has a size which is smaller than `min_size`, the cells of the grid are set to 0, and the patch is not recorded. A list of all
patches (and patchsizes) is available with the {{#crossLink "SpatialAnalysis/patchsizes:property"}}{{/crossLink}} property.


See also: {{#crossLink "SpatialAnalysis/patchsizes:property"}}{{/crossLink}}

@method patches
@param {Grid} grid grid to analyze
@param {integer} min_size ignore patches with an area below `min_size` pixels.
*/
/**
The `patchsizes` property provides a list of with the number of pixels of each extracted patch in a prior call to  See also: {{#crossLink "SpatialAnalysis/patches:method"}}{{/crossLink}}.
The first entry of the array is the number of pixels of the patch ID 1, etc.

See also: {{#crossLink "SpatialAnalysis/patches:method"}}{{/crossLink}}

@property patchsizes
@type array[integer]
@Example
    // let 'wind_grid' be a grid of wind damages, non-zero pixels had some kind of damage (the damage map has a 10m resolution)
    // now run the patch analysis and keep damaged areas >4 px
    wind_grid = SpatialAnalysis.patches(wind_grid, 5);
    // save the patches grid to the output directory and a user code given in the XML file
    wind_grid.save( Globals.defaultDirectory('output') + Globals.setting('user.code')+ '_stormDamage_v' + v + '.asc' );
    // save also a list of the extracted patch sizes
    Globals.saveTextFile(Globals.defaultDirectory('output') + Globals.setting('user.code')+ '_stormPatches_v' + v + '.txt', SpatialAnalysis.patchsizes + "\n");

*/

SpatialAnalysis = {
}
