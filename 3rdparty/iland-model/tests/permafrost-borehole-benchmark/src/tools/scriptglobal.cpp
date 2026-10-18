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
// for redirecting the script output
#ifdef ILAND_GUI
#include <QTextEdit>
#endif
#include <QJSValue>
#include "global.h"
#include "scriptglobal.h"
#include "model.h"
#include "resourceunit.h"
#include "globalsettings.h"
#include "helper.h"
#include "debugtimer.h"
#include "standloader.h"
#include "mapgrid.h"
#include "outputmanager.h"
#include "modelcontroller.h"
#include "grid.h"
#include "snapshot.h"
#include "speciesset.h"
#include "species.h"
#include "seeddispersal.h"
#include "scriptgrid.h"
#include "expressionwrapper.h"
#include "watercycle.h"

// for accessing script publishing functions
#include "climateconverter.h"
#include "csvfile.h"
#include "spatialanalysis.h"
#include "dbhdistribution.h"
#include "spatialanalysis.h"
#include "scripttree.h"
#include "scriptresourceunit.h"

#ifdef ILAND_GUI
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "colors.h"
#endif

class ResourceUnit;

/** @class ScriptGlobal
  @ingroup scripts
   This is a global interface providing useful functionality for javascripts.
  Within javascript-code an instance of this class can be accessed as "Globals" in the global scope
 (no instantiation necessary).*/

/** \page globals Globals documentation
  Here are objects visible in the global space of javascript.
  \section sec An example section
  This page contains the subsections \ref subsection1 and \ref subsection2.
  For more info see page \ref page2.
  \subsection subsection1 The first subsection
  Text.
  \subsection subsection2 The second subsection
 - year integer. Current simulation year
 - currentDir current working directory. default value is the "script" directory defined in the project file.
  More text.
*/

QObject *ScriptGlobal::scriptOutput = nullptr;

ScriptGlobal::ScriptGlobal(QObject *)
{
    mModel = GlobalSettings::instance()->model();
    // current directory
    if (mModel)
        mCurrentDir = GlobalSettings::instance()->path(QString(), "script") + QDir::separator();

    mSRU = new ScriptResourceUnit();
    mRUValue = GlobalSettings::instance()->scriptEngine()->newQObject(mSRU);

}


QVariant ScriptGlobal::setting(QString key)
{
    const XmlHelper &xml = GlobalSettings::instance()->settings();
    if (!xml.hasNode(key)) {
        qDebug() << "scriptglobal: setting key " << key << "not valid.";
        return QVariant(); // undefined???
    }
    return QVariant(xml.value(key));
}
void ScriptGlobal::set(QString key, QString value)
{
    XmlHelper &xml = const_cast<XmlHelper&>(GlobalSettings::instance()->settings());
    if (!xml.hasNode(key)) {
        qDebug() << "scriptglobal: setting key " << key << "not valid.";
        return;
    }
    xml.setNodeValue(key, value);
}



void ScriptGlobal::print(QString message)
{
    qDebug() << message;
#ifdef ILAND_GUI
    if (ScriptGlobal::scriptOutput) {
        QTextEdit *e = qobject_cast<QTextEdit*>(ScriptGlobal::scriptOutput);
        if (e)
            e->append(message);
    }

#endif

}

void ScriptGlobal::alert(QString message)
{
    Helper::msg(message); // nothing happens when not in GUI mode

}


void ScriptGlobal::include(QString filename)
{
    QString path = GlobalSettings::instance()->path(filename);
    if (!QFile::exists(path))
        throw IException(QString("include(): The javascript source file '%1' could not be found.").arg(path));

    QString includeFile=Helper::loadTextFile(path);

    QJSValue ret = GlobalSettings::instance()->scriptEngine()->evaluate(includeFile, path);
    if (ret.isError()) {
        QString error_message = formattedErrorMessage(ret, includeFile);
        qDebug() << error_message;
        throw IException("Error in javascript-include():" + error_message);
    }

}

QString ScriptGlobal::defaultDirectory(QString dir)
{
    QString result = GlobalSettings::instance()->path(QString(), dir) + QDir::separator();
    return result;
}

QString ScriptGlobal::path(QString filename)
{
    return GlobalSettings::instance()->path(filename);
}

int ScriptGlobal::year() const
{
    return GlobalSettings::instance()->currentYear();
}
int ScriptGlobal::resourceUnitCount() const
{
    Q_ASSERT(mModel!=0);
    if (!mModel) return 0;
    return mModel->ruList().count();
}

double ScriptGlobal::worldX()
{
    return GlobalSettings::instance()->model()->extent().width();
}

double ScriptGlobal::worldY()
{
    return GlobalSettings::instance()->model()->extent().height();
}
// wrapped helper functions
QString ScriptGlobal::loadTextFile(QString fileName)
{
    return Helper::loadTextFile(GlobalSettings::instance()->path(fileName));
}
void ScriptGlobal::saveTextFile(QString fileName, QString content)
{
    Helper::saveToTextFile(fileName, content);
}
bool ScriptGlobal::fileExists(QString fileName)
{
    return QFile::exists(fileName);
}

void ScriptGlobal::systemCmd(QString command)
{
    qDebug() << "running system command:" << command;
    QProcess process;
    process.start(command);
    process.waitForFinished(); // will wait forever until finished

    QByteArray res_stdout = process.readAllStandardOutput();
    QByteArray res_stderr = process.readAllStandardError();
    qDebug() << "result (stdout):" << res_stdout;
    qDebug() << "result (stderr):" << res_stderr;
}

/// add trees on given resource unit
/// @param content init file in a string (containing headers)
/// @return number of trees added
int ScriptGlobal::addSingleTrees(const int resourceIndex, QString content)
{
    StandLoader loader(mModel);
    ResourceUnit *ru = mModel->ru(resourceIndex);
    if (!ru)
        throw IException(QString("addSingleTrees: invalid resource unit (index: %1").arg(resourceIndex));
    int cnt = loader.loadSingleTreeList(content, ru, "called_from_script");
    qDebug() << "script: addSingleTrees:" << cnt <<"trees loaded.";
    return cnt;
}

int ScriptGlobal::addTrees(const int resourceIndex, QString content)
{
    StandLoader loader(mModel);
    ResourceUnit *ru = mModel->ru(resourceIndex);
    if (!ru)
        throw IException(QString("addTrees: invalid resource unit (index: %1").arg(resourceIndex));
    return loader.loadDistributionList(content, ru, 0, "called_from_script");
}

int ScriptGlobal::addTreesOnMap(const int standID, QString content)
{
    StandLoader loader(mModel);
    return loader.loadDistributionList(content, nullptr, standID, "called_from_script");
}

/*
********** MapGrid wrapper
*/

//Q_SCRIPT_DECLARE_QMETAOBJECT(MapGridWrapper, QObject*)

void MapGridWrapper::addToScriptEngine(QJSEngine &engine)
{
    // about this kind of scripting magic see: http://qt.nokia.com/developer/faqs/faq.2007-06-25.9557303148
    //QJSValue cc_class = engine.scriptValueFromQMetaObject<MapGridWrapper>();
    // the script name for the object is "Map".
    // TODO: solution for creating objects!!!
    QObject *mgw = new MapGridWrapper();
    QJSValue mgw_cls = engine.newQObject(mgw);
    engine.globalObject().setProperty("Map", mgw_cls);
}

MapGridWrapper::MapGridWrapper(QObject *)
{
    mCreated = false;
    if (!GlobalSettings::instance()->model())
        return;
    mMap = const_cast<MapGrid*>(GlobalSettings::instance()->model()->standGrid());

}

MapGridWrapper::~MapGridWrapper()
{
    if (mCreated)
        delete mMap;
}


void MapGridWrapper::load(QString file_name)
{
    if (mCreated)
        delete mMap;
    mMap = new MapGrid(file_name);
    mCreated = true;
}

bool MapGridWrapper::isValid() const
{
    return mMap->isValid();
}

void MapGridWrapper::saveAsImage(QString)
{
    qDebug() << "not implemented";
}

void MapGridWrapper::paint(double min_value, double max_value)
{
    //gridToImage(mMap->grid(), false, min_value, max_value).save(file_name);
    if (mMap) {
        if (GlobalSettings::instance()->controller())
            GlobalSettings::instance()->controller()->paintMap(mMap, min_value, max_value);

    }

}

void MapGridWrapper::clear()
{
    if (!mCreated) {
        // create a empty map
        mMap = new MapGrid();
        mMap->createEmptyGrid();
        mCreated = true;
    }
    const_cast<Grid<int>& >(mMap->grid()).initialize(0); // clear all data and set to 0
}

void MapGridWrapper::clearProjectArea()
{
    if (!mCreated) {
        // create a empty map
        mMap = new MapGrid();
        mMap->createEmptyGrid();
        mCreated = true;
    }
    const MapGrid *stand_grid = GlobalSettings::instance()->model()->standGrid();
    if (!stand_grid) {
        qDebug() << "MapGridWrapper::clearProjectArea: no valid stand grid to copy from!";
        return;
    }
    for(int *src=stand_grid->grid().begin(), *dest=mMap->grid().begin(); src!=stand_grid->grid().end(); ++src, ++dest)
        *dest = *src<0? *src : 0;
}

void MapGridWrapper::createStand(int stand_id, QString paint_function, bool wrap_around)
{
    if (!mMap)
        throw IException("no valid map to paint on");
    Expression expr(paint_function);
    expr.setCatchExceptions(true);
    double *x_var = expr.addVar("x");
    double *y_var = expr.addVar("y");
    if (!wrap_around) {
        // now loop over all cells ...
        for (int *p = mMap->grid().begin(); p!=mMap->grid().end(); ++p) {
            QPoint pt = mMap->grid().indexOf(p);
            QPointF ptf = mMap->grid().cellCenterPoint(pt);
            // set the variable values and evaluate the expression
            *x_var = ptf.x();
            *y_var = ptf.y();
            if (expr.execute()) {
                *p = stand_id;
            }
        }
    } else {
        // WRAP AROUND MODE
        // now loop over all cells ...
        double delta_x = GlobalSettings::instance()->model()->extent().width();
        double delta_y = GlobalSettings::instance()->model()->extent().height();

        for (int *p = mMap->grid().begin(); p!=mMap->grid().end(); ++p) {
            QPoint pt = mMap->grid().indexOf(p);
            QPointF ptf = mMap->grid().cellCenterPoint(pt);
            if (ptf.x()<0. || ptf.x()>delta_x || ptf.y()<0. || ptf.y()>delta_y)
                continue;
            // set the variable values and evaluate the expression
            // we have to look at *9* positions to cover all wrap around cases....
            for (int dx=-1;dx<2;++dx) {
                for (int dy=-1;dy<2;++dy) {
                    *x_var = ptf.x() + dx*delta_x;
                    *y_var = ptf.y() + dy*delta_y;
                    if (expr.execute())
                        *p = stand_id;
                }
            }
        }
    }
    // after changing the map, recreate the index
    mMap->createIndex();
}

double MapGridWrapper::copyPolygonFromRect(QJSValue source, int id_in, int id, double destx, double desty, double x1, double y1, double x2, double y2)
{
    Grid<double> *dsrc=0;
    const Grid<int> *isrc=0;
    QObject *o = source.toQObject();
    if (o && qobject_cast<ScriptGrid*>(o)) {
        dsrc = qobject_cast<ScriptGrid*>(o)->grid();
    } else  if (o && qobject_cast<MapGridWrapper*>(o)) {
        isrc = & (qobject_cast<MapGridWrapper*>(o)->map()->grid());
    } else {
        qDebug() << "MapGridWrapper: copyPolygonFromRect: invalid source (neither Grid, nor MapGrid)!";
        return 0.;
    }

    Grid<int> &dest = const_cast<Grid<int> &>( mMap->grid() );
    QRect r = dest.rectangle().intersected(QRect(dest.indexAt(QPointF(destx, desty)),dest.indexAt(QPointF(destx+(x2-x1),desty+(y2-y1)))) );
    QPoint dest_coord = dest.indexAt(QPointF(destx, desty));
    QPoint offset = dest.indexAt(QPointF(x1,y1)) - dest_coord;
    qDebug() << "Rectangle" << r << "offset" << offset << "from" << QPointF(x1,y1) << "to" << QPointF(destx, desty);
    if (r.isNull())
        return 0.;

    GridRunner<int> gr(dest, r);

    int i=0, j=0;
    if (isrc) {
        while (gr.next()) {
            QPoint dp=gr.currentIndex()+offset;
            i++;
            // with another map grid, the size is identical
            if (isrc->isIndexValid(dp) && isrc->constValueAtIndex(dp)==id_in && *gr.current()>=0) {
                *gr.current() = id;
                //if (j<100) qDebug() << dp << gr.currentIndex() << src.constValueAtIndex(dp) << *gr.current();
                ++j;
            }
        }
    } else {
        double dbl_id = static_cast<double>(id_in);
        QPointF cc, tc;
        QPointF delta(x1-destx,y1-desty); // the offset of the source rectangle of the first point of the target rectangle (m)
        while (gr.next()) {
            cc = gr.currentCoord(); // coords on the destination map (m)
            tc = cc + delta;
            i++;
            // use the coordinates in the case of a general grid
            if (dsrc->coordValid(tc) && dsrc->valueAt(tc)==dbl_id && *gr.current()>=0) {
                *gr.current() = id;
                ++j;
            }
        }

    }
    //qDebug() << "copyPolygonFromRect: copied" << j << "from" << i;

    // after changing the map, recreate the index
    // mMap->createIndex();

    return double(j)/100.; // in ha

}

void MapGridWrapper::createMapIndex()
{
    if (mMap)
        mMap->createIndex();
}

QString MapGridWrapper::name() const
{
    if (mMap)
        return mMap->name();
    else
        return "invalid";
}
double MapGridWrapper::area(int id) {
    if (mMap && mMap->isValid())
        return mMap->area(id);
    else
        return -1;
}

bool ScriptGlobal::startOutput(QString table_name)
{
    if (table_name == "debug_dynamic") {
        GlobalSettings::instance()->controller()->setDynamicOutputEnabled(true);
        qDebug() << "started dynamic debug output";
        return true;
    }
    if (table_name.startsWith("debug_")) {
        GlobalSettings::DebugOutputs dbg = GlobalSettings::instance()->debugOutputId(table_name.mid(6));
        if (dbg==0)
            qDebug() << "cannot start debug output" << table_name << "because this is not a valid name.";
        GlobalSettings::instance()->setDebugOutput(dbg, true);
        return true;
    }
    OutputManager *om = GlobalSettings::instance()->outputManager();
    if (!om) return false;
    Output *out = om->find(table_name);
    if (!out) {
        QString err=QString("startOutput: Output '%1' is not a valid output.").arg(table_name);
        // TODO: ERROR function in script
//        if (context())
//           context()->throwError(err);
        qWarning() << err;
        return false;
    }
    try {
    out->setEnabled(true);
    } catch(const IException &e) { qWarning() << e.message(); }

    qDebug() << "started output" << table_name;
    return true;
}

bool ScriptGlobal::stopOutput(QString table_name)
{
    if (table_name == "debug_dynamic") {
        GlobalSettings::instance()->controller()->setDynamicOutputEnabled(false);
        qDebug() << "stopped dynamic debug output.";
        return true;
    }
    if (table_name.startsWith("debug_")) {
        GlobalSettings::DebugOutputs dbg = GlobalSettings::instance()->debugOutputId(table_name.mid(6));
        if (dbg==0)
            qDebug() << "cannot stop debug output" << table_name << "because this is not a valid name.";
        GlobalSettings::instance()->setDebugOutput(dbg, false);
        return true;
    }
    OutputManager *om = GlobalSettings::instance()->outputManager();
    if (!om) return false;
    Output *out = om->find(table_name);
    if (!out) {
        QString err=QString("stopOutput: Output '%1' is not a valid output.").arg(table_name);
        qWarning() << err;
        // TODO: ERROR function in script
//        if (context())
//           context()->throwError(err);
        return false;
    }
    out->setEnabled(false);
    qDebug() << "stopped output" << table_name;
    return true;
}

void ScriptGlobal::debugOutputFilter(QList<int> ru_indices)
{
    // ru_indices.contains()
    if (!GlobalSettings::instance()->model())
        return;

    const Grid<ResourceUnit*> &g = GlobalSettings::instance()->model()->RUgrid();
    int n_enabled = 0;
    for (ResourceUnit **ru = g.begin(); ru!=g.end(); ++ru)
        if (*ru) {
            bool do_dbg = ru_indices.isEmpty() || ru_indices.contains( (*ru)->index() );
            (*ru)->setCreateDebugOutput(do_dbg);
            n_enabled += do_dbg;
        }
    qDebug() << "debugOutputFilter: debug output enabled for" << n_enabled << "RUs";
}

bool ScriptGlobal::saveDebugOutputs(bool do_clear)
{
    GlobalSettings::instance()->controller()->saveDebugOutputJs(do_clear);
    qDebug() << "saved debug outputs.... clear:" << do_clear;
    return true;
}

bool ScriptGlobal::screenshot(QString file_name)
{
    if (GlobalSettings::instance()->controller())
        GlobalSettings::instance()->controller()->saveScreenshot(file_name);
    return true;
}

void ScriptGlobal::repaint()
{
    if (GlobalSettings::instance()->controller())
        GlobalSettings::instance()->controller()->repaint();
}

void ScriptGlobal::setViewport(double x, double y, double scale_px_per_m)
{
    if (GlobalSettings::instance()->controller())
        GlobalSettings::instance()->controller()->setViewport(QPointF(x,y), scale_px_per_m);
}

// helper function...
QString heightGrid_height(const HeightGridValue &hgv) {
    return QString::number(hgv.height);
}

/// write grid to a file...
bool ScriptGlobal::gridToFile(QString grid_type, QString file_name)
{
    if (!GlobalSettings::instance()->model())
        return false;
    QString result;
    if (grid_type == "height")
        result = gridToESRIRaster(*GlobalSettings::instance()->model()->heightGrid(), *heightGrid_height);
    if (grid_type == "lif")
        result = gridToESRIRaster(*GlobalSettings::instance()->model()->grid());

    if (!result.isEmpty()) {
        file_name = GlobalSettings::instance()->path(file_name);
        Helper::saveToTextFile(file_name, result);
        qDebug() << "saved grid to " << file_name;
        return true;
    }
    qDebug() << "could not save gridToFile because" << grid_type << "is not a valid grid.";
    return false;

}

QJSValue ScriptGlobal::grid(QString type)
{
    int index = -1;
    // height grid options
    if (type=="height") index = 0;
    if (type=="valid") index = 1;
    if (type=="count") index = 2;
    if (type=="forestoutside") index=3;
    if (type=="standgrid") index=4;
    // resource unit level
    if (type=="smallsaplingcover") index=10;
    if (type=="saplingcover") index=11;
    if (type=="swc") index=12;
    if (type=="swc_gs") index=13;
    if (type=="swc_pot") index=14;
    if (index<0) {
        qDebug()<< "ScriptGlobal::grid(): error: invalid grid specified:" << type << ". valid options: 'height', 'valid', 'count', 'forestoutside'.";
        return QJSValue();
    }

    if (index == 4) {
        // stand grid
        const MapGrid *sg = GlobalSettings::instance()->model()->standGrid();
        if (!sg || !sg->isValid())
            throw IException("Error in Globals.grid(): a valid stand grid is not available!");
        Grid<double> *dgrid = new Grid<double>(sg->grid().metricRect(), sg->grid().cellsize());
        double *p=dgrid->begin();
        for (int *s=sg->grid().begin(); s!=sg->grid().end(); ++s,++p)
            *p = *s;
        QJSValue g = ScriptGrid::createGrid(dgrid, type);
        return g;

    }
    if (index < 10) {
        HeightGrid *h = GlobalSettings::instance()->model()->heightGrid();
        //Grid<double> *dgrid = new Grid<double>(h->cellsize(), h->sizeX(), h->sizeY());
        Grid<double> *dgrid = new Grid<double>(h->metricRect(), h->cellsize());
        // fetch data from height grid
        double *p=dgrid->begin();
        for (HeightGridValue *hgv=h->begin(); hgv!=h->end(); ++hgv, ++p) {
            switch (index) {
            case 0: *p = hgv->height; break;
            case 1: *p = hgv->isValid()?1. : 0.; break;
            case 2: *p = hgv->count(); break;
            case 3: *p = hgv->isForestOutside()?1. : 0.; break;
            }
        }

        QJSValue g = ScriptGrid::createGrid(dgrid, type);
        return g;
    } else {
        // resource unit level
        const Grid<ResourceUnit*> &rg = GlobalSettings::instance()->model()->RUgrid();
        Grid<double> *dgrid=new Grid<double>(rg.cellsize(), rg.sizeX(), rg.sizeY());
        double *p = dgrid->begin();
        for (ResourceUnit **ru = rg.begin(); ru!=rg.end(); ++ru, ++p) {
            switch( index ) {
            case 10: *p = *ru ? (*ru)->saplingCoveredArea(true) / cRUArea : 0.; break; // either grass or saplings <1.3m
            case 11: *p = *ru ? (*ru)->saplingCoveredArea(false) / cRUArea : 0.; break; // saplings > 1.3m
            case 12: *p = *ru && (*ru)->waterCycle() ? (*ru)->waterCycle()->meanSoilWaterContent() : 0.; break;
            case 13: *p = *ru && (*ru)->waterCycle() ? (*ru)->waterCycle()->meanGrowingSeasonSWC() : 0.; break;
            case 14: *p = *ru && (*ru)->waterCycle() ? (*ru)->waterCycle()->fieldCapacity() : 0.; break;
            }
        }
        QJSValue g = ScriptGrid::createGrid(dgrid, type);
        return g;
    }

}

QJSValue ScriptGlobal::speciesShareGrid(QString species)
{
    Species *s = GlobalSettings::instance()->model()->speciesSet()->species(species);
    if (!s) {
        qDebug() << "speciesShareGrid: invalid species" << species;
        return QJSValue();
    }
    const Grid<ResourceUnit*> &rug = GlobalSettings::instance()->model()->RUgrid();
    Grid<double> *grid = new Grid<double>(rug.cellsize(), rug.sizeX(), rug.sizeY());
    double *p=grid->begin();
    for (ResourceUnit **ru=rug.begin(); ru!=rug.end(); ++ru, ++p) {
        if (*ru && (*ru)->constResourceUnitSpecies(s))
            *p = (*ru)->resourceUnitSpecies(s).statistics().basalArea();
        else
            *p=0.;
    }
    QJSValue g = ScriptGrid::createGrid(grid, species);
    return g;
}

QJSValue ScriptGlobal::resourceUnitGrid(QString expression)
{
    const Grid<ResourceUnit*> &rug = GlobalSettings::instance()->model()->RUgrid();
    Grid<double> *grid = new Grid<double>(rug.cellsize(), rug.sizeX(), rug.sizeY());
    double *p=grid->begin();
    RUWrapper ru_wrap;
    Expression ru_value(expression, &ru_wrap);

    for (ResourceUnit **ru=rug.begin(); ru!=rug.end(); ++ru, ++p) {
        if (*ru) {
            ru_wrap.setResourceUnit(*ru);
            double value = ru_value.execute();
            *p = value;
        } else {
            *p=0.;
        }
    }
    QJSValue g = ScriptGrid::createGrid(grid, "ru");
    return g;

}

QJSValue ScriptGlobal::resourceUnit(int index)
{
    if (!mSRU)
        return QJSValue();
    mSRU->clear();
    if (!GlobalSettings::instance()->model())
        return mRUValue;

    mSRU->setRU(GlobalSettings::instance()->model()->ru(index));
    return mRUValue;

}


bool ScriptGlobal::seedMapToFile(QString species, QString file_name)
{
    // does not fully work:
    // Problem: after a full year cycle the seed maps are already cleared and prepared for the next round
    // --> this is now more an "occurence" map

    if (!GlobalSettings::instance()->model())
        return false;
    // find species
    Species *s = GlobalSettings::instance()->model()->speciesSet()->species(species);
    if (!s) {
        qDebug() << "invalid species" << species << ". No seed map saved.";
        return false;
    }
    s->seedDispersal()->dumpMapNextYear(file_name);
    qDebug() << "creating raster in the next year cycle for species" << s->id();
    return true;

    //gridToImage( s->seedDispersal()->seedMap(), true, 0., 1.).save(GlobalSettings::instance()->path(file_name));
//    QString result = gridToESRIRaster(s->seedDispersal()->seedMap());
//    if (!result.isEmpty()) {
//        file_name = GlobalSettings::instance()->path(file_name);
//        Helper::saveToTextFile(file_name, result);
//        qDebug() << "saved grid to " << file_name;
//        return true;
//    }
//    qDebug() << "failed creating seed map";
//    return false;
}

void ScriptGlobal::wait(int milliseconds)
{
    // http://stackoverflow.com/questions/1950160/what-can-i-use-to-replace-sleep-and-usleep-in-my-qt-app
    QMutex dummy;
    dummy.lock();
    QWaitCondition waitCondition;
    waitCondition.wait(&dummy, static_cast<unsigned long>(milliseconds));
    dummy.unlock();
}

int ScriptGlobal::addSaplingsOnMap(MapGridWrapper *map, const int mapID, QString species, int px_per_hectare, double height, int age)
{
    QString csv_file = QString("species;count;height;age\n%1;%2;%3;%4").arg(species).arg(px_per_hectare).arg(height).arg(age);
    StandLoader loader(mModel);
    try {
        loader.setMap(map->map());
        return loader.loadSaplings(csv_file, mapID, "called from script");
    } catch (const IException &e) {
        throwError(e.message());
    }
    return 0;
}

int ScriptGlobal::addSaplings(int standId, double x, double y, double width, double height, QString species, double treeheight, int age)
{
    QRectF remove_rect(x, y, width, height);
    if (standId>0) {
        if (!GlobalSettings::instance()->model()->standGrid()) {
            throw IException("addSaplings - no stand grid available!");
        }

        QRectF box = GlobalSettings::instance()->model()->standGrid()->boundingBox(standId);
        remove_rect.adjust(box.left(), box.top(), box.left(), box.top());

    }
    if (GlobalSettings::instance()->model()->saplings()) {
        return GlobalSettings::instance()->model()->saplings()->addSaplings(remove_rect, species, treeheight, age);
    }
    return -1;

}

void ScriptGlobal::removeSaplings(int standId, double x, double y, double width, double height)
{
    QRectF remove_rect(x, y, width, height);
    if (standId>0) {
        if (!GlobalSettings::instance()->model()->standGrid()) {
            throw IException("addSaplings - no stand grid available!");
        }

        QRectF box = GlobalSettings::instance()->model()->standGrid()->boundingBox(standId);
        //remove_rect.moveTo(box.topLeft());
        remove_rect.adjust(box.left(), box.top(), box.left(), box.top());
    }
    if (GlobalSettings::instance()->model()->saplings()) {
        GlobalSettings::instance()->model()->saplings()->clearSaplings(remove_rect, false, true);
    }
}

/// saves a snapshot of the current model state (trees, soil, etc.)
/// to a dedicated SQLite database.
bool ScriptGlobal::saveModelSnapshot(QString file_name)
{
    try {
        Snapshot shot;
        QString output_db = GlobalSettings::instance()->path(file_name);
        return shot.createSnapshot(output_db);
    } catch (const IException &e) {
        throwError(e.message());
    }
    return false;
}

/// loads a snapshot of the current model state (trees, soil, etc.)
/// from a dedicated SQLite database.
bool ScriptGlobal::loadModelSnapshot(QString file_name)
{
    try {
        Snapshot shot;
        QString input_db = GlobalSettings::instance()->path(file_name);
        return shot.loadSnapshot(input_db);
    } catch (const IException &e) {
        throwError(e.message());
    }
    return false;
}

bool ScriptGlobal::saveStandSnapshot(int stand_id, QString file_name)
{
    try {
    Snapshot shot;
    const MapGrid *map_grid = GlobalSettings::instance()->model()->standGrid();
    if (!map_grid)
        return false;
    return shot.saveStandSnapshot(stand_id, map_grid, GlobalSettings::instance()->path(file_name));
    } catch (const IException &e) {
        throwError(e.message());
    }
    return false;
}

bool ScriptGlobal::loadStandSnapshot(int stand_id, QString file_name)
{
    try {
    Snapshot shot;
    const MapGrid *map_grid = GlobalSettings::instance()->model()->standGrid();
    if (!map_grid)
        return false;
    return shot.loadStandSnapshot(stand_id, map_grid, GlobalSettings::instance()->path(file_name));
    } catch (const IException &e) {
        throwError(e.message());
    }
    return false;
}

bool ScriptGlobal::saveStandCarbon(int stand_id, QList<int> ru_ids, bool rid_mode)
{
    try {
        Snapshot shot;
        return shot.saveStandCarbon(stand_id, ru_ids, rid_mode);
    } catch (const IException &e) {
        throwError(e.message());
    }
    return false;
}

bool ScriptGlobal::loadStandCarbon()
{
    try {
        Snapshot shot;
        return shot.loadStandCarbon();
    } catch (const IException &e) {
        throwError(e.message());
    }
    return false;

}

void ScriptGlobal::reloadABE()
{
    qDebug() << "attempting to reload ABE";
    GlobalSettings::instance()->model()->reloadABE();
}

void ScriptGlobal::setUIshortcuts(QJSValue shortcuts)
{
    if (!shortcuts.isObject()) {
        qDebug() << "setUIShortcuts: expected a JS-object (name: javascript-call, value: description). Got: " << shortcuts.toString();
    }
    QVariantMap vm = shortcuts.toVariant().toMap();
    GlobalSettings::instance()->controller()->setUIShortcuts(vm);
}

void ScriptGlobal::test_tree_mortality(double thresh, int years, double p_death)
{
#ifdef ALT_TREE_MORTALITY
    Tree::mortalityParams(thresh, years, p_death );
#else
    qDebug() << "test_tree_mortality() not enabled!!";
    Q_UNUSED(thresh) Q_UNUSED(years) Q_UNUSED(p_death)
#endif

}


void ScriptGlobal::throwError(const QString &errormessage)
{
    GlobalSettings::instance()->scriptEngine()->evaluate(QString("throw '%1'").arg(errormessage));
    qWarning() << "Scripterror:" << errormessage;
    // TODO: check if this works....
}

void ScriptGlobal::loadScript(const QString &fileName)
{
    QJSEngine *engine = GlobalSettings::instance()->scriptEngine();

    QString program = Helper::loadTextFile(fileName);
    if (program.isEmpty()) {
        qDebug() << "loading of Javascript file" << fileName << "failed because file is either missing or empty.";
        return;
    }

    QJSValue result = engine->evaluate(program);
    qDebug() << "javascript file loaded" << fileName;
    if (result.isError()) {
        int lineno = result.property("lineNumber").toInt();
        QStringList code_lines = program.replace('\r', "").split('\n'); // remove CR, split by LF
        QString code_part;
        for (int i=std::max(0, lineno - 5); i<std::min(lineno+5, code_lines.count()); ++i)
            code_part.append(QString("%1: %2 %3\n").arg(i).arg(code_lines[i]).arg(i==lineno?"  <---- [ERROR]":""));
        qDebug() << "Javascript Error in file" << fileName << ":" << result.property("lineNumber").toInt() << ":" << result.toString() << ":\n" << code_part;

    }

}

QString ScriptGlobal::executeScript(QString cmd)
{
    DebugTimer t("execute javascript");
    QJSEngine *engine = GlobalSettings::instance()->scriptEngine();
    QJSValue result;
    if (engine)
        result = engine->evaluate(cmd);
    if (result.isError()) {
        //int line = mEngine->uncaughtExceptionLineNumber();
        QString msg = QString( "Script Error occured: %1\n").arg( result.toString() );
        qDebug() << msg;
        //msg+=engine->uncaughtExceptionBacktrace().join("\n");
        return msg;
    } else {
        return QString();
    }
}

QString ScriptGlobal::executeJSFunction(QString function)
{
    DebugTimer t("execute javascript");
    QJSEngine *engine = GlobalSettings::instance()->scriptEngine();
    QJSValue result;
    if (!engine)
        return QStringLiteral("No valid javascript engine!");

    if (engine->globalObject().property(function).isCallable()) {
        result = engine->globalObject().property(function).call();
        if (result.isError()) {
            QString msg = QString( "Script Error occured: %1\n").arg( result.toString() );
            qDebug() << msg;
            return msg;
        }
    }
    return QString();

}

QString ScriptGlobal::formattedErrorMessage(const QJSValue &error_value, const QString &sourcecode)
{
    if (error_value.isError()) {
        int lineno = error_value.property("lineNumber").toInt();
        QString code = sourcecode;
        QStringList code_lines = code.replace('\r', "").split('\n'); // remove CR, split by LF
        QString code_part;
        for (int i=std::max(0, lineno - 5); i<std::min(lineno+5, code_lines.count()); ++i)
            code_part.append(QString("%1: %2 %3\n").arg(i).arg(code_lines[i]).arg(i==lineno?"  <---- [ERROR]":""));
        QString error_string = QString("Javascript Error in file '%1:%2':%3\n%4")
                .arg(error_value.property("fileName").toString())
                .arg(error_value.property("lineNumber").toInt())
                .arg(error_value.toString())
                .arg(code_part);
        return error_string;
    }
    return QString();
}

QJSValue ScriptGlobal::viewOptions()
{
    QJSValue res;
#ifdef ILAND_GUI
    MainWindow *mw = GlobalSettings::instance()->controller()->mainWindow();
    Ui::MainWindowClass *ui = mw->uiclass();
    // TODO: fix??

#endif
    return res;
}

void ScriptGlobal::setViewOptions(QJSValue opts)
{
#ifdef ILAND_GUI
    MainWindow *mw = GlobalSettings::instance()->controller()->mainWindow();
    Ui::MainWindowClass *ui = mw->uiclass();
    // ruler options
    if (opts.hasProperty("maxValue") || opts.hasProperty("minValue")) {
        mw->ruler()->setMaxValue(opts.property("maxValue").toNumber());
        mw->ruler()->setMinValue(opts.property("minValue").toNumber());
        mw->ruler()->setAutoScale(false);
    } else {
        mw->ruler()->setAutoScale(true);
    }

    // main visualization options
    QString type = opts.property("type").toString();
    if (type=="lif")
        ui->visFon->setChecked(true);
    if (type=="dom")
        ui->visDomGrid->setChecked(true);
    if (type=="regeneration")
        ui->visRegeneration->setChecked(true);
    if (type=="trees") {
        ui->visImpact->setChecked(true);
        ui->visSpeciesColor->setChecked(false);
    }
    if (type=="ru") {
        ui->visResourceUnits->setChecked(true);
        ui->visRUSpeciesColor->setChecked(false);
    }
    if (type=="seed") {
        ui->visSeeds->setChecked(true);
    }

    // further options
    if (opts.hasProperty("clip"))
        ui->visClipStandGrid->setChecked(opts.property("clip").toBool());

    if (opts.hasProperty("transparent"))
        ui->drawTransparent->setChecked(opts.property("transparent").toBool());


    // color by a species ID
    if (opts.hasProperty("species") && opts.property("species").isBool() && type=="trees") {
        ui->visSpeciesColor->setChecked(opts.property("species").toBool());
        ui->speciesFilterBox->setCurrentIndex(0); // all species
    }

    if (opts.hasProperty("species") && opts.property("species").isString()) {
        QString species=opts.property("species").toString();
        if (type=="ru")
            ui->visRUSpeciesColor->setChecked(true);
        else if (type=="trees")
            ui->visSpeciesColor->setChecked(true);

        int idx = ui->speciesFilterBox->findData(species);
        ui->speciesFilterBox->setCurrentIndex(idx);
    }
    if (opts.hasProperty("autoscale"))
        ui->visAutoScale->setChecked(opts.property("autoscale").toBool());

    if (opts.hasProperty("shade"))
        ui->visAutoScale->setChecked(opts.property("shade").toBool());

    // draw a specific grid
    if (opts.property("grid").isString()) {
        QString grid=opts.property("grid").toString();
        mw->setPaintGrid(grid);
        ui->visOtherGrid->setChecked(true);
        //throw IException("todo: fix with the new grid...");
        //ui->otherGridTree->find()
        //int idx = ui->paintGridBox->findData(grid);
        //ui->paintGridBox->setCurrentIndex(idx);
    }

    if (opts.hasProperty("expression"))
        ui->lTreeExpr->setText(opts.property("expression").toString());

    if (opts.hasProperty("filter")) {
        ui->expressionFilter->setText(opts.property("filter").toString());
        ui->cbDrawFiltered->setChecked(!ui->expressionFilter->text().isEmpty());
    }


#else
    Q_UNUSED(opts)
    qDebug() << "Globals.setViewOptions() not supported in ilandc";

#endif

}

QJSValue ScriptGlobal::valueFromJs(const QJSValue &js_value, const QString &key, const QString default_value, const QString &errorMessage)
{
    if (!js_value.hasOwnProperty(key)) {
        if (!errorMessage.isEmpty())
            throw IException(QString("Error: required key '%1' not found. In: %2 (JS: %3)").arg(key).arg(errorMessage).arg(JStoString(js_value)));
        else if (default_value.isEmpty())
            return QJSValue();
        else {
            // return a numeric or string as default value
            bool ok;
            double default_numeric = default_value.toDouble(&ok);
            if (ok)
                return default_numeric;
            else
                return default_value;
        }
    }
    return js_value.property(key);

}

QString ScriptGlobal::JStoString(QJSValue value)
{
    if (value.isArray() || value.isObject()) {
        QJSValue fun = GlobalSettings::instance()->scriptEngine()->evaluate("(function(a) { return JSON.stringify(a); })");
        QJSValue result = fun.call(QJSValueList() << value);
        return result.toString();
    } else
        return value.toString();
}


void ScriptGlobal::setupGlobalScripting()
{
    QJSEngine *engine = GlobalSettings::instance()->scriptEngine();
//    QJSValue dbgprint = engine->newFunction(script_debug);
//    QJSValue sinclude = engine->newFunction(script_include);
//    QJSValue alert = engine->newFunction(script_alert);
//    engine->globalObject().setProperty("print",dbgprint);
//    engine->globalObject().setProperty("include",sinclude);
//    engine->globalObject().setProperty("alert", alert);

    // check if update necessary
    if (engine->globalObject().property("include").isCallable())
        return;

    // wrapper functions for (former) stand-alone javascript functions
    // Qt5 - modification
    engine->evaluate("function print(x) { Globals.print(x); } \n" \
                     "function include(x) { Globals.include(x); } \n" \
                     "function alert(x) { Globals.alert(x); } \n");
    // add a (fake) console.log / console.print
/*/    engine->evaluate("var console = { log: function(x) {Globals.print(x); }, " \
//                     "                print: function(x) { for(var propertyName in x)  " \
//                     "                                       console.log(propertyName + ': ' + x[propertyName]); " \
//                     "                                   } " \
//                     "              }"); */


    ScriptObjectFactory *factory = new ScriptObjectFactory;
    QJSValue obj = GlobalSettings::instance()->scriptEngine()->newQObject(factory);
    engine->globalObject().setProperty("Factory", obj);



    // other object types
    ClimateConverter::addToScriptEngine(*engine);
    CSVFile::addToScriptEngine(*engine);
    MapGridWrapper::addToScriptEngine(*engine);
    SpatialAnalysis::addToScriptEngine();
    ScriptTree::addToScriptEngine(*engine);
    ScriptTreeExpr::addToScriptEngine(*engine);
    ScriptGrid::addToScriptEngine(engine);

}

int ScriptGlobal::msec() const
{
    return QTime::currentTime().msecsSinceStartOfDay();
}

// Factory functions


ScriptObjectFactory::ScriptObjectFactory(QObject *parent):
    QObject(parent)
{
    mObjCreated = 0;
}

QJSValue ScriptObjectFactory::newCSVFile(QString filename)
{
    CSVFile *csv_file = new CSVFile;
    if (!filename.isEmpty()) {
        qDebug() << "CSVFile: loading file" << filename;
        csv_file->loadFile(filename);
    }

    QJSValue obj = GlobalSettings::instance()->scriptEngine()->newQObject(csv_file);
    mObjCreated++;
    return obj;
}

QJSValue ScriptObjectFactory::newClimateConverter()
{
    ClimateConverter *cc = new ClimateConverter(nullptr);
    QJSValue obj = GlobalSettings::instance()->scriptEngine()->newQObject(cc);
    mObjCreated++;
    return obj;

}


QJSValue ScriptObjectFactory::newMap()
{
    MapGridWrapper *map = new MapGridWrapper(nullptr);
    QJSValue obj = GlobalSettings::instance()->scriptEngine()->newQObject(map);
    mObjCreated++;
    return obj;

}

QJSValue ScriptObjectFactory::newDBHDistribution()
{
    DBHDistribution *dbh = new DBHDistribution();
    QJSValue obj = GlobalSettings::instance()->scriptEngine()->newQObject(dbh);
    mObjCreated++;
    return obj;
}

QJSValue ScriptObjectFactory::newGrid()
{
    QJSValue result = ScriptGrid::createGrid(0); // create with an empty grid
    return result;
}

QJSValue ScriptObjectFactory::newSpatialAnalysis()
{
    SpatialAnalysis *spati = new SpatialAnalysis;
    QJSValue v = GlobalSettings::instance()->scriptEngine()->newQObject(spati);
    mObjCreated++;
    return v;

}


