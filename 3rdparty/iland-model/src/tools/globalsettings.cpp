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

/** @class GlobalSettings
  This class contains various global structures/definitions. This class is a Singleton and accessed via the static instance() function.
  @par various (textual) meta data (SettingMetaData)

  @par global database connections
  There are two defined global database connections dbin() and dbout() with the names "in" and "out".
  They are setup with setupDatabaseConnection(). Currently, only SQLite DBs are supported.
  Use dbin() and dbout() to faciliate those database connections:
  @code
  ...
  QSqlQuery query(GlobalSettings::instance()->dbin());
  query.exec(...);
  ...
  @endcode

  @par Helpers with file Paths
  the actual project file is parsed for path defined in the <path> section.
  Use the path() function to expand a @p fileName to a iLand-Path. To check if a file exists, you could
  use fileExists().
  Available paths:
  - home: the project's home directory. All other directories can be defined relative to this dir.
  - lip: path for the storage of LIP (aka binary Stamp files) (default: home/lip)
  - database: base path for SQLite database files (default: home/database)
  - temp: path for storage of temporary files (default: home/temp)
  - log: storage for log-files (default: home/log)
  - exe: the path to the executable file.
  @code
  // home is "e:/iland/test", temp is "c:\temp" and log is omitted in project-file:
  QString p;
  p = Globals->path("somestuff.txt", "temp"); // > c:\temp\somestuff.txt
  p = Globals->path("e:\averyspecial\place.txt", "temp"); // -> e:\averyspecial\place.txt
                                                          //    (abs. path is not changed)
  p = Globals->path("log123.txt", "log"); // -> e:/iland/test/log/log123.txt (default for log)
  @endcode

  @par Fine-Grained debugging outputs
  The enumeration DebugOutputs defines a list of realms (uses binary notation: 1,2,4,8,...!).
  Use setDebugOutput() to enable/disable such an output. Use isDebugEnabled() to test inside the
  code if the generation of debug output for a specific type is enabled. Internally, this is a single
  bitwise operation which is very fast.
  Call debugLists() to retrieve a list of lists of data that fit specific criteria.
  @code
    // use something like that somewhere in a tree-growth-related routine:
    DBGMODE(
       if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dTreeGrowth) {
            DebugList &out = GlobalSettings::instance()->debugList(mId, GlobalSettings::dTreeGrowth); // get a ref to the list
            out << hd_growth << factor_diameter << delta_d_estimate << d_increment;   // fill with data
       }
    ); // only in debugmode
  @endcode

*/
#include "globalsettings.h"
#include <QtCore>
#include <QtXml>
#include <QtSql>
#include <QJSEngine>
#include <algorithm>
#include "global.h"
#include "helper.h"
#include "xmlhelper.h"
#include "stdint.h"

#include "settingmetadata.h"
#include "standstatistics.h"
#include "scriptglobal.h"


#include "outputmanager.h"

// debug macro helpers
void dbg_helper(const char *where, const char *what,const char* file,int line)
{
    qDebug() << "Warning in " << where << ":"<< what << ". (file: " << file << "line:" << line;
}
void dbg_helper_ext(const char *where, const char *what,const char* file,int line, const QString &s)
{
    qDebug() << "Warning in " << where << ":"<< what << ". (file: " << file << "line:" << line << "more:" << s;
}

static int _loglevel=0;
 // true, if detailed debug information is logged
bool logLevelDebug()
{
    return _loglevel<1;
}

// true, if only important aggreate info is logged
bool logLevelInfo()
{
    return _loglevel<2;
}

// true if only severe warnings/errors are logged.
bool logLevelWarning()
{
    return _loglevel<3;
}
void setLogLevel(int loglevel)
{
    _loglevel=loglevel;
    switch (loglevel) {
    case 0: qDebug() << "Loglevel set to Debug."; break;
    case 1: qDebug() << "Loglevel set to Info."; break;
    case 2: qDebug() << "Loglevel set to Warning."; break;
    case 3: qDebug() << "Loglevel set to Error/Quiet."; break;
    default: qDebug() << "invalid log level" << loglevel; break;
    }
}

GlobalSettings *GlobalSettings::mInstance = nullptr;

GlobalSettings::GlobalSettings()
{
    mDebugOutputs = 0;
    mModel = nullptr;
    mModelController = nullptr;
    mSystemStatistics = new SystemStatistics;
    // create output manager
    mOutputManager = new OutputManager();
    mScriptEngine = nullptr;

}


GlobalSettings::~GlobalSettings()
{
    delete mSystemStatistics;
    mInstance = nullptr;
    delete mOutputManager;
    // clear all databases
    clearDatabaseConnections();
    if (mScriptEngine)
        delete mScriptEngine;
}

QString GlobalSettings::executeJavascript(const QString &command)
{
    QString result = ScriptGlobal::executeScript(command);
    if (!ScriptGlobal::lastErrorMessage().isEmpty())
        Helper::msg("Javascript-Error: \n" + ScriptGlobal::lastErrorMessage());
    return result;
}

QString GlobalSettings::executeJSFunction(const QString function_name)
{
    QString result = ScriptGlobal::executeJSFunction(function_name);
    if (!ScriptGlobal::lastErrorMessage().isEmpty())
        Helper::msg("Javascript-Error: \n" + ScriptGlobal::lastErrorMessage());
    return result;
}

void GlobalSettings::resetScriptEngine()
{
    if (mScriptEngine)
        delete mScriptEngine;

    mScriptEngine = new QJSEngine();
    // add "console" extension (enabling the Console API)
    mScriptEngine->installExtensions(QJSEngine::ConsoleExtension);
    // globals object: instatiate here, but ownership goes to script engine
    ScriptGlobal *global = new ScriptGlobal();
    QJSValue glb = mScriptEngine->newQObject(global);
    mScriptEngine->globalObject().setProperty("Globals", glb);
}

// debugging
void GlobalSettings::setDebugOutput(const GlobalSettings::DebugOutputs dbg, const bool enable)
{
    if (enable)
        mDebugOutputs |= int(dbg);
    else
        mDebugOutputs &= static_cast<unsigned int>(dbg) ^ 0xffffffff;
}

// storing the names of debug outputs
//    enum DebugOutputs { dTreeNPP=1, dTreePartition=2, dTreeGrowth=4,
// dStandNPP=8, dWaterCycle=16, dDailyResponses=32, dEstablishment=64, dCarbonCycle=128 }; ///< defines available debug output types.
const QStringList debug_output_names=QStringList() << "treeNPP" << "treePartition" << "treeGrowth" << "waterCycle" << "dailyResponse" << "establishment" << "carbonCycle" << "performance";

///< returns the name attached to 'd' or an empty string if not found
QString GlobalSettings::debugOutputName(const DebugOutputs d)
{
    // this is a little hacky...(and never really tried!)
    for (int i=0;i<debug_output_names.count();++i) {
        if (d & (2<<i))
            return debug_output_names[i];
    }
    return QString();
}

///< returns the DebugOutputs bit or 0 if not found
GlobalSettings::DebugOutputs GlobalSettings::debugOutputId(const QString debug_name)
{
    int index = debug_output_names.indexOf(debug_name);
    if (index==-1) return GlobalSettings::DebugOutputs(0);
    return GlobalSettings::DebugOutputs(2 << index); // 1,2,4,8, ...
}



void GlobalSettings::clearDebugLists()
{
    mDebugLists.clear();
}

static QMutex debugListMutex;
DebugList &GlobalSettings::debugList(const int ID, const DebugOutputs dbg)
{
    QMutexLocker m(&debugListMutex); // serialize creation of debug outputs
    DebugList dbglist;
    dbglist << ID << dbg << currentYear();
    int id = ID;
    // use negative values for debug-outputs on RU - level
    // Note: at some point we will also have to handle RUS-level...
    if (dbg == dEstablishment || dbg == dCarbonCycle || dbg == dSaplingGrowth)
        id = -id;
    QMultiHash<int, DebugList>::iterator newitem = mDebugLists.insert(id, dbglist);
    return *newitem;
}
bool debuglist_sorter (const DebugList *i,const DebugList *j)
{
    return ((*i)[0].toInt() < (*j)[0].toInt());
}
const QList<const DebugList*> GlobalSettings::debugLists(const int ID, const DebugOutputs dbg)
{
    QList<const DebugList*> result_list;
    if (ID==-1) {
        foreach(const DebugList &list, mDebugLists)
            if (list.count()>2)  // contains data
                if (int(dbg)==-1 || (list[1]).toInt() & int(dbg) ) // type fits or is -1 for all
                    result_list << &list;
    } else {
        // search a specific id
        QMultiHash<int, DebugList>::const_iterator res = mDebugLists.find(ID);
        while (res != mDebugLists.end() && res.key() == ID)  {
            const DebugList &list = res.value();
            if (list.count()>2)  // contains data
                if (int(dbg)==-1 || (list[1]).toInt() & int(dbg) ) // type fits or is -1 for all
                    result_list << &list;
            ++res;
        }
    }
    // sort result list
    //std::sort(result_list.begin(), result_list.end(), debuglist_sorter); // changed because of compiler warnings
    std::sort(result_list.begin(), result_list.end(), debuglist_sorter);
    return result_list;
}

QStringList GlobalSettings::debugListCaptions(const DebugOutputs dbg)
{
    QStringList treeCaps = QStringList() << "Id" << "Species" << "Dbh" << "Height" << "x" << "y" << "ru_index" << "LRI"
                                         << "mStemMass" << "mCoarseRootMass" << "mFoliageMass" << "mLeafArea";

    if ( int(dbg)==0)
        return treeCaps;

    switch(dbg) {
    case dTreeNPP:
        return QStringList() << "id" << "type" << "year" << treeCaps
                             << "LRI_modified" << "light_response" << "effective_area" << "raw_gpp" << "gpp" << "npp" << "aging_factor";

    case dTreeGrowth:
        return QStringList() << "id" << "type" << "year" << treeCaps
                             << "net_stem_npp" << "stem_mass_before" << "hd_growth_ratio" << "factor_diameter"
                             << "dbh_inc_estimate_cm" << "dbh_inc_final_cm";

    case dTreePartition:
        return QStringList() << "id" << "type" << "year" << treeCaps << "mFineroot" << "mBranch"
                             << "npp_kg" << "apct_foliage" << "apct_wood" << "apct_root"
                             << "delta_foliage" << "delta_woody" << "delta_root" << "biomass_loss"
                             << "mNPPReserve" << "netStemInc" << "stress_index";

    case dStandGPP:
        return QStringList() << "id" << "type" << "year" << "species" << "RU_index" << "rid"
                             << "lai" << "gpp_kg_m2" << "gpp_kg" << "avg_aging" << "f_env_yr";

    case dWaterCycle:
        return QStringList() << "id" << "type" << "year" << "date" << "ruindex" << "rid" << "temp" << "vpd" << "prec" << "rad" << "combined_response"
                             << "after_intercept" << "after_snow" << "et_canopy" << "evapo_intercepted"
                             << "content" << "psi_kpa" << "excess_mm" << "snow_height" << "lai_effective"
                             // Permafrost details with corrected names
                             << "pf_top" << "pf_bottom" << "pf_freezeback" << "pf_delta_water_mm" << "pf_delta_soil_m" << "pf_k_unfrozen"
                             << "pf_soil_frozen_m" << "pf_water_frozen_mm" << "pf_unfrozen_fc_mm"
                             << "moss_f_light" << "moss_f_deciduous";

    case dDailyResponses:
        return QStringList() << "id" << "type" << "year" << "day_id" << "ru_index" << "ru_id" << "species"
                             << "temp" << "vpd" << "rad"
                             << "resp_soil" << "resp_vpd" << "resp_temp" << "resp_meteo"
                             << "resp_pheno" << "resp_co2" << "apar_mj_m2"
                             << "resp_daylength" << "utilizable_rad";
    case dEstablishment:
        return QStringList() << "id" << "type" << "year" << "species" << "RU_index" << "rid"
                             << "avgSeedDensity" << "TACAminTemp" << "TACAchill" << "TACAfrostFree" << "TACAgdd"
                             << "frostDaysAfterBudburst" << "waterLimitation" << "GDD" << "pAbiotic"
                             << "fEnvYr" << "newSaplings" ;

    case dSaplingGrowth:
        return QStringList() << "id" << "type" << "year" << "species" << "RU_index" << "rid"
                             << "Living_cohorts" << "averageHeight" << "averageAge" << "avgDeltaHPot" << "avgDeltaHRealized"
                             << "added" << "addedVegetative" << "died" << "recruited" << "refRatio"
                             << "carbonLiving" << "carbonGain";

    case dCarbonCycle:

        return QStringList() << "id" << "type" << "year" << "RU_index" << "rid"
                             << "SnagState_c" << "TotalC_in" << "TotalC_toAtm" << "SWDtoDWD_c" << "SWDtoDWD_n" << "toLabile_c" << "toLabile_n" << "toRefr_c" << "toRefr_n"
                             << "swd1_c" << "swd1_n" << "swd1_count" << "swd1_tsd" << "toSwd1_c" << "toSwd1_n" << "dbh1" << "height1" << "volume1"
                             << "swd2_c" << "swd2_n" << "swd2_count" << "swd2_tsd" << "toSwd2_c" << "toSwd2_n" << "dbh2" << "height2" << "volume2"
                             << "swd3_c" << "swd3_n" << "swd3_count" << "swd3_tsd" << "toSwd3_c" << "toSwd3_n" << "dbh3" << "height3" << "volume3"
                             << "otherWood1_c" << "otherWood1_n" << "otherWood2_c" << "otherWood2_n" << "otherWood3_c" << "otherWood3_n" << "otherWood4_c" << "otherWood4_n" << "otherWood5_c" << "otherWood5_n"
                             << "iLabC" << "iLabN" << "iKyl" << "iRefC" << "iRefN" << "iKyr" << "re" << "kyl" << "kyr" << "ylC" << "ylN" << "yrC" << "yrN" << "somC" << "somN"
                             << "NAvailable" << "NAVLab" << "NAVRef" << "NAVSom";
    case dPerformance:
        // Note: The 'id' column for this output is the year.
        return QStringList() << "id" << "type" << "year" << "treeCount" << "saplingCount" << "newSaplings" << "management"
                             << "applyPattern" << "readPattern" << "treeGrowth" << "seedDistribution" <<  "establishment"<< "saplingGrowth" << "carbonCycle"
                             << "writeOutput" << "totalYear";

    }
    return QStringList() << "invalid debug output!";
}


QStringList GlobalSettings::debugDataTable(GlobalSettings::DebugOutputs type,
                                           const QString separator,
                                           const QString fileName,
                                           const bool do_append)
{

    GlobalSettings *g = GlobalSettings::instance();
    QList<const DebugList*> ddl = g->debugLists(-1, type); // get all debug data

    QStringList result;
    if (ddl.count()==0)
        return result;

    QFile out_file(fileName);
    QTextStream ts;
    if (!fileName.isEmpty()) {
        if (do_append) {
            if (out_file.open(QFile::Append)) {
                ts.setDevice(&out_file);
            }
        } else {
            if (out_file.open(QFile::WriteOnly)) {
                ts.setDevice(&out_file);
                ts << g->debugListCaptions(type).join(separator) << Qt::endl;
            } else {
                qDebug() << "Cannot open debug output file" << fileName;
            }
        }

    }

    for (int i=ddl.count()-1; i>=0; --i) {
        QString line;
        int c=0;
        foreach(const QVariant &value, *ddl.at(i)) {
            if (c++)
                line+=separator;
            line += value.toString();
        }
        // save data to the file, or to the
        if (out_file.isOpen())
            ts << line << Qt::endl;
        else
            result << line;
    }
    if (!result.isEmpty())
        result.push_front( g->debugListCaptions(type).join(separator) );

    return result;
}

QList<QPair<QString, QVariant> > GlobalSettings::debugValues(const int ID)
{

    QList<QPair<QString, QVariant> > result;
    QMultiHash<int, DebugList>::iterator res = mDebugLists.find(ID);
    while (res != mDebugLists.end() && res.key() == ID)  {
        DebugList &list = res.value();
        if (list.count()>2) { // contains data
           QStringList cap = debugListCaptions( DebugOutputs(list[1].toInt()) );
           result.append(QPair<QString, QVariant>("Debug data", "Debug data") );
           int first_index = 3;
           if (list[3]=="Id")  // skip default data fields (not needed for drill down)
               first_index=14;
           for (int i=first_index;i<list.count();++i)
               result.append(QPair<QString, QVariant>(cap[i], list[i]));
        }
        ++res;
    }
    return result;
}


QString childText(QDomElement &elem, const QString &name, const QString &def="") {
    QDomElement e = elem.firstChildElement(name);
    if (elem.isNull())
        return def;
    else
        return e.text();
}


void GlobalSettings::clearDatabaseConnections()
{
    QSqlDatabase::removeDatabase("in");
    QSqlDatabase::removeDatabase("out");
    QSqlDatabase::removeDatabase("climate");
}

bool GlobalSettings::setupDatabaseConnection(const QString& dbname, const QString &fileName, bool fileMustExist)
{

    //QSqlDatabase::database(dbname).close(); // close database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dbname); // addDatabase replaces a connection with the same name
    qDebug() << "setup database connection" << dbname << "to" << fileName;
    //db.setDatabaseName(":memory:");
    if (fileMustExist) {
        if (!QFile::exists(fileName)) {
            throw IException("Error setting up database connection: file " + fileName + " does not exist!");
        }
    } else {
        // make sure the path exists
        QString db_path = QFileInfo(fileName).absolutePath();
        QDir().mkpath(db_path);
    }
    db.setDatabaseName(fileName);
    if (fileMustExist)
        db.setConnectOptions("QSQLITE_OPEN_READONLY"); // connect as read only

    if (!db.open()) {
        throw IException(QString("Error in setting up the database connection <%2> connection to file %1.\n").arg(fileName, dbname));
    }
    if (!fileMustExist) {
        // for output databases:
        // some special commands (pragmas: see also: http://www.sqlite.org/pragma.html)
        // db.exec("pragma temp_store(2)"); // temp storage in memory
        // db.exec("pragma synchronous(1)"); // medium synchronization between memory and disk (faster than "full", more than "none")
        // db.exec("pragma journal_mode(OFF)"); // disable transactions
        QSqlQuery query;
        // https://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite
        query.exec("PRAGMA synchronous = OFF");
        query.exec("PRAGMA journal_mode = MEMORY");
    }
    return true;
}


///////// Path functions
void GlobalSettings::printDirectories() const
{
    qDebug() << "current File Paths:";
    QHash<QString, QString>::const_iterator i;
    for (i=mFilePath.constBegin(); i!=mFilePath.constEnd(); ++i)
        qDebug() << i.key() << ": " << i.value();
}

void GlobalSettings::setupDirectories(QDomElement pathNode, const QString &projectFilePath)
{
    mFilePath.clear();
    mFilePath.insert("exe", QCoreApplication::applicationDirPath());
    XmlHelper xml(pathNode);
    QString homePath = xml.value("home", projectFilePath);
    mFilePath.insert("home", homePath);
    // make other paths relativ to "home" if given as relative paths
    mFilePath.insert("lip", path(xml.value("lip", "lip"), "home"));
    mFilePath.insert("database", path(xml.value("database", "database"), "home"));
    mFilePath.insert("temp", path(xml.value("temp", "", false), "home"));
    mFilePath.insert("log", path(xml.value("log", "", false), "home"));
    mFilePath.insert("script", path(xml.value("script", "", false), "home"));
    mFilePath.insert("init", path(xml.value("init", "", false), "home"));
    mFilePath.insert("output", path(xml.value("output", "output"), "home"));
}

/** extend the file to a full absoulte path of the given type (temp, home, ...).
  If @p file is already an absolute path, nothing is done. @sa setupDirectories().
  */
QString GlobalSettings::path(const QString &fileName, const QString &type)
{
    if (!fileName.isEmpty()) {
        QFileInfo fileinfo(fileName);
        if (fileinfo.isAbsolute())
            return QDir::cleanPath(fileName);
    }

    QDir d;
    if (mFilePath.contains(type))
        d.setPath(mFilePath.value(type));
    else {
        qDebug() << "GlobalSettings::path() called with unknown type" << type;
        d = QDir::currentPath();
    }

    return QDir::cleanPath(d.filePath(fileName)); // let QDir build the correct path
}

/// returns true if file @p fileName exists.
bool GlobalSettings::fileExists(const QString &fileName, const QString &type)
{
    QString name = path(fileName, type);

    if (!QFile::exists(name)) {
        qDebug() << "Path" << fileName << "(expanded to:)"<< name << "does not exist!";
        return false;
    }
    return true;
}


void GlobalSettings::loadProjectFile(const QString &fileName)
{
    qDebug() << "Loading Project file" << fileName;
    if (!QFile::exists(fileName))
        throw IException(QString("The project file %1 does not exist!").arg(fileName));
    mXml.loadFromFile(fileName);
    setupDirectories(mXml.node("system.path"),QFileInfo(fileName).absolutePath());

}

