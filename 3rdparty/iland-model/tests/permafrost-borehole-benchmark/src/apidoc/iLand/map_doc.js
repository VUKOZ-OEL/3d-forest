/**
The `Map` object encapsulates a "GIS" grid. Grids can be read from ESRI ASCII raster files, and are
automatically mapped to a 10x10m grid (the resolution and extent of the height grid).
Internally, a "spatial index" is created allowing for fast access to trees that lie on specific pixels.
See also the wiki page [landscape setup](https://iland-model.org/landscape+setup).
The loaded map can be used, e.g., to specifically apply management on specific areas.


A newly created `Map` object (without a call to `load()`) points to the global stand grid
defined in the [project file](https://iland-model.org/project+file).

Use {{#crossLink "Map/load:method"}}{{/crossLink}} to read a raster file from disk.


Example
--------
    function loadMap()
    {
      var path = Globals.defaultDirectory("script"); // get project script folder; see also the "currentDir" property of "Globals". defaultDirectory() adds already a slash
      // var stand_map = new Map(); // Qt4
      var stand_map = Factory.newMap(); // Qt5
      stand_map.load(path + "test.txt");
      // now load all trees on pixels with value '2020' in the "test.txt" grid
      management.loadFromMap(stand_map, 2020);
      // ... now do something ....

      var map = new Map();
      // select all trees on stand with id 127 of the 'system' stand grid
      management.loadFromMap(map, 127);
    }

 @module iLand
 @class Map
 */

Map = {

    /**
    The filename for successfully loaded grid or 'invalid'.

    @property name
    @type string
    @readOnly
    */

    /**
    Load a grid (provided in ESRI textformat) from disk. See [landscape setup](https://iland-model.org/landscape+setup) for information about projections.

    @method load
    @param {string} file_name
    */
    /**
    Retrieves the area of the polygon `stand_id` in square meters (m2). Returns -1, if the map is not valid, and 0 if no pixels with the stand `stand_id` are on the map.

    @method area
    @param {integer} stand_id ID of the polygon for which to return the area.
    @return {double}
    The area (m2) of the polygon.
    */
    /**
    Visualization of the map in the iLand Viewers' main window (if present).
    Map values are colorized between `min_value` (blue) and `max_value` (red).


    @method paint
    @param {double} min_value The minimum value for the color ramp in the visualization
    @param {double} max_value The minimum value for the color ramp in the visualization
    */
    /**
    Clears the map (set all values to 0)

    @method clear
    */

    /**
    Clear only the project area (set all cell values to 0), but do not affect pixels
    that are "outside of project area" (i.e., have values of -1 and -2).
    (see [Landscape setup](https://iland-model.org/Landscape+setup))

    @method clearProjectArea
    */

    /**
    "Paint" a shape on the Map with an ID `stand_id`.
    The `paint_function` is a valid iLand [Expression](https://iland-model.org/Expression)
    (with the paramters: `x`and `y` as *metric* coordinates). All pixels for which `paint_function`
    evaluates to `true` are set to `stand_id`, all other pixels are not modified.


    @method createStand
    @param {integer} stand_id ID of the polygon to be created
    @param {string} paint_function the function defining the shape
    @param {boolean} wrap_around if `true`, the shape is wrapped around the edges of the simulated area (torus)
    @Example
        var map = undefined;
        // the function create 10 random circles
        // with a radius between 10 and 60m on a random location on the landscape,
        // and removes some of those trees
        function random_circles()
        {
            if (map == undefined) {
               map = Factory.newMap(); // create a new map
               map.clear();
            }
            for (var i=1;i<10;++i) {
                var x = Math.random() * 600;
                var y = Math.random() * 400;
                var r = 10 + Math.random() * 50;
                map.clear();
                map.createStand(i,'(x-'+x+')^2+(y-'+y+')^2<'+r+'^2',true);
                // load all trees that are present on the stand
                management.loadFromMap(map, i);
                print(management.count + " trees in the area...");
                // apply a special filter polygon
                management.filter('polygon(dbh, 10,0, 30,1)');
                print(management.count + " after filter: trees in the area...");
                management.killAll(); // kill the trees
            }
        }
    */




}
