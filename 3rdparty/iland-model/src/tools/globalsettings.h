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

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QtCore>
#include <QtSql>
#include <QtSql/QSqlDatabase>

#include "global.h"
#include "settingmetadata.h"
#include "xmlhelper.h"
// The favorite random number generator:
// use either the MersenneTwister or the WELLS algoritm:
// #include "randomwell.h"
// #include "../3rdparty/MersenneTwister.h"
// use faster method to concatenate strings (see qt - documentation on QString)
#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

typedef QList<QVariant> DebugList;

class Model;
class OutputManager;
class ModelController; // forward
class SystemStatistics;
class QJSEngine; // forward

/// General settings and globally available data
class GlobalSettings
{
public:
    // singleton-access
    static GlobalSettings *instance() { if (mInstance) return mInstance; mInstance = new GlobalSettings(); return mInstance; }
    ~GlobalSettings();
    // Access
    // model and clock
    Model *model() const { return mModel; }
    ModelController *controller() const { return mModelController; }

    void setModel(Model *model) {mModel = model; }
    void setModelController(ModelController *mc) {mModelController = mc; }

    /// the current simulation year (starting with 1)
    int currentYear() const { return mRunYear; }
    void setCurrentYear(const int year) { mRunYear = year; }

    /// access the global QScriptEngine used throughout the model
    /// for all Javascript related functionality.
    QString executeJavascript(const QString &command);
    /// execute a javasript function in the global context
    QString executeJSFunction(const QString function_name);
    QJSEngine *scriptEngine() const { return mScriptEngine; }
    void resetScriptEngine(); ///< re-creates the script engine (when the Model is re-created)

    // system statistics
    SystemStatistics *systemStatistics() { return mSystemStatistics; }

    // debugging fain grained debug outputs
    enum DebugOutputs { dTreeNPP=1, dTreePartition=2, dTreeGrowth=4,
                        dStandGPP=8, dWaterCycle=16, dDailyResponses=32,
                        dEstablishment=64, dSaplingGrowth=128, dCarbonCycle=256,
                        dPerformance=512}; ///< defines available debug output types.
    void setDebugOutput(const int debug) { mDebugOutputs = GlobalSettings::DebugOutputs(debug); }
    void setDebugOutput(const DebugOutputs dbg, const bool enable=true); ///< enable/disable a specific output type.
    bool isDebugEnabled(const DebugOutputs dbg) {return int(dbg) & mDebugOutputs;} ///< returns true, if a specific debug outut type is enabled.
    int currentDebugOutput() const { return mDebugOutputs; }
    QString debugOutputName(const DebugOutputs d); ///< returns the name attached to 'd' or an empty string if not found
    DebugOutputs debugOutputId(const QString debug_name); ///< returns the DebugOutputs bit or 0 if not found
    DebugList &debugList(const int ID, const DebugOutputs dbg); ///< returns a ref to a list ready to be filled with debug output of a type/id combination.
    const QList<const DebugList*> debugLists(const int ID, const DebugOutputs dbg); ///< return a list of debug outputs
    QStringList debugListCaptions(const DebugOutputs dbg); ///< returns stringlist of captions for a specific output type
    QList<QPair<QString, QVariant> > debugValues(const int ID); ///< all debug values for object with given ID
    /// clear all debug data
    void clearDebugLists();
    /// output for all available items (trees, ...) in table form or write to a file
    QStringList debugDataTable(GlobalSettings::DebugOutputs type, const QString separator, const QString fileName=QString(), const bool do_append=false);

    // database access functions
    QSqlDatabase dbin() { return QSqlDatabase::database("in"); }
    QSqlDatabase dbout() { return QSqlDatabase::database("out"); }
    QSqlDatabase dbclimate() { return QSqlDatabase::database("climate"); }

    // path and directory
    QString path(const QString &fileName, const QString &type="home");
    bool fileExists(const QString &fileName, const QString &type="home");

    // xml project file
    const XmlHelper &settings() const { return mXml; }

    // setup and maintenance

    // xml project settings
    void loadProjectFile(const QString &fileName);

    // Database connections
    bool setupDatabaseConnection(const QString& dbname, const QString &fileName, bool fileMustExist);
    void clearDatabaseConnections(); ///< shutdown and clear connections
    // output manager
    OutputManager *outputManager() { return mOutputManager; }

    // path
    void setupDirectories(QDomElement pathNode, const QString &projectFilePath);
    void printDirectories() const;


private:
    GlobalSettings(); // private ctor
    static GlobalSettings *mInstance;
    Model *mModel;
    ModelController *mModelController;
    OutputManager *mOutputManager;
    QJSEngine *mScriptEngine;
    int mRunYear;
    SystemStatistics *mSystemStatistics;

    // special debug outputs
    QMultiHash<int, DebugList> mDebugLists;
    int mDebugOutputs; // "bitmap" of enabled debugoutputs.

    QHash<QString, QString> mFilePath; ///< storage for file paths

    XmlHelper mXml; ///< xml-based hierarchical settings
};

// constants
// We assume:
// Light-Grid: 2x2m
// Height-Grid: 10x10m
// Resource-Unit: 100x100m
const int cPxSize = 2; // size of light grid (m)
const int cRUSize = 100; // size of resource unit (m)
const double cRUArea = 10000.; // area of a resource unit (m2)
const int cHeightSize = 10; // size of a height grid pixel (m)
const int cPxPerHeight = 5; // 10 / 2 LIF pixels per height pixel
const int cPxPerRU = 50; // 100/2
const int cHeightPerRU = 10; // 100/10 height pixels per resource unit
const int cPxPerHectare = 2500; // pixel/ha ( 10000 / (2*2) )
const double cHeightPixelArea = 100.; // 100m2 area of a height pixel
const float cSapHeight = 4.f; // height from which on trees are modeled as individual trees (instead of saplings)

// other constants
const double biomassCFraction = 0.5; // fraction of (dry) biomass which is carbon
const double cAutotrophicRespiration = 0.47;

/// shortcut to the GlobalSettings Singleton object.
#define Globals (GlobalSettings::instance())

// provide a hashing function for the QPoint type (needed from stand init functions, ABE, ...)
inline size_t qHash(const QPoint &key)
{
    return qHash(key.x()) ^ qHash(key.y());
}
#endif // GLOBALSETTINGS_H
