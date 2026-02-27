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

/** @class Model
  Main object of the iLand model composited of various sub models / sub components.
  @ingroup core
  The class Model is the top level container of iLand. The Model holds a collection of ResourceUnits, links to SpeciesSet and Climate.
  ResourceUnit are grid cells with (currently) a size of 1 ha (100x100m). Many stand level processes (NPP produciton, WaterCycle) operate on this
  level.
  The Model also contain the landscape-wide 2m LIF-grid (https://iland-model.org/competition+for+light).

  */
#include "global.h"
#include "model.h"
#include "sqlhelper.h"
#include "version.h"

#include "xmlhelper.h"
#include "debugtimer.h"
#include "environment.h"
#include "timeevents.h"
#include "helper.h"
#include "resourceunit.h"
#include "climate.h"
#include "microclimate.h"
#include "watercycle.h"
#include "speciesset.h"
#include "standloader.h"
#include "tree.h"
#include "management.h"
#include "saplings.h"
#include "modelsettings.h"
#include "standstatistics.h"
#include "mapgrid.h"
#include "modelcontroller.h"
#include "modules.h"
#include "dem.h"
#include "grasscover.h"
#include "svdstate.h"

#include "outputmanager.h"

#include "forestmanagementengine.h"
#include "biteengine.h"

#include <QtCore>
#include <QtXml>

/** iterate over all trees of the model. return NULL if all trees processed.
  Usage:
  @code
  AllTreeIterator trees(model);
  while (Tree *tree = trees.next()) { // returns NULL when finished.
     tree->something(); // do something
  }
  @endcode  */
Tree *AllTreeIterator::next()
{

    if (!mTreeEnd) {
        // initialize to first ressource unit
        mRUIterator = mModel->ruList().constBegin();
        // fast forward to the first RU with trees
        while (mRUIterator!=mModel->ruList().constEnd()) {
            if ((*mRUIterator)->trees().count()>0)
                break;
            ++mRUIterator;
        }
            // finished if all RU processed
        if (mRUIterator == mModel->ruList().constEnd())
            return nullptr;
        mTreeEnd = &((*mRUIterator)->trees().back()) + 1; // let end point to "1 after end" (STL-style)
        mCurrent = &((*mRUIterator)->trees().front());
    }
    if (mCurrent==mTreeEnd) {
        ++mRUIterator; // switch to next RU (loop until RU with trees is found)
        while (mRUIterator!=mModel->ruList().constEnd()) {
            if ((*mRUIterator)->trees().count()>0) {
                break;
            }
            ++mRUIterator;
        }
        if (mRUIterator == mModel->ruList().constEnd()) {
            mCurrent = nullptr;
            return nullptr; // finished!!
        }else {
            mTreeEnd = &((*mRUIterator)->trees().back()) + 1;
            mCurrent = &((*mRUIterator)->trees().front());
        }
    }

    return mCurrent++;
}
Tree *AllTreeIterator::nextLiving()
{
    while (Tree *t = next())
        if (!t->isDead()) return t;
    return nullptr;
}
Tree *AllTreeIterator::current() const
{
    return mCurrent?mCurrent-1:nullptr;
}

/// multithreaded execution of the microclimate routine
static void nc_microclimate(ResourceUnit *unit)
{
    try {
        unit->analyzeMicroclimate();

    } catch (const IException &e) {
        // thread-safe error message
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }
}


ModelSettings Model::mSettings;
Model::Model()
{
    initialize();
    GlobalSettings::instance()->setModel(this);
    GlobalSettings::instance()->resetScriptEngine(); // clear the script
    QString dbg="extended debug checks disabled.";
    DBGMODE( dbg="extended debug checks enabled."; );
    qDebug() << dbg;
}

Model::~Model()
{
    clear();
    GlobalSettings::instance()->setModel(nullptr);
}

/** Initial setup of the Model.
  */
void Model::initialize()
{
   mSetup = false;
   GlobalSettings::instance()->setCurrentYear(0);
   mGrid = nullptr;
   mHeightGrid = nullptr;
   mManagement = nullptr;
   mABEManagement = nullptr;
   mBiteEngine = nullptr;
   mEnvironment = nullptr;
   mTimeEvents = nullptr;
   mStandGrid = nullptr;
   mModules = nullptr;
   mDEM = nullptr;
   mGrassCover = nullptr;
   mSaplings=nullptr;
   mSVDStates=nullptr;
}

/** sets up the simulation space.
*/
void Model::setupSpace()
{
    setCurrentTask("setup landscape");
    GeoTIFF::clearProjection(); // first chance to load a tif

    XmlHelper xml(GlobalSettings::instance()->settings().node("model.world"));
    double cellSize = xml.value("cellSize", "2").toDouble();
    double width = xml.value("width", "100").toDouble();
    double height = xml.value("height", "100").toDouble();
    double buffer = xml.value("buffer", "60").toDouble();
    if (fmod(width, 100.)!=0. || fmod(height, 100.)!=0. || fmod(buffer, 20.)!=0. || buffer<=0.) {
        throw IException("setup of the world: 'width' and 'height' need to be multiple of 100, 'buffer' a multiple of 20 (>0).");
    }
    mModelRect = QRectF(0., 0., width, height);

    qDebug() << QString("setup of the world: %1x%2m with cell-size=%3m and %4m buffer").arg(width).arg(height).arg(cellSize).arg(buffer);

    QRectF total_grid(QPointF(-buffer, -buffer), QPointF(width+buffer, height+buffer));
    qDebug() << "setup grid rectangle:" << total_grid;

    if (mGrid)
        delete mGrid;
    mGrid = new FloatGrid(total_grid, static_cast<float>(cellSize));
    if (mGrid->isEmpty()) {
        throw IException("setup of the world: definition of project area (width/height/buffer) invalid or too large.");
    }
    mGrid->initialize(1.f);
    if (mHeightGrid)
        delete mHeightGrid;
    mHeightGrid = new HeightGrid(total_grid, static_cast<float>(cellSize)*cPxPerHeight);
    mHeightGrid->wipe(); // set all to zero
    Tree::setGrid(mGrid, mHeightGrid);

    // setup the spatial location of the project area
    if (xml.hasNode("location")) {
        // setup of spatial location
        double loc_x = xml.valueDouble("location.x");
        double loc_y = xml.valueDouble("location.y");
        double loc_z = xml.valueDouble("location.z");
        double loc_rot = xml.valueDouble("location.rotation");
        setupGISTransformation(loc_x, loc_y, loc_z, loc_rot);
        qDebug() << "setup of spatial location: x/y/z" << loc_x << loc_y << loc_z << "rotation:" << loc_rot;
    } else {
        setupGISTransformation(0., 0., 0., 0.);
    }

    // load environment (multiple climates, speciesSets, ... )
    if (mEnvironment)
        delete mEnvironment;
    mEnvironment = new Environment();

    if (xml.valueBool("environmentEnabled", false)) {
        QString env_file = GlobalSettings::instance()->path(xml.value("environmentFile"));
        bool grid_mode = (xml.value("environmentMode")=="grid");
        QString grid_file = GlobalSettings::instance()->path(xml.value("environmentGrid"));
        if (grid_mode) {
            if (QFile::exists(grid_file) && !xml.value("environmentGrid").isEmpty())
                mEnvironment->setGridMode(grid_file);
            else
                throw IException(QString("File '%1' specified in key 'environmentGrid' does not exit ('environmentMode' is 'grid').").arg(grid_file) );
        }

        if (!mEnvironment->loadFromFile(env_file))
            return;
    } else {
        // load and prepare default values
        // (2) SpeciesSets: currently only one a global species set.
        SpeciesSet *speciesSet = new SpeciesSet();
        mSpeciesSets.push_back(speciesSet);
        speciesSet->setup();
        // Climate...
        Climate *c = new Climate();
        mClimates.push_back(c);
        mEnvironment->setDefaultValues(c, speciesSet);
    } // environment?

    // time series data
    if (xml.valueBool(".timeEventsEnabled", false)) {
        mTimeEvents = new TimeEvents();
        mTimeEvents->loadFromFile(GlobalSettings::instance()->path(xml.value("timeEventsFile"), "script"));
    }


    // simple case: create ressource units in a regular grid.
    bool has_stand_grid = false;
    if (xml.valueBool("resourceUnitsAsGrid")) {

        mRUmap.setup(QRectF(0., 0., width, height),100.); // Grid, that holds positions of resource units
        mRUmap.wipe();

        bool mask_is_setup = false;
        if (xml.valueBool("standGrid.enabled")) {
            QString fileName = GlobalSettings::instance()->path(xml.value("standGrid.fileName"));
            mStandGrid = new MapGrid(fileName,false); // create stand grid index later
            if (!mStandGrid->isValid()) {
                throw IException("Error loading stand grid '" + fileName + "'.");
            } else {
                has_stand_grid = true;
            }


            for (int i=0;i<mStandGrid->grid().count();i++) {
                const int &grid_value = mStandGrid->grid().constValueAtIndex(i);
                mHeightGrid->valueAtIndex(i).setValid( grid_value > -1 );
                if (grid_value>-1)
                    mRUmap.valueAt(mStandGrid->grid().cellCenterPoint(i)) = (ResourceUnit*)1;
                if (grid_value < -1)
                    mHeightGrid->valueAtIndex(i).setForestOutside(true);
            }

            mask_is_setup = true;
        } else {
            if (!settings().torusMode) {
                // in the case we have no stand grid but only a large rectangle (without the torus option)
                // we assume a forest outside
                for (int i=0;i<mHeightGrid->count();++i) {
                    const QPointF &p = mHeightGrid->cellCenterPoint(mHeightGrid->indexOf(i));
                    if (p.x() < 0. || p.x()>width || p.y()<0. || p.y()>height) {
                        mHeightGrid->valueAtIndex(i).setForestOutside(true);
                        mHeightGrid->valueAtIndex(i).setValid(false);
                    }
                }

            }
        }

        ResourceUnit **p; // ptr to ptr!
        ResourceUnit *new_ru;

        int ru_index = 0;
        int ru_skipped = 0;
        for (p=mRUmap.begin(); p!=mRUmap.end(); ++p) {
            QRectF r = mRUmap.cellRect(mRUmap.indexOf(p));
            if (!has_stand_grid || *p!=nullptr) {
                mEnvironment->setPosition( r.center() ); // if environment is 'disabled' default values from the project file are used.
                if (mEnvironment->currentID() >= 0) {
                    // create resource units for valid positions only
                    new_ru = new ResourceUnit(ru_index++); // create resource unit
                    new_ru->setBoundingBox(r);
                    new_ru->setID( mEnvironment->currentID() ); // set id of resource unit in grid mode
                    new_ru->setClimate( mEnvironment->climate() );
                    if (!mEnvironment->climate()) {
                        QString err_msg = QString("Setup of landscape: Trying to set up a resource unit " \
                                                  "with center point (%1/%2), but no climate is defined in the environment for that location. \n " \
                                                  "Check spatial extent of your stand grid / environment grid, and the log file.").
                                          arg(r.center().x()).arg(r.center().y());
                        throw IException(err_msg);
                    }
                    new_ru->setSpeciesSet( mEnvironment->speciesSet() );
                    new_ru->setup();
                    mRU.append(new_ru);
                    *p = new_ru; // save in the RUmap grid
                } else {
                    // resource unit grid is null
                    if (*p != 0) {
                        QString err_msg = QString("Setup of landscape: The resource unit " \
                                                  "with center point (%1/%2) is null (according to environment grid), " \
                                                  "but the stand grid seems to have valid stands there. This is not good. \n" \
                                                  "Check for overlap of your stand grid / environment grid, and the log file.").
                                          arg(r.center().x()).arg(r.center().y());
                        throw IException(err_msg);
                    }
                    ru_skipped++;


                }
            } else {
                ru_skipped++;
            }
        }
        qDebug() << "Created" << ru_index << "resource units (" << ru_skipped << " resource units skipped).";
        if (mEnvironment) {
            // retrieve species sets and climates (that were really used)
            mSpeciesSets << mEnvironment->speciesSetList();
            mClimates << mEnvironment->climateList();
            QString climate_file_list;
            for (int i=0, c=0;i<mClimates.count();++i) {
                climate_file_list += mClimates[i]->name() + ", ";
                if (++c>5) {
                    climate_file_list += "...";
                    break;
                }

            }
            qDebug() << "Setup of climates: #loaded:" << mClimates.count() << "tables:" << climate_file_list;
            qDebug() << "setup of" << mEnvironment->climateList().size() << "climates performed.";
        }


        if (mStandGrid && mStandGrid->isValid()) {
            mStandGrid->createIndex();
            GlobalSettings::instance()->controller()->addScriptLayer(nullptr, mStandGrid, "iLand standGrid");
            qDebug() << "Loaded stand grid from " << mStandGrid->name() << ", #stands: " << mStandGrid->count();
        }
        // now store the pointers in the grid.
        // Important: This has to be done after the mRU-QList is complete - otherwise pointers would
        // point to invalid memory when QList's memory is reorganized (expanding)
//        ru_index = 0;
//        for (p=mRUmap.begin();p!=mRUmap.end(); ++p) {
//            *p = mRU.value(ru_index++);
//        }
        qDebug() << "created a grid of ResourceUnits: count=" << mRU.count() << "number of RU-map-cells:" << mRUmap.count();


        calculateStockableArea();

        // setup of the project area mask
        if (!mask_is_setup && xml.valueBool("areaMask.enabled", false) && xml.hasNode("areaMask.imageFile")) {
            // to be extended!!! e.g. to load ESRI-style text files....
            // setup a grid with the same size as the height grid...
            FloatGrid tempgrid(static_cast<int>(mHeightGrid->cellsize()), mHeightGrid->sizeX(), mHeightGrid->sizeY());
            QString fileName = GlobalSettings::instance()->path(xml.value("areaMask.imageFile"));
            qDebug() << "loading project area mask from" << fileName << "...";
            loadGridFromImage(fileName, tempgrid); // fetch from image
            for (int i=0;i<tempgrid.count(); i++)
                mHeightGrid->valueAtIndex(i).setValid( tempgrid.valueAtIndex(i)>0.99f );

        }

        // list of "valid" resource units
        QList<ResourceUnit*> valid_rus;
        foreach(ResourceUnit* ru, mRU)
            if (ru->id()!=-1)
                valid_rus.append(ru);

        // setup of the digital elevation map (if present)
        QString dem_file = xml.value("DEM");
        if (!dem_file.isEmpty()) {
            mDEM = new DEM(GlobalSettings::instance()->path(dem_file));
            // add them to the visuals...
            GlobalSettings::instance()->controller()->addGrid(mDEM, "DEM - height", GridViewRainbow, 0, 1000);
            GlobalSettings::instance()->controller()->addGrid(mDEM->slopeGrid(), "DEM - slope", GridViewRainbow, 0, 3);
            GlobalSettings::instance()->controller()->addGrid(mDEM->aspectGrid(), "DEM - aspect", GridViewRainbow, 0, 360);
            GlobalSettings::instance()->controller()->addGrid(mDEM->viewGrid(), "DEM - view", GridViewGray, 0, 1);

        }

        // setup of saplings
        if (mSaplings) {
            delete mSaplings; mSaplings=nullptr;
        }
        if (settings().regenerationEnabled) {
            mSaplings = new Saplings();
            mSaplings->setup();
        }


        // setup of the grass cover
        if (!mGrassCover)
            mGrassCover = new GrassCover();
        mGrassCover->setup();

        // setup of external modules
        mModules->setup();
        if (mModules->hasSetupResourceUnits()) {
            for (p=mRUmap.begin(); p!=mRUmap.end(); ++p) {
                if (*p) {
                    QRectF r = mRUmap.cellRect(mRUmap.indexOf(p));
                    mEnvironment->setPosition( r.center() ); // if environment is 'disabled' default values from the project file are used.
                    mModules->setupResourceUnit( *p );
                }
            }
        }

        // setup of scripting environment
        ScriptGlobal::setupGlobalScripting();

        // setup the helper that does the multithreading
        bool do_multithreading =GlobalSettings::instance()->settings().valueBool("system.settings.multithreading");
        int n_threads = GlobalSettings::instance()->settings().valueInt("system.settings.threadCount",-1);
        if (do_multithreading) {
            if (n_threads>0) {
                QThreadPool::globalInstance()->setMaxThreadCount(n_threads);
                qDebug() << "Multithreading: set max thread count to" << n_threads;
            } else {
                QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount()); // reset
            }
        }
        threadRunner.setup(valid_rus);
        threadRunner.setMultithreading(do_multithreading);
        threadRunner.print();


    } else  {
        throw IException("resourceUnitsAsGrid MUST be set to true - at least currently :)");
    }
    mSetup = true;
}


/** clear() frees all ressources allocated with the run of a simulation.

  */
void Model::clear()
{
    mSetup = false;
    qDebug() << "Model clear: attempting to clear" << mRU.count() << "RU, " << mSpeciesSets.count() << "SpeciesSets.";
    // clear ressource units
    qDeleteAll(mRU); // delete ressource units (and trees)
    mRU.clear();

    qDeleteAll(mSpeciesSets); // delete species sets
    mSpeciesSets.clear();

    // delete climate data
    qDeleteAll(mClimates);

    // delete the grids
    if (mGrid)
        delete mGrid;
    if (mHeightGrid)
        delete mHeightGrid;
    if (mSaplings)
        delete mSaplings;
    if (mManagement)
        delete mManagement;
    if (mEnvironment)
        delete mEnvironment;
    if (mTimeEvents)
        delete mTimeEvents;
    if (mStandGrid) {
        GlobalSettings::instance()->controller()->removeMapGrid(nullptr, mStandGrid);
        delete mStandGrid;
    }
    if (mModules)
        delete mModules;
    if (mDEM)
        delete mDEM;
    if (mGrassCover)
        delete mGrassCover;
    if (mABEManagement)
        delete mABEManagement;
    if (mSVDStates)
        delete mSVDStates;
    if (mBiteEngine)
        delete  mBiteEngine;

    mGrid = nullptr;
    mHeightGrid = nullptr;
    mManagement = nullptr;
    mEnvironment = nullptr;
    mTimeEvents = nullptr;
    mStandGrid  = nullptr;
    mModules = nullptr;
    mDEM = nullptr;
    mGrassCover = nullptr;
    mABEManagement = nullptr;
    mBiteEngine = nullptr;
    mSVDStates = nullptr;

    GlobalSettings::instance()->outputManager()->close();

    qDebug() << "Model resources freed.";
}

/** Setup of the Simulation.
  This really creates the simulation environment and does the setup of various aspects.
  */
void Model::loadProject()
{
    DebugTimer dt("load project");
    setCurrentTask("Loading project area....");
    GlobalSettings *g = GlobalSettings::instance();
    g->printDirectories();
    const XmlHelper &xml = g->settings();

    // load javascript code into the engine
    QString script_file = xml.value("system.javascript.fileName");
    if (!script_file.isEmpty()) {
        script_file = g->path(script_file, "script");
        ScriptGlobal::loadScript(script_file);
        g->controller()->setLoadedJavascriptFile(script_file);
        // call the global event very early in the process of creating the model
        GlobalSettings::instance()->executeJSFunction("onBeforeCreate");
    }


    g->clearDatabaseConnections();
    // database connections: reset
    GlobalSettings::instance()->clearDatabaseConnections();
    // input and climate connection
    // see initOutputDatabase() for output database
    QString dbPath = g->path( xml.value("system.database.in"), "database");
    GlobalSettings::instance()->setupDatabaseConnection("in", dbPath, true);
    dbPath = g->path( xml.value("system.database.climate"), "database");
    GlobalSettings::instance()->setupDatabaseConnection("climate", dbPath, true);

    mSettings.loadModelSettings();
    mSettings.print();

    DebugTimer::setResponsiveMode(xml.valueBool("system.settings.responsive"));

    // random seed: if stored value is <> 0, use this as the random seed (and produce hence always an equal sequence of random numbers)
    uint seed = xml.value("system.settings.randomSeed","0").toUInt();
    RandomGenerator::setup(RandomGenerator::ergMersenneTwister, seed); // use the MersenneTwister as default
    // linearization of expressions: if true *and* linearize() is explicitely called, then
    // function results will be cached over a defined range of values.
    bool do_linearization = xml.valueBool("system.settings.expressionLinearizationEnabled", false);
    Expression::setLinearizationEnabled(do_linearization);
    if (do_linearization)
        qDebug() << "The linearization of expressions is enabled (performance optimization).";

    // log level
    QString log_level = xml.value("system.settings.logLevel", "debug").toLower();
    if (log_level=="debug") setLogLevel(0);
    if (log_level=="info") setLogLevel(1);
    if (log_level=="warning") setLogLevel(2);
    if (log_level=="error") setLogLevel(3);

    // snag dynamics / soil model enabled? (info used during setup of world)
    changeSettings().carbonCycleEnabled = xml.valueBool("model.settings.carbonCycleEnabled", false);
    // class size of snag classes
    Snag::setupThresholds(xml.valueDouble("model.settings.soil.swdDBHClass12"),
                          xml.valueDouble("model.settings.soil.swdDBHClass23"),
                          xml.valueDouble("model.settings.soil.swdDBHSingle", 1000.),
                          xml.value("model.settings.soil.decayClassThresholds", "0.2, 0.4, 0.7, 0.9"));

    // setup of modules
    if (mModules)
        delete mModules;
    mModules = new Modules();

    changeSettings().regenerationEnabled = xml.valueBool("model.settings.regenerationEnabled", false);


    setupSpace();
    if (mRU.isEmpty())
        throw IException("Setup of Model: no valid resource units found. Check extent (width and height of sim area), location (x,y offset for GIS data), and environment grid (plus stand grid). And the log.");

    // (3) additional issues

    // (3.2) setup of regeneration
    if (settings().regenerationEnabled) {
        foreach(SpeciesSet *ss, mSpeciesSets)
            ss->setupRegeneration();
    }
    Saplings::setRecruitmentVariation(xml.valueDouble("model.settings.seedDispersal.recruitmentDimensionVariation",0.1));

    // (3.3) management
    bool use_abe = xml.valueBool("model.management.abeEnabled");
    if (use_abe) {
        // use the agent based forest management engine
        mABEManagement = new ABE::ForestManagementEngine();
        // setup of ABE after loading of trees.
    }
    // use the standard management
    QString mgmtFile = xml.value("model.management.file");
    if (xml.valueBool("model.management.enabled")) {
        mManagement = new Management();
        QString path = GlobalSettings::instance()->path(mgmtFile, "script");
        mManagement->loadScript(path);
        qDebug() << "setup management using script" << path;
    }

    // SVD States
    if (mSVDStates) {
        delete mSVDStates; mSVDStates=nullptr;
    }
    if (xml.valueBool("model.settings.svdStates.enabled", false))
        mSVDStates=new SVDStates();

    // biotic disturbance module BITE
    if (mBiteEngine) {
        delete mBiteEngine; mBiteEngine=nullptr;
    }
    if (xml.valueBool("modules.bite.enabled", false)) {
        mBiteEngine = BITE::BiteEngine::instance();
        mBiteEngine->setup();
    }


}

void Model::reloadABE()
{
    // delete firest
    if (mABEManagement)
        delete mABEManagement;
    mABEManagement = new ABE::ForestManagementEngine();
    // and setup
    mABEManagement->setup();
    mABEManagement->runOnInit(true);

    mABEManagement->initialize();

    mABEManagement->runOnInit(false);

}


ResourceUnit *Model::ru(QPointF coord)
{
    if (!mRUmap.isEmpty() && mRUmap.coordValid(coord))
        return mRUmap.valueAt(coord);
    if (mRUmap.isEmpty())
        return ru(); // default RU if there is only one
    else
        return nullptr; // in this case, no valid coords were provided
}

ResourceUnit *Model::ruById(int id) const
{
    for (int i=0;i<mRU.size();++i)
        if (mRU[i]->id()==id)
            return mRU[i];
    return nullptr;
}

void Model::initOutputDatabase()
{
    GlobalSettings *g = GlobalSettings::instance();
    QString dbPath = g->path(g->settings().value("system.database.out"), "output");
    // replace path information
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    dbPath.replace("$date$", timestamp);
    // setup final path
   g->setupDatabaseConnection("out", dbPath, false);

   // create table for run meta data
   QSqlQuery creator(g->dbout());
   QString drop=QString("drop table if exists runinfo");
   creator.exec(drop); // drop table (if exists)
   creator.exec("create table runinfo (timestamp, version)");
   SqlHelper::executeSql(QString("insert into runinfo (timestamp, version) values ('%1', '%2')").arg(timestamp).arg(verboseVersion()), g->dbout());

}

/// multithreaded run function for resource unit level establishment
static void nc_establishment(ResourceUnit *unit)
{
    Saplings *s = GlobalSettings::instance()->model()->saplings();
    try {
        s->establishment(unit);

    } catch (const IException& e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }

}

/// multithreaded run function for resource unit level establishment
static void nc_sapling_growth(ResourceUnit *unit)
{
    Saplings *s = GlobalSettings::instance()->model()->saplings();
    try {
        s->saplingGrowth(unit);

    } catch (const IException& e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }
}



/// multithreaded execution of the carbon cycle routine
static void nc_carbonCycle(ResourceUnit *unit)
{
    try {
        // (1) do calculations on snag dynamics for the resource unit
        unit->calculateCarbonCycle();
        // (2) do the soil carbon and nitrogen dynamics calculations (ICBM/2N)
    } catch (const IException& e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }

}


/// beforeRun performs several steps before the models starts running.
/// inter alia: * setup of the stands
///             * setup of the climates
void Model::beforeRun()
{
    // setup outputs
    // setup output database
    if (GlobalSettings::instance()->dbout().isOpen())
        GlobalSettings::instance()->dbout().close();
    initOutputDatabase();
    GlobalSettings::instance()->outputManager()->setup();
    GlobalSettings::instance()->clearDebugLists();

    // initialize stands
    setCurrentTask("loading initialization");
    StandLoader loader(this);
    {
    DebugTimer loadtrees("load trees");
    loader.processInit();
    }
    // initalization of ABE
    if (mABEManagement) {
        mABEManagement->setup();
        mABEManagement->runOnInit(true);
    }

    // load climate
    setCurrentTask("loading climate");
    {
        if (logLevelDebug()) qDebug() << "attempting to load climate..." ;
        DebugTimer loadclim("load climate");
        foreach(Climate *c, mClimates) {
            if (!c->isSetup())
                c->setup();
        }
        // load the first year of the climate database
        foreach(Climate *c, mClimates)
            c->nextYear();

    }

    // microclimate
    if (Model::settings().microclimateEnabled) {
        MicroclimateVisualizer::setupVisualization();
        DebugTimer t("Microclimate setup");
        executePerResourceUnit(nc_microclimate, false /* true to force single threaded execution */);

    }

    setCurrentTask("loading initialization (finalize)");
    { DebugTimer loadinit("load standstatistics");
    if (logLevelDebug()) qDebug() << "attempting to calculate initial stand statistics (incl. apply and read pattern)..." ;
    Tree::setGrid(mGrid, mHeightGrid);
    // debugCheckAllTrees(); // introduced for debugging session (2012-04-06)
    applyPattern();
    readPattern();
    loader.processAfterInit(); // e.g. initialization of saplings

    // force the compilation of initial stand statistics
    createStandStatistics();
    }

    // initalization of ABE (now all stands are properly set up)
    if (mABEManagement) {
        mABEManagement->initialize();
        mABEManagement->runOnInit(false);
    }

    setCurrentTask("outputs during startup");
    // outputs to create with inital state (without any growth) are called here:
    GlobalSettings::instance()->setCurrentYear(0); // set clock to "0" (for outputs with initial state)

    GlobalSettings::instance()->outputManager()->execute("stand"); // year=0
    GlobalSettings::instance()->outputManager()->execute("landscape"); // year=0
    GlobalSettings::instance()->outputManager()->execute("sapling"); // year=0
    GlobalSettings::instance()->outputManager()->execute("saplingdetail"); // year=0
    GlobalSettings::instance()->outputManager()->execute("tree"); // year=0
    GlobalSettings::instance()->outputManager()->execute("dynamicstand"); // year=0
    GlobalSettings::instance()->outputManager()->execute("carbon"); // year=0
    GlobalSettings::instance()->outputManager()->execute("svdstate"); // year=0
    GlobalSettings::instance()->outputManager()->execute("devstage"); // year=0
    GlobalSettings::instance()->outputManager()->execute("ecoviz"); // tree output for visualization, year 0
    GlobalSettings::instance()->outputManager()->execute("customagg"); // custom aggregation, much like dynamic stand, year 0
    GlobalSettings::instance()->outputManager()->save(); // commit database changes


    GlobalSettings::instance()->setCurrentYear(1); // set to first year

}

/** Main model run routine.
  The sequence of actions is as follows:
  (1) Load the climate of the new year
  (2) Reset statistics for resource unit as well as for dead/managed trees
  (3) Invoke Management.
  (4) *after* that, calculate Light patterns
  (5) 3PG on stand level, tree growth. Clear stand-statistcs before they are filled by single-tree-growth. calculate water cycle (with LAIs before management)
  (6) execute Regeneration
  (7) invoke disturbance modules
  (8) calculate carbon cycle
  (9) calculate statistics for the year
  (10) write database outputs
  */
void Model::runYear()
{
    DebugTimer t_all("Model::runYear()");
    GlobalSettings::instance()->systemStatistics()->reset();
    threadRunner.clearErrors();
    RandomGenerator::checkGenerator(); // see if we need to generate new numbers...
    // initalization at start of year for external modules
    mModules->yearBegin();

    // execute scheduled events for the current year
    if (mTimeEvents)
        mTimeEvents->run();

    // load the next year of the climate database (except for the first year - the first climate year is loaded immediately
    if (GlobalSettings::instance()->currentYear()>1) {
        foreach(Climate *c, mClimates)
            c->nextYear();
    }
    // run microclimate
    if (Model::settings().microclimateEnabled) {
        DebugTimer t("Microclimate");
        executePerResourceUnit(nc_microclimate, false /* true to force single threaded execution */);
    }

    WaterCycle::resetPsiMin();

    // reset statistics
    foreach(ResourceUnit *ru, mRU)
        ru->newYear();

    foreach(SpeciesSet *set, mSpeciesSets)
        set->newYear();

    // management classic
    if (mManagement) {
        setCurrentTask("Management");
        DebugTimer t("management");
        mManagement->run();
        GlobalSettings::instance()->systemStatistics()->tManagement+=t.elapsed();
    }
    // ... or ABE (the agent based variant)
    if (mABEManagement) {
        DebugTimer t("ABE:run");
        setCurrentTask("ABE");
        mABEManagement->run();
        GlobalSettings::instance()->systemStatistics()->tManagement+=t.elapsed();
    }

    // if trees are dead/removed because of management, the tree lists
    // need to be cleaned (and the statistics need to be recreated)
    cleanTreeLists(true); // recalculate statistics (LAIs per species needed later in production)

    // process a cycle of individual growth
    setCurrentTask("apply LIP");
    applyPattern(); // create Light Influence Patterns
    setCurrentTask("read LIP");
    readPattern(); // readout light state of individual trees

    setCurrentTask("tree growth");
    grow(); // let the trees grow (growth on stand-level, tree-level, mortality)

    mGrassCover->execute(); // evaluate the grass / herb cover (and its effect on regeneration)

    // regeneration
    if (settings().regenerationEnabled) {
        // seed dispersal
        setCurrentTask("Seed dispersal");
        DebugTimer tseed("Seed dispersal, establishment, sapling growth");
        foreach(SpeciesSet *set, mSpeciesSets)
            set->regeneration(); // parallel execution for each species set

        GlobalSettings::instance()->systemStatistics()->tSeedDistribution+=tseed.elapsed();

        // establishment
        Saplings::updateBrowsingPressure();


        { DebugTimer t("establishment");
        setCurrentTask("Establishment");
        executePerResourceUnit( nc_establishment, false /* true: force single threaded operation */);
        GlobalSettings::instance()->systemStatistics()->tEstablishment+=t.elapsed();
        }
        { DebugTimer t("sapling growth");
        setCurrentTask("sapling growth");

        foreach(SpeciesSet *set, mSpeciesSets) {
            // the sapling seed maps are cleared before sapling growth (where sapling seed maps are filled)
            // the content of the seed maps is used in the *next* year
            set->clearSaplingSeedMap();
        }

        executePerResourceUnit( nc_sapling_growth, false /* true: force single threaded operation */);
        GlobalSettings::instance()->systemStatistics()->tSapling+=t.elapsed();
        }

        mGrassCover->executeAfterRegeneration(); // evaluate ground vegetation

        // Establishment::debugInfo(); // debug test
        threadRunner.checkErrors();

    }

    // external modules/disturbances
    setCurrentTask("BITE");
    if (mBiteEngine) {
        mBiteEngine->setYear(GlobalSettings::instance()->currentYear());
        mBiteEngine->run();
    }

    setCurrentTask("Disturbance modules");
    mModules->run();
    // cleanup of tree lists if external modules removed trees.
    cleanTreeLists(false); // do not recalculate statistics - this is done in ru->yearEnd()


    // calculate soil / snag dynamics
    if (settings().carbonCycleEnabled) {
        DebugTimer ccycle("carbon cylce");
        setCurrentTask("carbon cycle");
        executePerResourceUnit( nc_carbonCycle, false /* true: force single threaded operation */);
        GlobalSettings::instance()->systemStatistics()->tCarbonCycle+=ccycle.elapsed();

    }


    DebugTimer toutput("outputs");
    // calculate statistics
    foreach(ResourceUnit *ru, mRU)
        ru->yearEnd();

    if (mABEManagement) {
        DebugTimer t("ABE:yearEnd");
        setCurrentTask("ABE yearEnd");
        mABEManagement->yearEnd();
        GlobalSettings::instance()->systemStatistics()->tManagement+=t.elapsed();
    }

    threadRunner.checkErrors();

    // create outputs
    setCurrentTask("Write outputs");
    OutputManager *om = GlobalSettings::instance()->outputManager();
    om->execute("tree"); // single tree output
    om->execute("treeremoved"); // single removed tree output
    om->execute("stand"); //resource unit level x species
    om->execute("landscape"); //landscape x species
    om->execute("landscape_removed"); //removed trees on landscape x species
    om->execute("sapling"); // sapling layer per RU x species
    om->execute("saplingdetail"); // individual sapling cohorts (per RU)
    om->execute("production_month"); // 3pg responses growth per species x RU x month
    om->execute("dynamicstand"); // output with user-defined columns (based on species x RU)
    om->execute("standdead"); // resource unit level x species
    om->execute("management"); // resource unit level x species
    om->execute("carbon"); // resource unit level, carbon pools above and belowground
    om->execute("carbonflow"); // resource unit level, GPP, NPP and total carbon flows (atmosphere, harvest, ...)
    om->execute("soilinput"); // resource unit level carbon input to the soil
    om->execute("water"); // resource unit/landscape level water output (ET, rad, snow cover, ...)
    om->execute("svdgpp"); // pot. gpp per m2 and for a number of species (SVD related)
    om->execute("svdstate"); // forest state information (SVD related)
    om->execute("svdindicator"); // forest indicators on RU level (SVD related)
    om->execute("svduniquestate"); // list of forest vegetation states (SVD related)
    om->execute("devstage"); // spatial analysis of developement stages
    om->execute("ecoviz"); // tree output for visualization
    om->execute("customagg"); // custom aggregation, much like dynamic stand

    GlobalSettings::instance()->systemStatistics()->tWriteOutput+=toutput.elapsed();
    GlobalSettings::instance()->systemStatistics()->tTotalYear+=t_all.elapsed();
    GlobalSettings::instance()->systemStatistics()->writeOutput();

    // global javascript event
    GlobalSettings::instance()->executeJSFunction("onYearEnd");

    GlobalSettings::instance()->setCurrentYear(GlobalSettings::instance()->currentYear()+1);

    // try to clean up a bit of memory (useful if many large JS objects (e.g., grids) are used)
    GlobalSettings::instance()->scriptEngine()->collectGarbage();
}



void Model::afterStop()
{
    // do some cleanup
}

/// multithreaded running function for LIP printing
static void nc_applyPattern(ResourceUnit *unit)
{

    QVector<Tree>::iterator tit;
    QVector<Tree>::iterator tend = unit->trees().end();

    try {

        // light concurrence influence
        if (!GlobalSettings::instance()->model()->settings().torusMode) {
            // height dominance grid
            for (tit=unit->trees().begin(); tit!=tend; ++tit) {
                (*tit).heightGrid(); // just do it ;)
            }

            for (tit=unit->trees().begin(); tit!=tend; ++tit)
                (*tit).applyLIP(); // just do it ;)

        } else {
            // height dominance grid
            for (tit=unit->trees().begin(); tit!=tend; ++tit)
                (*tit).heightGrid_torus(); // just do it ;)

            for (tit=unit->trees().begin(); tit!=tend; ++tit)
                (*tit).applyLIP_torus(); // do it the wraparound way
        }

    } catch (const IException &e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }
}

/// multithreaded running function for LIP value extraction
static void nc_readPattern(ResourceUnit *unit)
{
    QVector<Tree>::iterator tit;
    QVector<Tree>::iterator  tend = unit->trees().end();
    try {
        if (!GlobalSettings::instance()->model()->settings().torusMode) {
            for (tit=unit->trees().begin(); tit!=tend; ++tit)
                (*tit).readLIF(); // multipliactive approach
        } else {
            for (tit=unit->trees().begin(); tit!=tend; ++tit)
                (*tit).readLIF_torus(); // do it the wraparound way
        }
    } catch (const IException &e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }
}

/// multithreaded running function for growth of individual trees
static void nc_grow(ResourceUnit *unit)
{
    QVector<Tree>::iterator tit;
    QVector<Tree>::iterator  tend = unit->trees().end();
    try {
        unit->beforeGrow(); // reset statistics
        // calculate light responses
        // responses are based on *modified* values for LightResourceIndex
        for (tit=unit->trees().begin(); tit!=tend; ++tit) {
            (*tit).calcLightResponse();
        }

        unit->calculateInterceptedArea();

        for (tit=unit->trees().begin(); tit!=tend; ++tit) {
            (*tit).grow(); // actual growth of individual trees
        }
    } catch (const IException &e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }

    GlobalSettings::instance()->systemStatistics()->treeCount+=unit->trees().count();
}

/// multithreaded running function for resource level production
static void nc_production(ResourceUnit *unit)
{
    try {
        unit->production();
    } catch (const IException &e) {
        GlobalSettings::instance()->model()->threadExec().throwError(e.message());
    }
}


void Model::test()
{
    // Test-funktion: braucht 1/3 time von readGrid()
    DebugTimer t("test");
    FloatGrid averaged = mGrid->averaged(10);
    int count = 0;
    float *end = averaged.end();
    for (float *p=averaged.begin(); p!=end; ++p)
        if (*p > 0.9f)
            count++;
    qDebug() << count << "LIF>0.9 of " << averaged.count();
}

void Model::debugCheckAllTrees()
{
    AllTreeIterator at(this);
    bool has_errors = false; double dummy=0.;
    while (Tree *t = at.next()) {
        // plausibility
        if (t->dbh()<0 || t->dbh()>10000.f || t->biomassFoliage()<0.f || t->height()>1000.f || t->height() < 0.f
                || t->biomassFoliage() <0.f)
            has_errors = true;
        // check for objects....
        dummy = t->stamp()->offset() + t->ru()->ruSpecies()[1]->statistics().count();
    }
    if (has_errors)
        qDebug() << "model: debugCheckAllTrees found problems" << dummy;
}

void Model::applyPattern()
{

    DebugTimer t("applyPattern()");
    // intialize grids...
    initializeGrid();

    // initialize height grid with a default value of 4m. This is the height of the regeneration layer
    for (HeightGridValue *h=mHeightGrid->begin();h!=mHeightGrid->end();++h) {
        h->resetCount(); // set count = 0, but do not touch the flags
        h->height = cSapHeight;
        h->clearStemHeight();
    }

    threadRunner.run(nc_applyPattern);
    GlobalSettings::instance()->systemStatistics()->tApplyPattern+=t.elapsed();
}

void Model::readPattern()
{
    DebugTimer t("readPattern()");
    threadRunner.run(nc_readPattern);
    GlobalSettings::instance()->systemStatistics()->tReadPattern+=t.elapsed();

}

/** Main function for the growth of stands and trees.
   This includes several steps.
   (1) calculate the stocked area (i.e. count pixels in height grid)
   (2) 3PG production (including response calculation, water cycle)
   (3) single tree growth (including mortality)
   (4) cleanup of tree lists (remove dead trees)
  */
void Model::grow()
{


    { DebugTimer t("growRU()");
    calculateStockedArea();

    // Production of biomass (stand level, 3PG)
    threadRunner.run(nc_production);
    }

    DebugTimer t("growTrees()");
    threadRunner.run(nc_grow); // actual growth of individual trees

    foreach(ResourceUnit *ru, mRU) {
       ru->cleanTreeList();
       ru->afterGrow();
       //qDebug() << (b-n) << "trees died (of" << b << ").";
   }
   GlobalSettings::instance()->systemStatistics()->tTreeGrowth+=t.elapsed();
}

/** calculate for each resource unit the fraction of area which is stocked.
  This is done by checking the pixels of the global height grid.
  */
void Model::calculateStockedArea()
{
    // iterate over the whole heightgrid and count pixels for each ressource unit
    HeightGridValue *end = mHeightGrid->end();
    QPointF cp;
    ResourceUnit *ru;
    for (HeightGridValue *i=mHeightGrid->begin(); i!=end; ++i) {
        cp = mHeightGrid->cellCenterPoint(mHeightGrid->indexOf(i));
        if (mRUmap.coordValid(cp)) {
            ru = mRUmap.valueAt(cp);
            if (ru) {
                ru->countStockedPixel( (*i).count()>0 );
            }
        }

    }
}

/** calculate for each resource unit the stockable area.
  "stockability" is determined by the isValid flag of resource units which in turn
  is derived from stand grid values.
  */
void Model::calculateStockableArea()
{

    mTotalStockableArea = 0.;
    foreach(ResourceUnit *ru, mRU) {
        // //
        //        if (ru->id()==-1) {
        //            ru->setStockableArea(0.);
        //            continue;
        //        }
        GridRunner<HeightGridValue> runner(*mHeightGrid, ru->boundingBox());
        int valid=0, total=0;
        while (runner.next()) {
            if ( runner.current()->isValid() )
                valid++;
            total++;
        }
        if (total) {
            ru->setStockableArea( cHeightPixelArea * valid); // in m2
            if (ru->snag())
                ru->snag()->scaleInitialState();
            mTotalStockableArea += cHeightPixelArea * valid / cRUArea; // in ha
            if (valid==0 && ru->id()>-1) {
                // invalidate this resource unit
                ru->setID(-1);
            }
            if (valid>0 && ru->id()==-1) {
                qDebug() << "Warning: a resource unit is marked as invalid (id=-1), but has stockable area (id was set to 0)!!! ru: " << ru->boundingBox() << "with index" << ru->index();
                ru->setID(0);
                // test-code
                //GridRunner<HeightGridValue> runner(*mHeightGrid, ru->boundingBox());
                //while (runner.next()) {
                //    qDebug() << mHeightGrid->cellCenterPoint(mHeightGrid->indexOf( runner.current() )) << ": " << runner.current()->isValid();
                //}

            }
        } else
            throw IException("calculateStockableArea: resource unit without pixels!");

    }
    // mark those pixels that are at the edge of a "forest-out-of-area"
    GridRunner<HeightGridValue> runner(*mHeightGrid, mHeightGrid->metricRect());
    HeightGridValue* neighbors[8];
    while (runner.next()) {
        if (runner.current()->isForestOutside()) {
            // if the current pixel is a "radiating" border pixel,
            // then check the neighbors and set a flag if the pixel is a neighbor of a in-project-area pixel.
            runner.neighbors8(neighbors);
            for (int i=0;i<8;++i)
                if (neighbors[i] &&  neighbors[i]->isValid())
                    runner.current()->setIsRadiating();

        }
    }

    qDebug() << "Total stockable area of the landscape is" << mTotalStockableArea << "ha.";

}

void Model::initializeGrid()
{
    // fill the whole grid with a value of "1."
    mGrid->initialize(1.f);

    // apply special values for grid cells border regions where out-of-area cells
    // radiate into the main LIF grid.
    QPoint p;
    int ix_min, ix_max, iy_min, iy_max, ix_center, iy_center;
    const int px_offset = cPxPerHeight / 2; // for 5 px per height grid cell, the offset is 2
    const int max_radiate_distance = 7;
    const float step_width = 1.f / static_cast<float>(max_radiate_distance);
    int c_rad = 0;
    for (HeightGridValue *hgv=mHeightGrid->begin(); hgv!=mHeightGrid->end(); ++hgv) {
        if (hgv->isRadiating()) {
            p=mHeightGrid->indexOf(hgv);
            ix_min = p.x() * cPxPerHeight - max_radiate_distance + px_offset;
            ix_max = ix_min + 2*max_radiate_distance + 1;
            ix_center = ix_min + max_radiate_distance;
            iy_min = p.y() * cPxPerHeight - max_radiate_distance + px_offset;
            iy_max = iy_min + 2*max_radiate_distance + 1;
            iy_center = iy_min + max_radiate_distance;
            for (int y=iy_min; y<=iy_max; ++y) {
                for (int x=ix_min; x<=ix_max; ++x) {
                    if (!mGrid->isIndexValid(x,y) ||  !(*mHeightGrid)(x/cPxPerHeight, y/cPxPerHeight).isValid())
                        continue;
                    float value = qMax(qAbs(x-ix_center), qAbs(y-iy_center)) * step_width;
                    float &v = mGrid->valueAtIndex(x, y);
                    if (value>=0.f && v>value)
                        v = value;
                }
            }
            c_rad++;
        }
    }
    if (logLevelDebug())
        qDebug() << "initialize grid:" << c_rad << "radiating pixels...";

}


/// Force the creation of stand statistics.
/// - stocked area (for resourceunit-areas)
/// - ru - statistics
void Model::createStandStatistics()
{
    calculateStockedArea();
    foreach(ResourceUnit *ru, mRU) {
        ru->addTreeAgingForAllTrees();
        ru->createStandStatistics();
    }
}

void Model::cleanTreeLists(bool recalculate_stats)
{
    foreach(ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        if (ru->hasDiedTrees()) {
            ru->cleanTreeList();
            ru->recreateStandStatistics(recalculate_stats);
        }
    }
}

