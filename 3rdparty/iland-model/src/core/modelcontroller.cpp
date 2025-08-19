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

/** ModelController is a helper class used to control the flow of operations during a model run.
  The ModelController encapsulates the Model class and is the main control unit. It is used by the
  iLand GUI as well as the command line version (ilandc).

  */

#include "modelcontroller.h"
#include <QObject>

#include "model.h"
#include "debugtimer.h"
#include "helper.h"
#include "version.h"
#include "expression.h"
#include "expressionwrapper.h"
#include "../output/outputmanager.h"

#include "species.h"
#include "speciesset.h"
#include "mapgrid.h"
#include "statdata.h"


#include "biteengine.h"

#ifdef ILAND_GUI
#include "mainwindow.h" // for the debug message buffering
#endif

ModelController::ModelController()
{
    mModel = NULL;
    mPaused = false;
    mRunning = false;
    mHasError = false;
    mIsStartingUp = false;
    mIsBusy = false;
    mYearsToRun = 0;
    mViewerWindow = 0;
    mDynamicOutputEnabled = false;
}

ModelController::~ModelController()
{
    destroy();
}

void ModelController::connectSignals()
{
    if (!mViewerWindow)
        return;
#ifdef ILAND_GUI
    connect(this,SIGNAL(bufferLogs(bool)), mViewerWindow, SLOT(bufferedLog(bool)));
#endif
}

/// prepare a list of all (active) species
QList<const Species*> ModelController::availableSpecies()
{
    QList<const Species*> list;
    if (mModel) {
        SpeciesSet *set = mModel->speciesSet();
        if (!set)
            throw IException("there are 0 or more than one species sets.");
        foreach (const Species *s, set->activeSpecies()) {
            list.append(s);
        }
    }
    return list;
}

bool ModelController::canCreate()
{
    if (mModel)
        return false;
    return true;
}

bool ModelController::canDestroy()
{
    return mModel != NULL;
}

bool ModelController::canRun()
{
    if (mModel && mModel->isSetup())
        return true;
    return false;
}

bool ModelController::isRunning()
{
    return mRunning;
}

bool ModelController::isFinished()
{
    if (!mModel)
        return false;
    return canRun() && !isRunning()  && mFinished;
}

bool ModelController::isPaused()
{
    return mPaused;
}

int ModelController::currentYear() const
{
    return GlobalSettings::instance()->currentYear();
}

QString ModelController::timeString() const
{
    int elapsed = mStartTime.msecsTo(QTime::currentTime());
    QString time_str = DebugTimer::timeStr(elapsed, false);
    double frac_done = totalYears()>0 ? currentYear() / double(totalYears()) : 0;
    QString todo_str="-";
    if (frac_done>0.){
        int todo = (1. / frac_done - 1.) * elapsed;
        todo_str = DebugTimer::timeStr(todo, false);
    }
    return QString("%1 (%2 remaining)").arg(time_str, todo_str);
}

bool ModelController::setFileName(QString initFileName)
{
    mInitFile = initFileName;
    try {
        GlobalSettings::instance()->loadProjectFile(mInitFile);
        return true;
    } catch(const IException &e) {
        QString error_msg = e.message();
        Helper::msg(error_msg);
        mHasError = true;
        mLastError = error_msg;
        qDebug() << error_msg;
    }
    return false;
}

void ModelController::create()
{
    if (!canCreate())
        return;
    emit bufferLogs(true);
    const_cast<XmlHelper&>(GlobalSettings::instance()->settings()).resetWarnings();
    mIsStartingUp = true;
    mIsBusy = true;
    qDebug() << "**************************************************";
    qDebug() << "project-file:" << mInitFile;
    qDebug() << "started at: " << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    qDebug() << "iLand " << currentVersion() << " (" << verboseVersion() << ")";
    qDebug() << "**************************************************";


    try {
        mHasError = false;
        DebugTimer::clearAllTimers();
        mModel = new Model();

        mModel->loadProject();
        if (!mModel->isSetup()) {
            mHasError = true;
            mIsStartingUp = false;
            mIsBusy = false;
            mLastError = "An error occured during the loading of the project. Please check the logs.";
            return;
        }

        // reset clock...
        GlobalSettings::instance()->setCurrentYear(1); // reset clock
        // initialization of trees, output on startup
        mModel->beforeRun();
        GlobalSettings::instance()->executeJSFunction("onAfterCreate");
    } catch(const IException &e) {
        QString error_msg = e.message();
        Helper::msg(error_msg);
        mLastError = error_msg;
        mHasError = true;
        mIsStartingUp = false;
        qDebug() << error_msg;
    }
    emit bufferLogs(false);

    const_cast<XmlHelper&>(GlobalSettings::instance()->settings()).printSuppressedWarnings();
    qDebug() << "Model created.";
    mIsStartingUp = false;
    mIsBusy = false;
}

void ModelController::destroy()
{
    if (canDestroy()) {
        GlobalSettings::instance()->executeJSFunction("onBeforeDestroy");
        Model *m = mModel;
        mModel = 0;
        delete m;
        GlobalSettings::instance()->setCurrentYear(0);
        qDebug() << "ModelController: Model destroyed.";
    }
}

void ModelController::runloop()
{
    static QTime sStartTime = QTime::currentTime();
#ifdef ILAND_GUI
 //   QApplication::processEvents();
#else
 //   QCoreApplication::processEvents();
#endif
    if (mPaused)
        return;
    bool doStop = false;
    mHasError = false;
    if (GlobalSettings::instance()->currentYear()<=1) {
        mStartTime = QTime::currentTime(); // reset clock at the beginning of the simulation
    }

    if (!mCanceled && GlobalSettings::instance()->currentYear() < mYearsToRun) {
        emit bufferLogs(true);

        mIsBusy = true;
        mHasError = runYear(); // do the work!
        mIsBusy = false;

        mRunning = true;
        emit year(GlobalSettings::instance()->currentYear());
        if (!mHasError) {
            int elapsed = sStartTime.msecsTo(QTime::currentTime());
            int time=0;
            if (currentYear()%50==0 && elapsed>10000)
                time = 100; // a 100ms pause...
            if (currentYear()%100==0 && elapsed>10000) {
                time = 500; // a 500ms pause...
            }
            if (time>0) {
                sStartTime = QTime::currentTime(); // reset clock
                qDebug() << "--- little break ---- (after " << elapsed << "ms).";
                //QTimer::singleShot(time,this, SLOT(runloop()));
            }

        } else {
           doStop = true; // an error occured
           mLastError = "An error occured while running the model. Please check the logs.";
           mHasError = true;
        }

    } else {
        doStop = true; // all years simulated
    }

    if (doStop || mCanceled) {
                // finished
        internalStop();
    }

#ifdef ILAND_GUI
    QApplication::processEvents();
#else
    QCoreApplication::processEvents();
#endif
}

bool ModelController::internalRun()
{
    // main loop
    try {
        while (mRunning && !mPaused &&  !mFinished) {
            runloop(); // start the running loop
        }
    } catch (IException &e) {
#ifdef ILAND_GUI
        Helper::msg(e.message());
#else
        qDebug() << e.message();
#endif

    }
    return isFinished();
}

void ModelController::internalStop()
{
    if (mRunning) {
        GlobalSettings::instance()->outputManager()->save();
        DebugTimer::printAllTimers();
        saveDebugOutputs(true);
        //if (GlobalSettings::instance()->dbout().isOpen())
        //    GlobalSettings::instance()->dbout().close();

        mFinished = true;
    }
    mRunning = false;
    mPaused = false; // in any case
    emit bufferLogs(false); // stop buffering
    emit finished(QString());
    emit stateChanged();

}

void ModelController::run(int years)
{
    if (!canRun())
        return;
    emit bufferLogs(true); // start buffering

    DebugTimer many_runs(QString("Timer for %1 runs").arg(years));
    mPaused = false;
    mFinished = false;
    mCanceled = false;
    mYearsToRun = years;
    //GlobalSettings::instance()->setCurrentYear(1); // reset clock

    DebugTimer::clearAllTimers();

    mRunning = true;
    emit stateChanged();

    qDebug() << "ModelControler: runloop started.";
    internalRun();
    emit stateChanged();
}

bool ModelController::runYear()
{
    if (!canRun()) return false;
    DebugTimer t("ModelController:runYear");
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss:") << "ModelController: run year" << currentYear();

    bool err=false;
    try {
        emit bufferLogs(true);
        mIsBusy = true;
        GlobalSettings::instance()->executeJSFunction("onYearBegin");
        mModel->runYear();
        mIsBusy = false;

        fetchDynamicOutput();
        saveDebugOutputs(false);
    } catch(const IException &e) {
        QString error_msg = e.message();
        Helper::msg(error_msg);
        qDebug() << error_msg;
        err=true;
    }
    emit bufferLogs(false);
#ifdef ILAND_GUI
    QApplication::processEvents();
#else
    QCoreApplication::processEvents();
#endif

    return err;
}

bool ModelController::pause()
{
    if(!isRunning())
        return mPaused;

    if (mPaused) {
        // currently in pause - mode -> continue
        mPaused = false;

    } else {
        // currently running -> set to pause mode
        GlobalSettings::instance()->outputManager()->save();
        mPaused = true;
        emit bufferLogs(false);
    }
    emit stateChanged();
    return mPaused;
}

bool ModelController::continueRun()
{
    mRunning = true;
    emit stateChanged();
    return internalRun();
}

void ModelController::cancel()
{
    mCanceled = true;
    internalStop();
    emit stateChanged();
}


// this function is called when exceptions occur in multithreaded code.
QMutex error_mutex;
void ModelController::throwError(const QString msg)
{
    QMutexLocker lock(&error_mutex); // serialize access
    qDebug() << "ModelController: throwError reached:";
    qDebug() << msg;
    mLastError = msg;
    mHasError = true;
    emit bufferLogs(false);
    emit bufferLogs(true); // start buffering again

    emit finished(msg);
    throw IException(msg); // raise error again

}
//////////////////////////////////////
// dynamic outut
//////////////////////////////////////
//////////////////////////////////////
void ModelController::setupDynamicOutput(QString fieldList)
{
    mDynFieldList.clear();
    if (!fieldList.isEmpty()) {
        QRegularExpression re("((?:\\[.+\\]|\\w+)\\.\\w+)");
        for (const QRegularExpressionMatch &match : re.globalMatch(fieldList)) {
            mDynFieldList.append(match.captured(1));
        }

        mDynFieldList.prepend("count");
        mDynFieldList.prepend("year"); // fixed fields.
    }
    mDynData.clear();
    mDynData.append(mDynFieldList.join(";"));
    mDynamicOutputEnabled = true;
}

QString ModelController::dynamicOutput()
{
    return mDynData.join("\n");
}

const QStringList aggList = QStringList() << "mean" << "sum" << "min" << "max" << "p25" << "p50" << "p75" << "p5"<< "p10" << "p90" << "p95";
void ModelController::fetchDynamicOutput()
{
    if (!mDynamicOutputEnabled || mDynFieldList.isEmpty())
        return;
    DebugTimer t("dynamic output");
    QStringList var;
    QString lastVar = "";
    QVector<double> data;
    AllTreeIterator at(mModel);
    TreeWrapper tw;
    int var_index;
    StatData stat;
    double value;
    QStringList line;
    Expression custom_expr;
    bool simple_expression;
    foreach (QString field, mDynFieldList) {
        if (field=="count" || field=="year")
            continue;
// - removed this to get rid of QRegExp - not sure if it is still used?
//        if (field.count()>0 && field.at(0)=='[') {
//            //QRegularExpression rex("\\[(.+)\\]\\.(\\w+)");
//            QRegExp rex("\\[(.+)\\]\\.(\\w+)");
//            rex.indexIn(field);
//            var = rex.capturedTexts();
//            var.pop_front(); // drop first element (contains the full string)
//            simple_expression = false;
//        } else {
        var = field.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
        simple_expression = true;
        //}
        if (var.count()!=2)
                throw IException(QString("Invalid variable name for dynamic output:") + field);
        if (var.first()!=lastVar) {
            // load new field
            data.clear();
            at.reset(); var_index = 0;
            if (simple_expression) {
                var_index = tw.variableIndex(var.first());
                if (var_index<0)
                    throw IException(QString("Invalid variable name for dynamic output:") + var.first());

            } else {
                custom_expr.setExpression(var.first());
                custom_expr.setModelObject(&tw);
            }
            while (Tree *t = at.next()) {
                tw.setTree(t);
                if (simple_expression)
                    value = tw.value(var_index);
                else
                    value = custom_expr.execute();
                data.push_back(value);
            }
            stat.setData(data);
        }
        // fetch data
        var_index = aggList.indexOf(var[1]);
        switch (var_index) {
            case 0: value = stat.mean(); break;
            case 1: value = stat.sum(); break;
            case 2: value = stat.min(); break;
            case 3: value = stat.max(); break;
            case 4: value = stat.percentile25(); break;
            case 5: value = stat.median(); break;
            case 6: value = stat.percentile75(); break;
            case 7: value = stat.percentile(5); break;
            case 8: value = stat.percentile(10); break;
            case 9: value = stat.percentile(90); break;
            case 10: value = stat.percentile(95); break;
            default: throw IException(QString("Invalid aggregate expression for dynamic output: %1\nallowed:%2")
                                  .arg(var[1]).arg(aggList.join(" ")));
        }
        line+=QString::number(value);
    }
    line.prepend( QString::number(data.size()) );
    line.prepend( QString::number(GlobalSettings::instance()->currentYear()) );
    mDynData.append(line.join(";"));
}

void ModelController::saveDebugOutputs(bool is_final)
{
    // save to files if switch is true
    if (!GlobalSettings::instance()->settings().valueBool("system.settings.debugOutputAutoSave"))
        return;

    bool clear_data = GlobalSettings::instance()->settings().paramValueBool("debug_clear");
    bool do_append=false;
    if (clear_data && currentYear()>2) // >2: is called after incrementing the year counter
        do_append = true;
    QString p = GlobalSettings::instance()->path("debug_", "temp");

    if (is_final) {
        Helper::saveToTextFile(p+"dynamic.csv", dynamicOutput());
        //Helper::saveToTextFile(p+ "version.txt", verboseVersion());
    }


    // write outputs
    saveDebugOutputsCore(p, do_append);

    if (logLevelDebug())
        qDebug() << "saved debug outputs to" << p;

    if (clear_data)
        GlobalSettings::instance()->clearDebugLists();  // clear debug data


}

void ModelController::saveDebugOutputsCore(QString p, bool do_append)
{
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dTreePartition, ";", p + "tree_partition.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dTreeGrowth, ";", p + "tree_growth.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dTreeNPP, ";", p + "tree_npp.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dStandGPP, ";", p + "stand_gpp.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dWaterCycle, ";", p + "water_cycle.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dDailyResponses, ";", p + "daily_responses.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dEstablishment, ";", p + "establishment.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dSaplingGrowth, ";", p + "saplinggrowth.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dCarbonCycle, ";", p + "carboncycle.csv", do_append);
    GlobalSettings::instance()->debugDataTable(GlobalSettings::dPerformance, ";", p + "performance.csv", do_append);

}

void ModelController::saveDebugOutputJs(bool do_clear)
{
    QString p = GlobalSettings::instance()->path("debug_", "temp");

    // save the debug outputs, start new file(s):
    saveDebugOutputsCore(p, false);

    if (do_clear)
        GlobalSettings::instance()->clearDebugLists();  // clear debug data


}

void ModelController::saveScreenshot(QString file_name)
{
#ifdef ILAND_GUI
    if (!mViewerWindow)
        return;
    QImage img = mViewerWindow->screenshot();
    img.save(GlobalSettings::instance()->path(file_name));
#else
    Q_UNUSED(file_name);
#endif
}

void ModelController::paintMap(MapGrid *map, double min_value, double max_value)
{
#ifdef ILAND_GUI
    if (mViewerWindow) {
        mViewerWindow->paintGrid(map, "", GridViewRainbow, min_value, max_value);
        qDebug() << "painted map grid" << map->name() << "min-value (blue):" << min_value << "max-value(red):" << max_value;
    }
#else
    Q_UNUSED(map);Q_UNUSED(min_value);Q_UNUSED(max_value);
#endif
}

void ModelController::paintGrid(Grid<double> *grid, QString name, GridViewType view_type, double min_value, double max_value)
{
#ifdef ILAND_GUI
    if (mViewerWindow) {
        mViewerWindow->paintGrid(grid, name, view_type, min_value, max_value);
        qDebug() << "painted custom grid min-value (blue):" << min_value << "max-value(red):" << max_value;
    }
#else
    Q_UNUSED(grid);Q_UNUSED(min_value);Q_UNUSED(max_value);Q_UNUSED(name); Q_UNUSED(view_type)
#endif
}

void ModelController::addScriptLayer(Grid<double> *grid, MapGrid *map, QString name)
{
#ifdef ILAND_GUI
    if (mViewerWindow) {
        if (map)
            mViewerWindow->addPaintLayer(nullptr, map, name);
        if (grid)
            mViewerWindow->addPaintLayer(grid, nullptr, name);

    }
#else
    Q_UNUSED(grid);Q_UNUSED(map);
#endif

}

void ModelController::removeMapGrid(Grid<double> *grid, MapGrid *map)
{
#ifdef ILAND_GUI
    if (mViewerWindow) {
        mViewerWindow->removePaintLayer(grid, map);
    }
#else
    Q_UNUSED(grid);Q_UNUSED(map);
#endif
}

void ModelController::addGrid(const FloatGrid *grid, const QString &name, const GridViewType view_type, double min_value, double max_value)
{
#ifdef ILAND_GUI

    if (mViewerWindow) {
        mViewerWindow->paintGrid(grid, name, view_type, min_value, max_value);
        qDebug() << "painted grid min-value (blue):" << min_value << "max-value(red):" << max_value;
    }
#else
    Q_UNUSED(grid); Q_UNUSED(name); Q_UNUSED(view_type); Q_UNUSED(min_value);Q_UNUSED(max_value);
#endif
}

void ModelController::addLayers(const LayeredGridBase *layers, const QString &name)
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->addLayers(layers, name);
    //qDebug() << layers->names();
#else
    Q_UNUSED(layers); Q_UNUSED(name);
#endif
}
void ModelController::removeLayers(const LayeredGridBase *layers)
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->removeLayers(layers);
    //qDebug() << layers->names();
#else
    Q_UNUSED(layers);
#endif
}

void ModelController::addPaintLayers(QObject *handler, const QStringList names, const QVector<GridViewType> view_types)
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->addPaintLayers(handler, names, view_types);

#else
    Q_UNUSED(handler) Q_UNUSED(names) Q_UNUSED(view_types)
#endif
}

void ModelController::removePaintLayers(QObject *handler)
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->removePaintLayers(handler);
#else
    Q_UNUSED(handler)
#endif
}

Grid<double> *ModelController::preparePaintGrid(QObject *handler, QString name, std::pair<QStringList, QStringList> *rNamesColors)
{
    // call the slot "paintGrid" from the handler.
    // the handler slot should return a pointer to a (double) grid
    Grid<double> *grid_ptr = nullptr;
    bool success = false;
    handler->dumpObjectTree();


    //if (handler->metaObject()->indexOfMethod("paintGrid")>-1) {
        success = QMetaObject::invokeMethod(handler, "paintGrid", Qt::DirectConnection,
                                                 Q_RETURN_ARG(Grid<double> *, grid_ptr),
                                                 Q_ARG(QString, name),
                                                 Q_ARG(QStringList&, rNamesColors->first),
                                                 Q_ARG(QStringList&, rNamesColors->second)
                                                 );
    //}

    if (success) {
        return grid_ptr;
    }

    // In case the request is not handled, we fall back to asking BITE.
    Grid<double> *grid = BITE::BiteEngine::instance()->preparePaintGrid(handler, name);
    if (grid)
        return grid;
    return nullptr;
}

QStringList ModelController::evaluateClick(QObject *handler, const QPointF coord, const QString &grid_name)
{
    return BITE::BiteEngine::instance()->evaluateClick(handler, coord, grid_name);
}

double ModelController::valueAtHandledGrid(QObject *handler, const QPointF coord, const int layer_id)
{
    return BITE::BiteEngine::instance()->variableValueAt(handler, coord, layer_id);
}

void ModelController::setViewport(QPointF center_point, double scale_px_per_m)
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->setViewport(center_point, scale_px_per_m);
#else
    Q_UNUSED(center_point);Q_UNUSED(scale_px_per_m);
#endif
}

void ModelController::setUIShortcuts(QVariantMap shortcuts)
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->setUIshortcuts(shortcuts);
#else
    Q_UNUSED(shortcuts);
#endif
}

void ModelController::repaint()
{
#ifdef ILAND_GUI
    if (mViewerWindow)
        mViewerWindow->repaint();
#endif
}







