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

#ifndef SCRIPTGLOBAL_H
#define SCRIPTGLOBAL_H

#include <QObject>
#include <QVariant>
#include <QJSValue>
#include <QRectF>


// Scripting Interface for MapGrid
class MapGrid; // forward
class ScriptGrid; // forward
class MapGridWrapper: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int valid READ isValid)
    Q_PROPERTY(QString name READ name)
public:
    Q_INVOKABLE MapGridWrapper(QObject *parent=nullptr);
    Q_INVOKABLE MapGridWrapper(QString fileName): MapGridWrapper() { load(fileName); }
    ~MapGridWrapper();

    static void addToScriptEngine(QJSEngine &engine);
    MapGrid *map() const { return mMap; } ///< acccess for C++ classes
    bool isValid() const; ///< returns true if map is successfully loaded
    QString name() const;
    //

public slots:
    // query
    double area(int id); ///< retrieve the area (m2) of the polygon denoted by 'id'
    // actions
    void load(QString file_name);
    void saveAsImage(QString file);
    void paint(double min_value, double max_value);

    /// register this map for interactive viewing in the UI
    void registerUI(QString name=QString());

    /// retrieve bounding box of a stand
    QRectF boundingBox(int stand_id);

    // active modifications of the map
    void clear(); ///< clears the map (set all values to 0)
    void clearProjectArea(); ///< clear the project area (set to 0), but copy mask with pixels from "outside of project area" (i.e., -1, -2)
    /// paint a shape on the stand grid with id stand_id
    /// paint_function is a valid expression (paramters: x, y as *metric* coordinates)
    /// if wrap_around=true, then the shape is wrapped around the edges of the simulated area (torus)
    void createStand(int stand_id, QString paint_function, bool wrap_around);

    /// copy a bit of the source-map 'source' to this map. The
    /// source rectangle is given by coordinates (x1/y1) to (x2/y2).
    /// The rectangle will be blitted to the new coordinates destx/desty (moved from x1/y1).
    /// id_in: the id of the polygon to copy, id: the id of the pixels in the target
    /// return the size (ha) of the valid thing
    double copyPolygonFromRect(QJSValue source, int id_in, int id, double destx, double desty, double x1, double y1, double x2, double y2);

    void createMapIndex(); ///< call after creating stands with copyPolygonFromRect

    void copyFromGrid(ScriptGrid *grid);

private:
    MapGrid *mMap;
    bool mCreated;

};

/** ScriptGlobal contains a set of
  functions and properties that are accessible by JavaScript.
  */
class Model;
class ScriptResourceUnit; // forward
class ScriptGlobal : public QObject
{
    Q_OBJECT
    // read only properties
    Q_PROPERTY(int year READ year)
    Q_PROPERTY(int resourceUnitCount READ resourceUnitCount)
    Q_PROPERTY(QString currentDir WRITE setCurrentDir READ currentDir)
    Q_PROPERTY(double worldX READ worldX)
    Q_PROPERTY(double worldY READ worldY)
    Q_PROPERTY(bool qt5 READ qt5)
    Q_PROPERTY(int msec READ msec)
    Q_PROPERTY(QJSValue viewOptions READ viewOptions WRITE setViewOptions)
    Q_PROPERTY(QString lastErrorMessage READ lastErrorMessage);

public:
    ScriptGlobal(QObject *parent=nullptr);
    static void setupGlobalScripting();
    // properties accesible by scripts
    bool qt5() const {return true; } ///< is this the qt5-model? (changes in script object creation)
    int msec() const; ///< the milliseconds since the start of the day
    int year() const; ///< current year in the model
    int resourceUnitCount() const; ///< get number of resource uinit
    QString currentDir() const { return mCurrentDir; } ///< current execution directory (default is the Script execution directory)
    void setCurrentDir(QString newDir) { mCurrentDir = newDir; } ///< set current working dir
    double worldX(); ///< extent of the world (without buffer) in meters (x-direction)
    double worldY(); ///< extent of the world (without buffer) in meters (y-direction)

    // general functions
    static void loadScript(const QString &fileName);
    static QString executeScript(QString cmd);
    static QString executeJSFunction(QString function);
    static QObject *scriptOutput; ///< public "pipe" for script output (is redirected to GUI if available)
    static QString formattedErrorMessage(const QJSValue &error_value, const QString sourcecode=QString());
    static void throwError(const QString &errormessage);
    static QString lastErrorMessage() { return mLastErrorMessage; }

    // view options
    /* View options:
     * * type: {...}
     * * species: bool
     * * shade: bool
     *
    */
    QJSValue viewOptions(); ///< retrieve current viewing options (JS - object)
    void setViewOptions(QJSValue opts); ///< set current view options

    // static functions
    static QJSValue valueFromJs(const QJSValue &js_value, const QString &key, const QString default_value=QStringLiteral(""), const QString &errorMessage=QStringLiteral(""));
    static QString JStoString(QJSValue value);

public slots:
    // system stuff
    QVariant setting(QString key); ///< get a value from the global xml-settings (returns undefined if not present)
    void set(QString key, QString value); ///< set the value of a setting
    void print(QString message); ///< print the contents of the message to the log
    void alert(QString message); ///< shows a message box to the user (if in GUI mode)
    void include(QString filename); ///< "include" the given script file and evaluate. The path is relative to the "script" path
    void loadModule(QString moduleName, QString filename);
    /// return a random number between from and to (inclusive).
    /// Note: when using standard JS Math.random(), the random seed of iLand is not effective
    double random(double from=0., double to=1.);
    // file stuff
    QString defaultDirectory(QString dir); ///< get default directory of category 'dir'
    QString path(QString filename); ///< get a path relative to the project main folder
    QString loadTextFile(QString fileName); ///< load content from a text file in a String (@sa CSVFile)
    void saveTextFile(QString fileName, QString content); ///< save string (@p content) to a text file.
    bool fileExists(QString fileName); ///< return true if the given file exists.
    QString systemCmd(QString command); ///< execute system command (e.g., copy files)
    // add trees
    int addSingleTrees(const int resourceIndex, QString content); ///< add single trees
    int addTrees(const int resourceIndex, QString content); ///< add tree distribution
    int addTreesOnMap(const int standID, QString content); ///< add trees (distribution mode) for stand 'standID'
    // add saplings
    int addSaplingsOnMap(MapGridWrapper *map, const int mapID, QString species, int px_per_hectare, double height, int age);
    /// add sapling on a metric rectangle given with `width` and `height` at x/y.
    /// if a `standId` is provided (-1 or 0: no stand), x/y is relative to the lower left edge of the stand rectangle. If no
    /// stand is provided, x/y are absolute (relative to the project area).
    /// returns the number of successfully added sapling cells
    int addSaplings(int standId, double x, double y, double width, double height, QString species, double treeheight, int age);
    /// remove all saplings from a metric rectangle. See also `addSaplings()`.
    void removeSaplings(int standId, double x, double y, double width, double height);
    // enable/disable outputs
    bool startOutput(QString table_name); ///< starts output 'table_name'. return true if successful
    bool stopOutput(QString table_name); ///< stops output 'table_name'. return true if successful
    void useSpecialMapForOutputs(MapGridWrapper *m); ///< use a specific map / standgrid for outputs
    // debug outputs
    void debugOutputFilter(QList<int> ru_indices); ///< enable debug outputs for a list of resource units (output for other RUs are suppressed)
    bool saveDebugOutputs(bool do_clear); ///< save debug outputs to file; if do_clear=true then debug data is cleared from memory
    // miscellaneous stuff
    void setViewport(double x, double y, double scale_px_per_m); ///< set the viewport of the main project area view
    bool screenshot(QString file_name); ///< make a screenshot from the central viewing widget
    void repaint(); ///< force a repainting of the GUI visualization (if available)
    bool gridToFile(QString grid_type, QString file_name, double hlevel=0); ///< create a "ESRI-grid" text file 'grid_type' is one of a fixed list of names, 'file_name' the ouptut file location

    /// return Javascript grid for given type
    QJSValue grid(QString type);
    /// return a grid with the basal area of the given species (resource unit resolution)
    QJSValue speciesShareGrid(QString species);
    /// return a grid (level of resource units) with the result of an expression evaluated in the context of the resource unit.
    QJSValue resourceUnitGrid(QString expression);

    /// get a grid for a given variable and a month (1..12)
    QJSValue microclimateGrid(QString variable, int month=1);
    /// access to single resource unit (returns a reference)
    QJSValue resourceUnit(int index);


    // DOES NOT FULLY WORK
    bool seedMapToFile(QString species, QString file_name); ///< save the "seedmap" (i.e. a grid showing the seed distribution) as ESRI raster file
    void wait(int milliseconds); ///< wait for 'milliseconds' (or if ms=-1 until a key is pressed)
    // vegetation snapshots
    bool saveModelSnapshot(QString file_name);
    bool loadModelSnapshot(QString file_name);
    bool saveStandSnapshot(int stand_id, QString file_name);
    bool loadStandSnapshot(int stand_id, QString file_name);
    bool saveStandCarbon(int stand_id, QList<int> ru_ids, bool rid_mode=true);
    bool loadStandCarbon();
    // agent-based-model of forest management
    void reloadABE();

    // UI interface
    void setUIshortcuts(QJSValue shortcuts); ///< set a list of JS shortcuts in the UI

    void test_tree_mortality(double thresh, int years, double p_death);
private:
    static QString mLastErrorMessage;
    QString mCurrentDir;
    Model *mModel;
    QJSValue mRUValue;
    ScriptResourceUnit *mSRU;
};

/** The ScriptObjectFactory can instantiate objects of other C++ (QObject-based) types.
 *  This factory approach is used because the V8 (QJSEngine) does not work with
 *  the "new" way of creating objects.
*/
class ScriptObjectFactory: public QObject
{
    Q_OBJECT
public:
    ScriptObjectFactory(QObject *parent=nullptr);
public slots:
    QJSValue newCSVFile(QString filename); ///< create a new instance of CSVFile and return it
    QJSValue newClimateConverter(); ///< create new instance of ClimateConverter and return it
    QJSValue newMap(); ///< create new instance of Map and return it
    QJSValue newDBHDistribution(); ///< create new instance of DBHDistribution and return it
    QJSValue newGrid(); ///< creates a new grid object and return it
    QJSValue newSpatialAnalysis(); ///< creates a new SpatialAnalysis object and returns it
    int stats() {return mObjCreated;} ///< return the number of created objects
private:
    int mObjCreated;

};

#endif // SCRIPTGLOBAL_H
