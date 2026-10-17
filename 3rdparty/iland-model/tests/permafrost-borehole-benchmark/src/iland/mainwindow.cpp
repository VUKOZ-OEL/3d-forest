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

#include <QtCore>
#include <QtWidgets>
#include <QtXml>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QJSValueIterator>

#include <signal.h>

#include "global.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "settingmetadata.h"

#include "model.h"
#include "standloader.h"
#include "stampcontainer.h"
#include "resourceunit.h"
#include "speciesset.h"
#include "tree.h"
#include "species.h"
#include "seeddispersal.h"
#include "saplings.h"
#include "climate.h"

#include "exception.h"
#include "helper.h"
#include "colors.h"
#include "debugtimer.h"
#include "statdata.h"

#include "paintarea.h"

#include "expression.h"
#include "expressionwrapper.h"
#include "management.h"
#include "outputmanager.h"

#include "tests.h"
#include "mapgrid.h"
#include "layeredgrid.h"
#include "dem.h"
#include "svdstate.h"

#include "forestmanagementengine.h" // ABE

// global settings
static QDomDocument xmldoc;
static QDomNode xmlparams;

/** @class MainWindow
   @ingroup GUI
   The main window of the iLand viewer.


  */


double distance(const QPointF &a, const QPointF &b)
{
    return sqrt( (a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y()) );
}


double nrandom(const float& p1, const float& p2)
{
    return p1 + (p2-p1)*(rand()/float(RAND_MAX));
}

static bool showDebugMessages=true;
static QStringList bufferedMessages;
static bool doBufferMessages = false;
static bool doLogToWindow = false;
void logToWindow(bool mode)
{
    doLogToWindow = mode;
}
class LogToWindow
{
public:
    LogToWindow() { logToWindow(true);}
    ~LogToWindow() { logToWindow(false);}
};


static QMutex qdebug_mutex;
void dumpMessages();
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    Q_UNUSED(context);
    QMutexLocker m(&qdebug_mutex);
    //QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        if (showDebugMessages) {
            if (qstrcmp(context.category, "default")!=0)
                bufferedMessages.append(QString("%1: %2").arg(context.category).arg(msg));
            else
                bufferedMessages.append(QString(msg));
        }

        break;
    case QtWarningMsg:
    case QtInfoMsg: {
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stderr, "Info: %s\n", localMsg.constData());
        bufferedMessages.append(msg);
        break; }
    case QtCriticalMsg: {
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stderr, "Critical: %s\n", localMsg.constData());
        bufferedMessages.append(msg);
        break; }
    case QtFatalMsg: {
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stderr, "Fatal: %s\n", localMsg.constData());
        bufferedMessages.append(msg);

        QString file_name = GlobalSettings::instance()->path("fatallog.txt","log");
        Helper::msg(QString("Fatal message encountered:\n%1\nFatal-Log-File: %2").arg(msg, file_name));
        dumpMessages();
        Helper::saveToTextFile(file_name, MainWindow::logSpace()->toPlainText() + bufferedMessages.join("\n"));
    }
    }
    if (!doBufferMessages || bufferedMessages.count()>5000)
        dumpMessages();
}

static QMutex dump_message_mutex;
void dumpMessages()
{
    QMutexLocker m(&dump_message_mutex); // serialize access
    // 2011-03-08: encountered "strange" crashes
    // when a warning within Qt lead to a infinite loop/deadlock (also caused by mutex locking)
    // now we prevent this by installing temporarily a 0-handler
    qInstallMessageHandler(nullptr);

    if (MainWindow::logStream() && !doLogToWindow) {
        foreach(const QString &s, bufferedMessages)
            *MainWindow::logStream() << s << endl;
        MainWindow::logStream()->flush();

    } else {
        foreach(const QString &s, bufferedMessages)
            MainWindow::logSpace()->appendPlainText(s);

        // it does *not* work to just MainWindow::logSpace()->textCursor().movePosition()!
        // you have to "setTextCursor()".
        QTextCursor cursor = MainWindow::logSpace()->textCursor();
        cursor.movePosition(QTextCursor::End);
        MainWindow::logSpace()->setTextCursor(cursor);
        MainWindow::logSpace()->ensureCursorVisible();
    }

    bufferedMessages.clear();
    qInstallMessageHandler(myMessageOutput);
}


// handle signal...
// source: http://cplusplus.com/forum/unices/13455/
void handle_signal( int signo ) {
    QString file_name = GlobalSettings::instance()->path("fatallog.txt","log");
    //Helper::msg(QString("Fatal message encountered:\n%1\nFatal-Log-File: %2").arg(msg, file_name));
    dumpMessages();
    Helper::saveToTextFile(file_name, MainWindow::logSpace()->toPlainText() + bufferedMessages.join("\n"));

    qDebug() << "*** Received signal "<< signo << "****";
    Helper::msg(QString("Received Signal:%1\nCheck 'fatallog.txt' in your iLand directory.").arg(signo));
    //dumpMessages();

}

void MainWindow::bufferedLog(bool bufferLog)
{
    doBufferMessages = bufferLog;
    if (bufferLog==false)
        dumpMessages();
}

void MainWindow::setupFileLogging(const bool do_start)
{
    if (mLogStream) {
        if (mLogStream->device())
            delete mLogStream->device();
        delete mLogStream;
        mLogStream = nullptr;
    }
    if (!do_start)
        return;

    if (GlobalSettings::instance()->settings().value("system.logging.logTarget", "console") == "file") {
        QString fname = GlobalSettings::instance()->settings().value("system.logging.logFile", "logfile.txt");
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        fname.replace("$date$", timestamp);
        fname = GlobalSettings::instance()->path(fname, "log");
        QFile *file = new QFile(fname);

        if (!file->open(QIODevice::WriteOnly)) {
            qDebug() << "cannot open logfile" << fname;
        } else {
            qDebug() << "Log output is redirected to logfile" << fname;
            mLogStream = new QTextStream(file);
        }
    }

}


QPlainTextEdit *MainWindow::mLogSpace=nullptr;
QTextStream *MainWindow::mLogStream=nullptr;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);

    connect( ui->PaintWidget, SIGNAL(needsPainting(QPainter&)),
             this, SLOT(repaintArea(QPainter&)) );
    connect (ui->PaintWidget, SIGNAL(mouseClick(QPoint)),
             this, SLOT(mouseClick(const QPoint&)));
    connect(ui->PaintWidget, SIGNAL(mouseDrag(QPoint,QPoint,Qt::MouseButton)),
            this, SLOT(mouseDrag(const QPoint&, const QPoint &, const Qt::MouseButton)));
    connect(ui->PaintWidget, SIGNAL(mouseMove(QPoint)),
            this, SLOT(mouseMove(const QPoint&)));
    connect(ui->PaintWidget, SIGNAL(mouseWheel(QPoint, int)),
            this, SLOT(mouseWheel(const QPoint&, int)));

    // javascript console
    connect(ui->scriptCode, SIGNAL(executeJS(QString)),
            this, SLOT(executeJS(QString)) );

    // dock windows
    ui->menuView->addAction( ui->dockEditor->toggleViewAction() );
    ui->menuView->addAction( ui->dockLogviewer->toggleViewAction() );
    ui->menuView->addAction( ui->dockWidget->toggleViewAction() );
    ui->menuView->addAction( ui->dockModelDrill->toggleViewAction() );

    ui->pbResults->setMenu(ui->menuOutput_menu);

    mLogSpace = ui->logOutput;
    mLogSpace->setMaximumBlockCount(1000000); // set a maximum for the in-GUI size of messages. // removed in Qt5 (because it works ;) )
    qInstallMessageHandler(myMessageOutput);
    // install signal handler
    signal( SIGSEGV, handle_signal );

    readSettings();

    // create global scripting context
    GlobalSettings::instance()->resetScriptEngine();

    // load xml file: use either a command-line argument (if present), or load the content of a small text file....
    QString argText = QApplication::arguments().last();
    if (QApplication::arguments().count()>1 && !argText.isEmpty()) {
        ui->initFileName->setText(argText);
    } else {
        QString lastXml = QSettings().value("project/lastxmlfile").toString();
        //QString lastXml = Helper::loadTextFile( QCoreApplication::applicationDirPath()+ "/lastxmlfile.txt" );
        if (!lastXml.isEmpty() && QFile::exists(lastXml))
            ui->initFileName->setText(lastXml);
    }
    QString xmlFile = Helper::loadTextFile(ui->initFileName->text());

    if (!xmlFile.isEmpty()) {
        ui->iniEdit->setPlainText(xmlFile);
        QString errMsg;
        int errLine, errCol;
        if (!xmldoc.setContent(xmlFile, &errMsg, &errLine, &errCol)) {
            QMessageBox::information(this, "title text", QString("Cannot set content of XML file %1. \nat line %2 col %3: %4 ")
                                     .arg(ui->initFileName->text()).arg(errLine).arg(errCol).arg(errMsg));
            //return;
        }
    }

    on_actionEdit_XML_settings_triggered();

    qDebug() << "threadcount: " << QThread::idealThreadCount();

    // load window settings
    QString fileName = QDir::current().filePath("gui.txt");
    if (QFile::exists(fileName)) {

        QByteArray state = Helper::loadFile(fileName);
        restoreState(state);
    }
    checkModelState();
    ui->statusBar->addPermanentWidget(ui->modelRunProgress);
    ui->modelRunProgress->setValue(0);
    mStatusLabel = new QLabel(this);
    labelMessage("no model created.");
    ui->statusBar->addWidget(mStatusLabel);
    // remote control of model
    connect(&mRemoteControl, SIGNAL(year(int)),this,SLOT(yearSimulated(int)));
    connect(&mRemoteControl, SIGNAL(finished(QString)), this, SLOT(modelFinished(QString)));
    connect(&mRemoteControl, SIGNAL(stateChanged()), this, SLOT(checkModelState()));

    // log levels
    ui->actionDebug->setProperty("logLevel", QVariant(0));
    ui->actionInfo->setProperty("logLevel", QVariant(1));
    ui->actionWarning->setProperty("logLevel", QVariant(2));
    ui->actionError->setProperty("logLevel", QVariant(3));

    // species filter
    connect( ui->speciesFilterBox, SIGNAL(currentIndexChanged(int)), SLOT(repaint()));
    updatePaintGridList();

    // model controller
    mRemoteControl.setMainWindow( this );
    mRemoteControl.connectSignals();
    GlobalSettings::instance()->setModelController( &mRemoteControl );

    // automatic run
    if (QApplication::arguments().contains("run")) {
        QTimer::singleShot(3000, this, SLOT(automaticRun()));
    }

    // to silence some warnings during startup - maybe not required (anymore):
    qRegisterMetaType<QTextBlock>("QTextBlock");
    qRegisterMetaType<QTextCursor>("QTextCursor");

    ui->iniEdit->setVisible(false);
    ui->editStack->setTabEnabled(4,false); // the "other" tab

    // qml setup
    mRulerColors = new Colors();
    mRuler = nullptr;
    if (!QApplication::arguments().contains("noqml")) {

        //qDebug() << "mainwindow init before qml";
        QQuickView *view = new QQuickView();
        mRuler = view;
        //qDebug() << "createWindowContainer:";
        QWidget *container = QWidget::createWindowContainer(view, this);

        //qDebug() << "setContextProperty:";
        view->engine()->rootContext()->setContextProperty("rulercolors", mRulerColors);
        view->setResizeMode(QQuickView::SizeRootObjectToView);
        ui->pbReloadQml->setVisible(false); // enable for debug...
        //view->setSource(QUrl::fromLocalFile("E:/dev/iland_port_qt5_64bit/src/iland/qml/ruler.qml"));

        //qDebug() << "setSource:";
        view->setSource(QUrl("qrc:/qml/ruler.qml"));

        //view->show();
        //qDebug() << "addWidget:";
        ui->qmlRulerLayout->addWidget(container);
    }
    //ui->qmlRulerLayout->addWidget(container);
    //    QDir d(":/qml");
    //    qDebug() << d.entryList();
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    ui->logOutput->setFont(font);
    //qDebug() << "mainwindow init done";

}


MainWindow::~MainWindow()
{
    mRemoteControl.destroy(); // delete model and free resources.
    delete ui;
}

void MainWindow::batchLog(const QString s)
{
    QFile outfile(QCoreApplication::applicationDirPath()+ "/batchlog.txt");
    if (outfile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream str(&outfile);
        str << s << endl;
    }
}

/// automatically start a simulation...
void MainWindow::automaticRun()
{
    // start a simulation
    setWindowTitle("iLand viewer --- batch mode");
    batchLog("*** iland batch mode ***");
    batchLog(QString("%1 Loading project %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate),
                                                  ui->initFileName->text()));
    batchLog(QString("%1 Loading the model...").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
    setupModel();

    int count = QCoreApplication::arguments()[QCoreApplication::arguments().count()-2].toInt();
    if (count==0) {
        qDebug() << "invalid number of years....";
        return;
    }
    ui->modelRunProgress->setMaximum(count-1);
    batchLog(QString("%1 Running the model (%2 years)...").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(count));
    // note the "+1": this is similar to the normal way of starting
    // "1" means in Globals.year that we are in the 1st year.
    // the simulation stops when reaching the count+1 year.
    mRemoteControl.run(count + 1);

    // see the finsished() slot
}

void MainWindow::updateLabel()
{
    if (mRemoteControl.isRunning()) {
        labelMessage(QString("Running.... year %1 of %2 [%3] %4").arg(mRemoteControl.currentYear()).arg(mRemoteControl.totalYears()).arg(mRemoteControl.model()->currentTask(), mRemoteControl.timeString()));
    } else if (mRemoteControl.isStartingUp()) {
        labelMessage(QString("Creating model - %3").arg(mRemoteControl.model()->currentTask()));
    }
}

// simply command an update of the painting area
void MainWindow::repaint()
{
    ui->PaintWidget->update();
    //QCoreApplication::processEvents();
}

// control GUI actions
void MainWindow::checkModelState()
{
    ui->actionModelCreate->setEnabled(mRemoteControl.canCreate()&& !mRemoteControl.isRunning());
    ui->actionModelDestroy->setEnabled(mRemoteControl.canDestroy() && !mRemoteControl.isRunning());
    ui->actionModelRun->setEnabled(mRemoteControl.canRun() && !mRemoteControl.isPaused() && !mRemoteControl.isRunning());
    ui->actionRun_one_year->setEnabled(mRemoteControl.canRun() && !mRemoteControl.isPaused()&& !mRemoteControl.isRunning());
    ui->actionReload->setEnabled(mRemoteControl.canDestroy() && !mRemoteControl.isRunning());
    ui->actionStop->setEnabled(mRemoteControl.isRunning());
    ui->actionPause->setEnabled(mRemoteControl.isRunning());
    ui->actionPause->setText(mRemoteControl.isPaused()?"Continue":"Pause");
    dumpMessages();
}




void MainWindow::readwriteCycle()
{

    if (!mRemoteControl.canRun())
        return;

    Model *model = mRemoteControl.model();
    model->onlyApplyLightPattern();
}




QString MainWindow::dumpTreelist()
{
    if (!mRemoteControl.isRunning())
        return "";

    Model *model = mRemoteControl.model();

    AllTreeIterator at(model);
    DebugList treelist;
    QString line;
    QStringList result;
    result << "id;species;dbh;height;x;y;RU#;LRI;mWoody;mRoot;mFoliage;LA";
    while (Tree *tree = at.next()) {
        treelist.clear();
        tree->dumpList(treelist);
        line = "";
        foreach(QVariant value, treelist)
            line+=value.toString() + ";";
        result << line;
    }
    QString resStr = result.join("\n");
    return resStr;
}

void MainWindow::updatePaintGridList()
{

    ui->otherGridTree->clear();
    QList<QTreeWidgetItem *> items;
    QStack<QTreeWidgetItem*> stack;
    stack.push(nullptr);
    QMap<QString, PaintObject>::const_iterator i = mPaintList.begin();
    QString group=QString();
    while (i!=mPaintList.constEnd()) {
        QStringList elem = i.key().split("-");
        if (elem.size()>1 && elem[0] != group) {
            if (stack.size()>1)
                stack.pop();
            items.append(new QTreeWidgetItem(stack.last(), QStringList() << elem[0]));
            stack.push(items.back());
            group = elem[0];
        }

        items.append( new QTreeWidgetItem(stack.last(),  QStringList() << (elem.size()>1 ? elem[1] : elem[0])) );
        items.back()->setToolTip(0, i.value().description);
        items.back()->setData(0, Qt::UserRole+0, i.key());
        ++i;
    }
    ui->otherGridTree->addTopLevelItems(items);


}

void MainWindow::addLayers(const LayeredGridBase *layer, const QString &name)
{
    const QVector<LayeredGridBase::LayerElement> &names = const_cast<LayeredGridBase*>(layer)->names();
    int layer_id = 0;
    QString current_layer = mPaintNext.name;
    foreach (const LayeredGridBase::LayerElement &layername, names) {
        QString comb_name = QString("%1 - %2").arg(name, layername.name);
        PaintObject po;
        po.what = PaintObject::PaintLayers;
        po.view_type = layername.view_type;
        po.layered = layer;
        po.layer_id = layer_id++;
        po.name = layername.name;
        po.auto_range = true;
        mPaintList[comb_name] = po;
        if (current_layer == po.name)
            mPaintNext.what = PaintObject::PaintHeightGrid;
    }
    updatePaintGridList();
}

void MainWindow::addPaintLayers(QObject *handler, const QStringList names, const QVector<GridViewType> view_types)
{
    for (int i=0;i<names.count();++i) {
        if (mPaintList.contains(names[i]))
            throw IException("The name '" + names[i] + "' is already in the list of paint items. (This should not happen. Please file a complaint!)");
        QMap<QString, PaintObject>::iterator po = mPaintList.insert(names[i], PaintObject());
        po.value().handler = handler;
        po.value().what = PaintObject::PaintHandledObject;
        po.value().expression = names[i];
        po.value().auto_range = true;
        po.value().view_type = view_types.size()>=i ? view_types[i] : GridViewRainbow;
        po.value().layer_id = i;
        po.value().name = names[i];
    }
    updatePaintGridList();
}

void MainWindow::removePaintLayers(QObject *handler)
{
    // remove all layers that have the given handler
    QMutableMapIterator<QString, PaintObject> i(mPaintList);
    while (i.hasNext()) {
        i.next();
        if (i.value().handler == handler)
            i.remove();
    }
    if (mPaintNext.handler == handler)
        mPaintNext.what = PaintObject::PaintNothing;

    updatePaintGridList();
}

void MainWindow::removeLayers(const LayeredGridBase *layer)
{
    QMap<QString, PaintObject>::iterator it=mPaintList.begin();
    while(it!=mPaintList.end())
        if (it->layered == layer)
            it = mPaintList.erase(it);
        else
            ++it;
    if (mPaintNext.layered == layer)
        mPaintNext.what = PaintObject::PaintHeightGrid;
}

void MainWindow::setPaintGrid(const QString grid_name)
{
    if (!mPaintList.contains(grid_name))
        throw IException(QString("The name '%1' is not a valid name for a grid (use <category> - <name>, e.g. 'wind - basalArea'!").arg(grid_name));
    PaintObject &po = mPaintList[grid_name];
    mPaintNext = po;

}


void MainWindow::paintGrid(MapGrid *map_grid, const QString &name,
                           const GridViewType view_type,
                           double min_val, double max_val)
{
    if (map_grid==nullptr && !name.isEmpty()) {
        // remove the grid from the list
        mPaintList.remove(name);
        updatePaintGridList();
        return;
    }
    mPaintNext.what=PaintObject::PaintMapGrid;
    mPaintNext.min_value=min_val; mPaintNext.max_value=max_val;
    mPaintNext.map_grid = map_grid; mPaintNext.view_type = view_type;
    if (!name.isEmpty()) {
        updatePaintGridList();
        mPaintList[name] = mPaintNext;
    }
    if (mRemoteControl.canRun()) {
        ui->visOtherGrid->setChecked(true);
        repaint();
    } else {
        mPaintNext.what = PaintObject::PaintHeightGrid;
    }
}

void MainWindow::paintGrid(const FloatGrid *grid, const QString &name,
                           const GridViewType view_type,
                           double min_val, double max_val)
{
    if (grid==nullptr && !name.isEmpty()) {
        // remove the grid from the list
        mPaintList.remove(name);
        updatePaintGridList();
        return;
    }
    mPaintNext.what=PaintObject::PaintFloatGrid;
    mPaintNext.min_value=min_val;
    mPaintNext.max_value=max_val;
    mPaintNext.float_grid = grid;
    mPaintNext.view_type = view_type;
    mPaintNext.name = name;
    if (!name.isEmpty()) {
        mPaintList[name] = mPaintNext;
        updatePaintGridList();
    }
    if (mRemoteControl.canRun()) {
        ui->visOtherGrid->setChecked(true);
        repaint();
    } else {
        mPaintNext.what = PaintObject::PaintHeightGrid;
    }
}

void MainWindow::paintFON(QPainter &painter, QRect rect)
{
    DebugTimer drawtimer("painting");
    drawtimer.setSilent();
    // debug redraw counter
    ui->lRedraws->setText( QString::number( ui->lRedraws->text().toInt() + 1 )  );

    if (!mRemoteControl.canRun())
        return;
    Model *model = mRemoteControl.model();

    FloatGrid *grid = model->grid();
    HeightGrid *domGrid = model->heightGrid();
    // do the actual painting
    if (!grid)
        return;
    bool auto_scale_color = ui->visAutoScale->isChecked();
    bool show_fon = ui->visFon->isChecked();
    bool show_dom = ui->visDomGrid->isChecked();
    bool show_trees = ui->visImpact->isChecked();
    bool species_color = ui->visSpeciesColor->isChecked();
    bool show_ru = ui->visResourceUnits->isChecked();
    bool show_regeneration = ui->visRegeneration->isChecked();
    bool show_seedmaps = ui->visSeeds->isChecked();
    bool other_grid = ui->visOtherGrid->isChecked();
    bool shading = ui->visShading->isChecked();
    bool clip_with_stand_grid = ui->visClipStandGrid->isChecked();
    if (!mRemoteControl.model() || !mRemoteControl.model()->dem())
        shading=false;


    if (other_grid) {

        if (mPaintNext.what != PaintObject::PaintNothing) {
            if (mPaintNext.what == PaintObject::PaintMapGrid) {
                mRulerColors->setCaption(mPaintNext.name);
                paintMapGrid(painter, mPaintNext.map_grid, nullptr, nullptr,
                             mPaintNext.view_type,
                             mPaintNext.min_value, mPaintNext.max_value,
                             shading);
            }

            if (mPaintNext.what == PaintObject::PaintFloatGrid) {
                mRulerColors->setCaption(mPaintNext.name);
                if (!mRulerColors->autoScale()) {
                    mPaintNext.cur_max_value = mRulerColors->maxValue();
                    mPaintNext.cur_min_value = mRulerColors->minValue();
                } else {
                    mPaintNext.cur_max_value = mPaintNext.max_value;
                    mPaintNext.cur_min_value = mPaintNext.min_value;
                }
                paintMapGrid(painter, nullptr, mPaintNext.float_grid, nullptr,
                             mPaintNext.view_type,
                             mPaintNext.cur_min_value, mPaintNext.cur_max_value, shading);
            }

            if (mPaintNext.what == PaintObject::PaintLayers)
                paintGrid(painter, mPaintNext);

            if (mPaintNext.what == PaintObject::PaintHandledObject) {
                mRulerColors->setCaption(mPaintNext.name);
                // prepare grid via the handler
                std::pair<QStringList, QStringList> names_colors = std::pair<QStringList, QStringList>(QStringList(), QStringList());
                mPaintNext.dbl_grid = mRemoteControl.preparePaintGrid(mPaintNext.handler, mPaintNext.expression, &names_colors);
                if (!mPaintNext.dbl_grid)
                    return; // no painting
                if (mPaintNext.view_type == GridViewCustom) {
                    // custom colors and names
                    mRulerColors->setFactorColors(names_colors.second);
                    mRulerColors->setFactorLabels(names_colors.first);
                    mRulerColors->setPalette(GridViewCustom, 0., 1.);
                    mPaintNext.cur_max_value = names_colors.second.size();
                    mPaintNext.cur_min_value = 0.;

                } else {
                    // standard grids
                    if (!mRulerColors->autoScale()) {
                        mPaintNext.cur_max_value = mRulerColors->maxValue();
                        mPaintNext.cur_min_value = mRulerColors->minValue();
                    } else {
                        mPaintNext.max_value = mPaintNext.dbl_grid->max();
                        mPaintNext.min_value = mPaintNext.dbl_grid->min();
                        mPaintNext.cur_max_value = mPaintNext.max_value;
                        mPaintNext.cur_min_value = mPaintNext.min_value;
                    }
                }
                paintMapGrid(painter, nullptr, nullptr, mPaintNext.dbl_grid,
                             mPaintNext.view_type, mPaintNext.cur_min_value,
                             mPaintNext.cur_max_value, shading);

            }
            return;
        }
    }


    // clear background
    painter.fillRect(ui->PaintWidget->rect(), Qt::white);

    // what to paint??

    float maxval=1.f; // default maximum
    float minval=0.f;
    if (!auto_scale_color)
        maxval =grid->max();
    if (maxval==0.)
        return;
    QString species;
    if (ui->speciesFilterBox->currentIndex()>-1)
        species = ui->speciesFilterBox->itemData(ui->speciesFilterBox->currentIndex()).toString();

    int ix,iy;
    QColor fill_color;
    float value;

    if (show_fon ) {

        // start from each pixel and query value in grid for the pixel
        mRulerColors->setCaption("Light Influence Field", "value of the LIF at 2m resolution.");
        mRulerColors->setPalette(GridViewRainbowReverse,0., maxval); // ruler
        if (!mRulerColors->autoScale()) {
            maxval = static_cast<float>( mRulerColors->maxValue());
            minval = static_cast<float>( mRulerColors->minValue() );
        }
        int x,y;
        int sizex = rect.width();
        int sizey = rect.height();
        QPointF world;
        QRgb col;
        QImage &img = ui->PaintWidget->drawImage();
        for (x=0;x<sizex;x++)
            for (y=0;y<sizey;y++) {
                world = vp.toWorld(QPoint(x,y));
                if (grid->coordValid(world)) {
                    value = grid->valueAt(world);
                    col = Colors::colorFromValue(value, minval, maxval,true).rgb();
                    img.setPixel(x,y,col);
                }
            }

    }

    if (show_seedmaps) {
        if (species.isEmpty()) {
            qDebug() << "Please select a species!";
            return;
        }
        int x,y;
        mRulerColors->setCaption("Seed availability", QString("seed availability of species %1").arg(species));
        mRulerColors->setPalette(GridViewRainbow,0., 1.); // ruler
        int sizex = rect.width();
        int sizey = rect.height();
        QPointF world;
        QRgb col;
        QImage &img = ui->PaintWidget->drawImage();
        const Grid<float> &grid = GlobalSettings::instance()->model()->speciesSet()->species(species)->seedDispersal()->seedMap();
        QRgb gray_bg = QColor(100,100,100).rgb();

        for (x=0;x<sizex;x++)
            for (y=0;y<sizey;y++) {
                world = vp.toWorld(QPoint(x,y));
                if (grid.coordValid(world)) {
                    value = grid.constValueAt(world);
                    col = value>0.f ? Colors::colorFromValue(value, 0., 1., false).rgb() : gray_bg;
                    if (shading)
                        col = Colors::shadeColor(col, world, mRemoteControl.model()->dem()).rgb();

                    img.setPixel(x,y,col);
                }
            }

    }

    if (show_regeneration ) {

        if (mRegenerationGrid.isEmpty())
            mRegenerationGrid.setup(*model->grid()); // copy
        if (!GlobalSettings::instance()->model()->saplings())
            return;
        static int last_year=0;
        static QString last_species="";
        static bool last_regen_mode=false;
        if (last_year!=GlobalSettings::instance()->currentYear() || species!=last_species || ui->visRegenNew->isChecked()!=last_regen_mode) {
            last_year=GlobalSettings::instance()->currentYear();
            last_species=species;
            bool draw_established = ui->visRegenNew->isChecked();
            last_regen_mode = draw_established;
            // fill grid...
            DebugTimer t("create regeneration map...");
            mRegenerationGrid.wipe(0.f);
            if (species.isEmpty()) {
                // hmax of all species
                for (float *rg=mRegenerationGrid.begin();rg!=mRegenerationGrid.end(); ++rg) {
                    SaplingCell *sc=GlobalSettings::instance()->model()->saplings()->cell(mRegenerationGrid.indexOf(rg));
                    if (sc) {
                        if (draw_established)
                            *rg = sc->has_new_saplings() ? 1.f : 0.f;
                        else
                            *rg = sc->max_height();
                    }

                }
            } else {
                // filter a specific species
                int sidx = GlobalSettings::instance()->model()->speciesSet()->species(species)->index();
                for (float *rg=mRegenerationGrid.begin(); rg!=mRegenerationGrid.end(); ++rg) {
                    SaplingCell *sc=GlobalSettings::instance()->model()->saplings()->cell(mRegenerationGrid.indexOf(rg));
                    if (sc) {
                        SaplingTree *st=sc->saplingOfSpecies(sidx);
                        if (st) {
                            if (draw_established)
                                *rg = st->is_occupied() && st->age<2 ? 1.f : 0.f;
                            else
                                *rg = st ? st->height : 0.f;
                        }
                    }
                }
            }
        }
        // start from each pixel and query value in grid for the pixel
        int x,y;
        mRulerColors->setCaption("Regeneration Layer", "max. tree height of regeneration layer (blue=0m, red=4m)");
        mRulerColors->setPalette(GridViewRainbow,0., 4.); // ruler
        int sizex = rect.width();
        int sizey = rect.height();
        QPointF world;
        QRgb col;
        QImage &img = ui->PaintWidget->drawImage();

        for (x=0;x<sizex;x++)
            for (y=0;y<sizey;y++) {
                world = vp.toWorld(QPoint(x,y));
                if (mRegenerationGrid.coordValid(world)) {
                    value = mRegenerationGrid.valueAt(world);
                    col = Colors::colorFromValue(value, 0., 4., false).rgb(); // 0..4m
                    if (shading)
                        col = Colors::shadeColor(col, world, mRemoteControl.model()->dem()).rgb();
                    if (clip_with_stand_grid && !GlobalSettings::instance()->model()->heightGrid()->valueAt(world).isValid())
                        col = Qt::white;

                    img.setPixel(x,y,col);
                }
            }
    }

    if (show_dom) {
        // paint the lower-res-grid;
        bool stem_height = ui->visDomHeightStem->isChecked();
        float max_val = 50.f;
        if (auto_scale_color) {
            max_val = 0.;
            for (HeightGridValue *v = domGrid->begin(); v!=domGrid->end(); ++v)
                max_val = qMax(max_val, stem_height ? v->stemHeight() : v->height);
        }
        mRulerColors->setCaption("Dominant height (m)", "dominant tree height on 10m pixel.");
        mRulerColors->setPalette(GridViewRainbow,0., max_val); // ruler
        if (!mRulerColors->autoScale()) {
            // min_val = static_cast<float>( mRulerColors->minValue() );
            max_val = static_cast<float>( mRulerColors->maxValue() );
        }
        for (iy=0;iy<domGrid->sizeY();iy++) {
            for (ix=0;ix<domGrid->sizeX();ix++) {
                QPoint p(ix,iy);
                const HeightGridValue &hgv = domGrid->valueAtIndex(p);
                if (hgv.isValid()) {
                    value = stem_height ? domGrid->valueAtIndex(p).stemHeight() : domGrid->valueAtIndex(p).height;
                    QRect r = vp.toScreen(domGrid->cellRect(p));
                    fill_color = Colors::colorFromValue(value, 0., max_val); // 0..50m
                    if (shading)
                        fill_color = Colors::shadeColor(fill_color, domGrid->cellCenterPoint(p), mRemoteControl.model()->dem());

                    painter.fillRect(r, fill_color);
                } else if (shading) {
                    // out of project area is not drawn unless shading is enabled
                    fill_color = Colors::shadeColor(Qt::white, domGrid->cellCenterPoint(p), mRemoteControl.model()->dem());
                    QRect r = vp.toScreen(domGrid->cellRect(p));
                    painter.fillRect(r, fill_color);
                }
                // areas "outside" are drawn as gray.
                if (hgv.isForestOutside()) {
                    QRect r = vp.toScreen(domGrid->cellRect(p));
                    if (hgv.isRadiating())
                        painter.fillRect(r, Qt::gray);
                    else
                        painter.fillRect(r, QColor(240,240,240));

                }
            }
        }

    } // if (show_dom)

    if (show_ru){
        QString ru_expr = ui->lTreeExpr->text();
        if (ru_expr.isEmpty())
            ru_expr = "id";
        RUWrapper ru_wrapper;

        Expression ru_value(ru_expr, &ru_wrapper);
        ru_value.setCatchExceptions(); // silent catching...

        species_color = ui->visRUSpeciesColor->isChecked();
        const Species *drawspecies=nullptr;
        GridViewType view_type = GridViewRainbow;
        if (species_color) {
            drawspecies = model->speciesSet()->species(species);
            view_type = GridViewGreens;
        }

        double min_value = 0.;
        double max_value = 1.; // defaults
        double value;
        if (!mRulerColors->autoScale()) {
            max_value = mRulerColors->maxValue();
            min_value = mRulerColors->minValue();

        } else if (auto_scale_color) {
            min_value = 9999999999999999999.;
            max_value = -999999999999999999.;
            foreach (const ResourceUnit *ru, model->ruList()) {
                if (species_color && drawspecies) {
                    value = ru->constResourceUnitSpecies(drawspecies)->constStatistics().basalArea();
                } else {
                    ru_wrapper.setResourceUnit(ru);
                    value = ru_value.execute();
                }
                min_value = qMin(min_value, value);
                max_value = qMax(max_value, value);
            }
            qDebug() << "scale colors: min" << min_value << "max:" << max_value;
        }

        if (species_color) {
            if (drawspecies) {
                drawspecies = model->speciesSet()->species(species);
                mRulerColors->setCaption("Species share", QString("Species: '%1'").arg(species));
                mRulerColors->setPalette(GridViewGreens, static_cast<float>(min_value), static_cast<float>(max_value)); // ruler
            } else {
                mRulerColors->setCaption("Dominant species on resource unit", "The color indicates the species with the biggest share of basal area. \nDashed fill, if the basal area of the max-species is <50%.");
                QList<const Species*> specieslist=mRemoteControl.availableSpecies();
                QStringList colors; QStringList speciesnames;
                for (int i=0; i<specieslist.count();++i) {
                    colors.append(specieslist[i]->displayColor().name());
                    speciesnames.append(specieslist[i]->name());
                }
                mRulerColors->setFactorColors(colors);
                mRulerColors->setFactorLabels(speciesnames);
                mRulerColors->setPalette(GridViewCustom, 0., 1.);
            }
        } else {
            mRulerColors->setCaption("Resource Units", QString("Result of expression: '%1'").arg(ru_expr));
            mRulerColors->setPalette(GridViewRainbow, static_cast<float>(min_value), static_cast<float>(max_value)); // ruler
        }

        // paint resource units
        painter.setPen(Qt::black);
        foreach (const ResourceUnit *ru, model->ruList()) {
            bool stroke = false;
            if (species_color) {
                if (drawspecies) {
                    value = ru->constResourceUnitSpecies(drawspecies)->constStatistics().basalArea();
                    fill_color = Colors::colorFromValue(static_cast<float>(value), view_type, static_cast<float>(min_value), static_cast<float>(max_value));
                } else {
                    const Species *max_sp=nullptr; double max_ba = 0.; double total_ba=0.;
                    foreach(const ResourceUnitSpecies *rus, ru->ruSpecies()) {
                        total_ba += rus->constStatistics().basalArea();
                        if (rus->constStatistics().basalArea()>max_ba) {
                            max_ba = rus->constStatistics().basalArea();
                            max_sp=rus->species();
                        }
                    }
                    if (max_sp) {
                        fill_color = max_sp->displayColor();
                        if (max_ba < total_ba*0.5) {
                            stroke = true;
                        }
                    } else
                        fill_color = Qt::white;
                }
            } else {
                ru_wrapper.setResourceUnit(ru);
                value = ru_value.execute();
                fill_color = Colors::colorFromValue(static_cast<float>(value), view_type, static_cast<float>(min_value), static_cast<float>(max_value));
            }
            QRect r = vp.toScreen(ru->boundingBox());
            //fill_color = Colors::colorFromValue(value, min_value, max_value);
            //fill_color = Colors::colorFromValue(static_cast<float>(value), view_type, static_cast<float>(min_value), static_cast<float>(max_value));
            if (shading)
                fill_color = Colors::shadeColor(fill_color, ru->boundingBox().center(), mRemoteControl.model()->dem());

            if (stroke)
                painter.fillRect(r, QBrush(fill_color, Qt::Dense3Pattern));
            else
                painter.fillRect(r, fill_color);
        }
        if (shading) {
            const Grid<ResourceUnit*> &mgrid = mRemoteControl.model()->RUgrid();
            for (ResourceUnit **p=mgrid.begin(); p!=mgrid.end(); ++p) {
                if (*p == nullptr) { // paint only boxes outside of the project
                    QRectF rbox = mgrid.cellRect(mgrid.indexOf(p));
                    QRect r = vp.toScreen(rbox);
                    fill_color = Colors::shadeColor(Qt::white, rbox.center(), mRemoteControl.model()->dem());
                    painter.fillRect(r, fill_color);

                }
            }
        }
        if (!ru_value.lastError().isEmpty())
            qDebug() << "Expression error while painting: " << ru_value.lastError();
    }

    if (show_trees) {
        QString single_tree_expr = ui->lTreeExpr->text();
        if (single_tree_expr.isEmpty())
            single_tree_expr = "1-lri";
        TreeWrapper tw;

        Expression tree_value(single_tree_expr, &tw);    // get maximum value
        tree_value.setCatchExceptions(); // silent catching...

        QString filter_expr = ui->expressionFilter->text();
        bool do_filter = ui->cbDrawFiltered->isChecked();
        if (filter_expr.isEmpty())
            filter_expr = "1"; // a constant, always true

        Expression tree_filter(filter_expr, &tw);
        tree_filter.setCatchExceptions();

        bool draw_transparent = ui->drawTransparent->isChecked();
        AllTreeIterator treelist(model);
        Tree *tree;
        painter.setPen(Qt::gray);
        double max_val=1., min_val=0.;
        if (!mRulerColors->autoScale()) {
            max_val = mRulerColors->maxValue(); min_val = mRulerColors->minValue();
        }

        while ((tree = treelist.next())) {
            if ( !vp.isVisible(treelist.currentRU()->boundingBox()) ) {
                continue;
            }
            // filter species...
            if (!species.isEmpty())
                if (tree->species()->id() != species)
                    continue;

            // filter out dead trees
            if (tree->isDead())
                continue;

            // filter (user defined)
            if (do_filter) {
                tw.setTree(tree);
                if (tree_filter.execute()==0.)
                    continue;
            }

            QPointF pos = tree->position();
            QPoint p = vp.toScreen(pos);
            if (species_color) {
                // use species specific color....
                fill_color = tree->species()->displayColor();
            } else {
                // calculate expression
                tw.setTree(tree);
                value = static_cast<float>(tree_value.execute());
                fill_color = Colors::colorFromValue(value, static_cast<float>(min_val), static_cast<float>(max_val), false);
            }
            if (draw_transparent)
                fill_color.setAlpha(80); // 50%
            painter.setBrush(fill_color);
            int diameter = qMax(1,vp.meterToPixel( tree->crownRadius()));
            painter.drawEllipse(p, diameter, diameter);
        }
        if (!tree_value.lastError().isEmpty())
            qDebug() << "Expression error while painting: " << tree_value.lastError();
        // ruler
        if (species_color) {
            mRulerColors->setCaption("Single trees", "species specific colors.");
            QList<const Species*> specieslist=mRemoteControl.availableSpecies();
            QStringList colors; QStringList speciesnames;
            for (int i=0; i<specieslist.count();++i) {
                colors.append(specieslist[i]->displayColor().name());
                speciesnames.append(specieslist[i]->name());
            }
            mRulerColors->setFactorColors(colors);
            mRulerColors->setFactorLabels(speciesnames);
            mRulerColors->setPalette(GridViewCustom, 0., 1.);
        } else {
            mRulerColors->setCaption("Single trees", QString("result of expression: '%1'").arg(single_tree_expr));
            mRulerColors->setPalette(GridViewRainbow, 0., 1.);

        }

    } // if (show_trees)

    // highlight selected tree
    Tree *t = reinterpret_cast<Tree*>( ui->treeChange->property("tree").toLongLong() );
    if (t) {
        QPointF pos = t->position();
        painter.setPen(Qt::black);
        QPoint p = vp.toScreen(pos);
        painter.drawRect( p.x()-1, p.y()-1, 3,3);
    }
    // draw rectangle around the grid
    QRectF r = grid->metricRect();
    QRect rs = vp.toScreen(r);
    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rs);

}

void MainWindow::paintGrid(QPainter &painter, PaintObject &object)
{
    painter.fillRect(ui->PaintWidget->rect(), object.background_color);
    bool clip_with_stand_grid = ui->visClipStandGrid->isChecked();
    bool shading = ui->visShading->isChecked();
    if (!mRemoteControl.model() || !mRemoteControl.model()->dem())
        shading=false;


    int sx=0, sy=0;
    QRect total_rect;
    object.cur_min_value = object.min_value;
    object.cur_max_value = object.max_value;
    switch (object.what) {
    case PaintObject::PaintMapGrid:
        sx = object.map_grid->grid().sizeX();
        sy = object.map_grid->grid().sizeY();
        total_rect = vp.toScreen(object.map_grid->grid().metricRect());
        mRulerColors->setCaption("Map grid");
        break;
    case PaintObject::PaintFloatGrid:
        sx = object.float_grid->sizeX();
        sy = object.float_grid->sizeY();
        total_rect = vp.toScreen(object.float_grid->metricRect());
        mRulerColors->setCaption("Floating point grid");
        break;
    case PaintObject::PaintLayers:
        sx = object.layered->sizeX();
        sy = object.layered->sizeY();
        total_rect = vp.toScreen(object.layered->metricRect());
        if (object.auto_range) {
            object.layered->range( object.cur_min_value, object.cur_max_value, object.layer_id );
        }
        mRulerColors->setCaption(const_cast<LayeredGridBase*>(object.layered)->names()[object.layer_id].name,
                const_cast<LayeredGridBase*>(object.layered)->names()[object.layer_id].description);
        break;
    case PaintObject::PaintNothing:
        mRulerColors->setCaption("-");
        return;
    default: return;
    }
    if (!mRulerColors->autoScale()) {
        object.cur_max_value = mRulerColors->maxValue();
        object.cur_min_value = mRulerColors->minValue();
    }



    int ix,iy;
    double value=0.;
    QRect r;
    QColor fill_color;
    double max_value = -1.;
    QPointF pmetric;
    for (iy=0;iy<sy;iy++) {
        for (ix=0;ix<sx;ix++) {
            QPoint p(ix,iy);
            switch(object.what) {
            case PaintObject::PaintMapGrid:
                value = object.map_grid->grid().constValueAtIndex(p);
                pmetric = object.map_grid->grid().cellRect(p).center();
                r = vp.toScreen(object.map_grid->grid().cellRect(p));

                break;
            case PaintObject::PaintFloatGrid:
                value = object.float_grid->constValueAtIndex(p);
                pmetric = object.float_grid->cellRect(p).center();
                r = vp.toScreen(object.float_grid->cellRect(p));
                break;
            case PaintObject::PaintLayers:
                value = object.layered->value(ix, iy, object.layer_id);
                pmetric = object.layered->cellRect(p).center();
                max_value = qMax(max_value, value);
                r = vp.toScreen(object.layered->cellRect(p));
                break;
            default: ;
            }
            if (clip_with_stand_grid && !GlobalSettings::instance()->model()->heightGrid()->valueAt(pmetric).isValid()) {
                fill_color = Qt::white;
            } else {
                fill_color = Colors::colorFromValue(value, object.view_type, object.cur_min_value, object.cur_max_value);
                if (shading)
                    fill_color = Colors::shadeColor(fill_color, pmetric, mRemoteControl.model()->dem());
            }
            painter.fillRect(r, fill_color);
        }
    }
    painter.setPen(Qt::lightGray);
    painter.drawRect(total_rect);

    // update ruler
    if (object.view_type>=10) {
        QStringList labels;
        for (int i=0;i<=max_value;++i)
            labels.append(object.layered->labelvalue(i,object.layer_id));
        mRulerColors->setFactorLabels(labels);
    }
    mRulerColors->setPalette(object.view_type, object.cur_min_value, object.cur_max_value); // ruler

}

// paint the values of the MapGrid
void MainWindow::paintMapGrid(QPainter &painter,
                              MapGrid *map_grid, const FloatGrid *float_grid, const Grid<double> *double_grid,
                              const GridViewType view_type,
                              double min_val, double max_val,
                              bool shading)
{
    // clear background
    painter.fillRect(ui->PaintWidget->rect(), Qt::white);
    const Grid<int> *int_grid = nullptr;

    int sx, sy;
    QRect total_rect;

    if (map_grid) {
        int_grid = &map_grid->grid();
        sx = int_grid->sizeX();
        sy = int_grid->sizeY();
        total_rect = vp.toScreen(int_grid->metricRect());
    } else {

        if (!float_grid && !double_grid)
            return;
        if (float_grid) {
            sx = float_grid->sizeX();
            sy = float_grid->sizeY();
            total_rect = vp.toScreen(float_grid->metricRect());
        } else {
            sx = double_grid->sizeX();
            sy = double_grid->sizeY();
            total_rect = vp.toScreen(double_grid->metricRect());

        }
    }

    // paint the lower-res-grid;
    int ix,iy;
    double value;
    QRect r;
    QColor fill_color;

    if (view_type<10)
        mRulerColors->setPalette(view_type,min_val, max_val); // ruler
    else
        mRulerColors->setPalette(view_type, 0, max_val); // ruler

    bool reverse = view_type == GridViewRainbowReverse || view_type == GridViewGrayReverse;
    bool black_white = view_type == GridViewGray || view_type == GridViewGrayReverse;
    QPointF world;
    for (iy=0;iy<sy;iy++) {
        for (ix=0;ix<sx;ix++) {
            QPoint p(ix,iy);
            if (int_grid){
                value = int_grid->constValueAtIndex(p);
                r = vp.toScreen(int_grid->cellRect(p));
                world = int_grid->cellCenterPoint(p);
            } else if (float_grid) {
                value = float_grid->constValueAtIndex(p);
                r = vp.toScreen(float_grid->cellRect(p));
                world = float_grid->cellCenterPoint(p);
            } else {
                value = double_grid->constValueAtIndex(p);
                r = vp.toScreen(double_grid->cellRect(p));
                world = double_grid->cellCenterPoint(p);
            }
            fill_color = view_type<10? Colors::colorFromValue(value, min_val, max_val, reverse,black_white) : Colors::colorFromPalette(value, view_type);
            if (shading) {
                fill_color = Colors::shadeColor(fill_color, world, mRemoteControl.model()->dem()).rgb();
            }

            painter.fillRect(r, fill_color);
        }
    }

    // draw rectangle around the grid
    painter.setPen(Qt::lightGray);
    painter.drawRect(total_rect);


}

void MainWindow::repaintArea(QPainter &painter)
{
    if (!mRemoteControl.isBusy())  {
        paintFON(painter, ui->PaintWidget->rect());
        // fix viewpoint
        vp.setScreenRect(ui->PaintWidget->rect());
        mRulerColors->setScale(vp.pixelToMeter(1));
    } else {
        qDebug() << "mainwindow::repaint: skipped (busy)" << QDateTime::currentMSecsSinceEpoch();
    }
}


void MainWindow::on_visFon_toggled() { ui->PaintWidget->update(); }
void MainWindow::on_visDomGrid_toggled() { on_visFon_toggled(); }
void MainWindow::on_visImpact_toggled() { on_visFon_toggled(); }
bool wantDrag=false;
void MainWindow::mouseClick(const QPoint& pos)
{
    if (!mRemoteControl.canRun())
        return;

    QPointF coord=vp.toWorld(pos);
    //qDebug() << "to world:" << coord;
    wantDrag = false;
    ui->PaintWidget->setCursor(Qt::CrossCursor);
    Model *model = mRemoteControl.model();
    ResourceUnit *ru = model->ru(coord);
    // find adjactent tree

    // test ressource units...
    if (ui->visResourceUnits->isChecked()) {
        if (!ru) return;
        showResourceUnitDetails(ru);
        return;
    }

    // test for ABE grid
    if (ui->visOtherGrid->isChecked()) {
        if (showABEDetails(coord))
            return;
    }
    //qDebug() << "coord:" << coord << "RU:"<< ru << "ru-rect:" << ru->boundingBox();
    if (!ru)
        return;

    ui->treeChange->setProperty("tree",0);
    QVector<Tree> &mTrees =  ru->trees();
    QVector<Tree>::iterator tit;
    Tree *closestTree=0;
    double min_distance = 100000000, current_dist;
    for (tit=mTrees.begin(); tit!=mTrees.end(); ++tit) {
        current_dist = distance(tit->position(),coord);
        if (current_dist<min_distance) {
            closestTree = &(*tit);
            min_distance = current_dist;
        }
    }
    if (min_distance<5 && closestTree) {
        Tree *p = closestTree;
        //qDebug() << "found!" << tit->id() << "at" << tit->position()<<"value"<<p->lightResourceIndex();
        //qDebug() <<p->dump();
        showTreeDetails(p);

        ui->treeChange->setProperty("tree", qVariantFromValue((void*)p));
        ui->treeDbh->setValue(p->dbh());
        ui->treeHeight->setValue(p->height());
        ui->treePosX->setValue(p->position().x());
        ui->treePosY->setValue(p->position().y());
        ui->treeImpact->setText(QString("#:%1 - %2").arg(p->id()).arg(p->lightResourceIndex(),5));
        wantDrag=true;
        ui->PaintWidget->setCursor(Qt::SizeAllCursor);
        ui->PaintWidget->update();
    }

}

void MainWindow::showResourceUnitDetails(const ResourceUnit *ru)
{
    ui->dataTree->clear();
    RUWrapper ruw;
    ruw.setResourceUnit(ru);
    const QStringList &names = ruw.getVariablesList();
    QList<QTreeWidgetItem *> items;
    foreach(QString name, names) {
        items.append(new QTreeWidgetItem(QStringList()<<name<<QString::number(ruw.valueByName(name)) ));
    }
    // add special values (strings)
    if (ru->climate())
        items.append(new QTreeWidgetItem(QStringList()<<"climate"<<ru->climate()->name() ));
    // add species value: SVD state
    if (GlobalSettings::instance()->model()->svdStates())
        items.append(new QTreeWidgetItem(QStringList()<<"state"<<GlobalSettings::instance()->model()->svdStates()->stateLabel(ru->svdStateId()) ));

    QList<QPair<QString, QVariant> > dbgdata = GlobalSettings::instance()->debugValues(-ru->index()); // hack: use negative values for resource units

    QList<QPair<QString, QVariant> >::const_iterator i = dbgdata.constBegin();
    while (i != dbgdata.constEnd()) {
        //cout << i.key() << ": " << i.value() << endl;
        items.append(new QTreeWidgetItem(QStringList()
                                         << (*i).first
                                         << (*i).second.toString()) );
        ++i;
    }
    ui->dataTree->addTopLevelItems(items);
}

bool MainWindow::showABEDetails(const QPointF &coord)
{
    if (!mRemoteControl.canRun()) return false;
    if (mPaintNext.layered) {
        if (mPaintNext.layered->onClick(coord))
            return true;
    }
    QStringList list;

    if (mPaintNext.what == PaintObject::PaintHandledObject && mPaintNext.handler!=nullptr) {
        list = mRemoteControl.evaluateClick(mPaintNext.handler, coord, mPaintNext.name);
    } else {
        if (!mPaintNext.layered || !mRemoteControl.model()->ABEngine()) return false;
        QString grid_name = mPaintNext.name;
        list = mRemoteControl.model()->ABEngine()->evaluateClick(coord, grid_name);
    }

    ui->dataTree->clear();
    QList<QTreeWidgetItem *> items;
    QStack<QTreeWidgetItem*> stack;
    stack.push(0);
    foreach (QString s, list) {
        QStringList elem = s.split(":");
        if (s=="-")
            stack.push(items.back());
        else if (s=="/-")
            stack.pop();
        else  {
            items.append( new QTreeWidgetItem(stack.last(), elem) );
        }
    }
    ui->dataTree->addTopLevelItems(items);
    return true; // handled



}


void MainWindow::showTreeDetails(Tree *tree)
{
    ui->dataTree->clear();
    TreeWrapper tw;
    tw.setTree(tree);
    const QStringList &names = tw.getVariablesList();
    QList<QTreeWidgetItem *> items;
    foreach(QString name, names) {
        if (name=="species")
            items.append(new QTreeWidgetItem(QStringList() << name << mRemoteControl.model()->speciesSet()->species(tw.valueByName(name))->id() ));
        else
            items.append(new QTreeWidgetItem(QStringList() << name << QString::number(tw.valueByName(name), 'f') ));
    }
    QList<QPair<QString, QVariant> > dbgdata = GlobalSettings::instance()->debugValues(tree->id());

    QList<QPair<QString, QVariant> >::const_iterator i = dbgdata.constBegin();
    while (i != dbgdata.constEnd()) {
        //cout << i.key() << ": " << i.value() << endl;
        items.append(new QTreeWidgetItem(QStringList()
                                         << (*i).first
                                         << (*i).second.toString()) );
        ++i;
    }
    ui->dataTree->addTopLevelItems(items);
}

void MainWindow::mouseMove(const QPoint& pos)
{

    if (!mRemoteControl.canRun() ) // || mPaintNext.what == PaintObject::PaintNothing
        return;
    FloatGrid *grid = mRemoteControl.model()->grid();
    QPointF p = vp.toWorld(pos);
    bool has_value = true;
    double value;
    if (grid->coordValid(p)) {
        QString location=QString("%1 / %2").arg(p.x()).arg(p.y());
        if (ui->visOtherGrid->isChecked()) {
            switch (mPaintNext.what) {
            case PaintObject::PaintFloatGrid:
                value = mPaintNext.float_grid->isEmpty()?0: mPaintNext.float_grid->constValueAt(p);
                break;
            case PaintObject::PaintMapGrid:
                value = mPaintNext.map_grid->grid().constValueAt(p);
                break;
            case PaintObject::PaintLayers:
                value = mPaintNext.layered->value(p, mPaintNext.layer_id);
                if (mPaintNext.view_type>=10) {// classes
                    location += QString("\n %1").arg(mPaintNext.layered->labelvalue(value, mPaintNext.layer_id));
                    ui->fonValue->setText(location);
                    return;
                }

                break;
            case PaintObject::PaintHandledObject:
                value = mRemoteControl.valueAtHandledGrid(mPaintNext.handler, p, mPaintNext.layer_id);
                break;

            default: has_value = false;
            }
            if (has_value) {
                location += QString("\n %1").arg(value);
                ui->fonValue->setText(location);
                return;
            }
        }

        if (ui->visFon->isChecked() || ui->visImpact->isChecked()) {
            if (mPaintNext.what == PaintObject::PaintFloatGrid && mPaintNext.float_grid)
                location += QString("\n %1").arg(mPaintNext.float_grid->constValueAt(p));
            else
                location += QString("\n %1").arg((*grid).valueAt(p));
        }
        if( ui->visDomGrid->isChecked()) {
            HeightGridValue &hgv = (*mRemoteControl.model()->heightGrid()).valueAt(p);
            float value = ui->visDomHeightStem->isChecked() ? hgv.stemHeight() : hgv.height;
            location += QString("\n %1m (N=%2)").arg(value).arg(hgv.count());
        }
        if( ui->visRegeneration->isChecked() && !mRegenerationGrid.isEmpty())
            location += QString("\n %1").arg(mRegenerationGrid.valueAt(p));
        if (ui->visSeeds->isChecked() && ui->speciesFilterBox->currentIndex()>-1) {
            Species *s=GlobalSettings::instance()->model()->speciesSet()->species(ui->speciesFilterBox->itemData(ui->speciesFilterBox->currentIndex()).toString());
            if (s && s->seedDispersal())
                location += QString("\n %1").arg(s->seedDispersal()->seedMap().constValueAt(p));
        }

        ui->fonValue->setText(location);
    }
}

void MainWindow::mouseWheel(const QPoint& pos, int steps)
{
    //qDebug() << "mouse-wheel" << steps;
    vp.zoomTo(pos, qMax(1-(2*steps/10.),0.2));
    ui->PaintWidget->update();
}

void MainWindow::executeJS(QString code)
{
    // execute Javascript code
    LogToWindow l; // force UI logging
    try {

        QString result = ScriptGlobal::executeScript(code);
        if (!result.isEmpty()) {
            qDebug() << result;
        }
    } catch(const IException &e) {
        Helper::msg(e.message());
    }

}

void MainWindow::mouseDrag(const QPoint& from, const QPoint &to, Qt::MouseButton button)
{
    qDebug() << "drag" << button;
    ui->PaintWidget->setCursor(Qt::CrossCursor);
    // move view area if not dedicately moving around a tree
    if (!wantDrag) {
        vp.moveTo(from, to);
        ui->PaintWidget->update();
        return;
    }
    wantDrag = false;
    qDebug() << "drag from" << from << "to" << to;
    Tree *t = reinterpret_cast<Tree*>( ui->treeChange->property("tree").toLongLong() );
    if (!t)
        return;
    QPointF pos = vp.toWorld(to);
    // calculate new position...
    t->setPosition(pos);
    readwriteCycle();
    ui->PaintWidget->update();
}



void MainWindow::on_actionEdit_XML_settings_triggered()
{
    ui->editStack->setCurrentIndex(0);
    ui->PaintWidget->update();
}

QMutex mutex_yearSimulated;
void MainWindow::yearSimulated(int year)
{
    QMutexLocker mutex_locker(&mutex_yearSimulated);
    checkModelState();
    ui->modelRunProgress->setValue(year);
    labelMessage(QString("Running.... year %1 of %2. %3").arg(year).arg(mRemoteControl.totalYears()).arg(mRemoteControl.timeString()));
    ui->treeChange->setProperty("tree",0);
    ui->PaintWidget->update();
    //QApplication::processEvents();
}

void MainWindow::modelFinished(QString errorMessage)
{
    if (!errorMessage.isEmpty()) {
        Helper::msg(errorMessage);
        labelMessage("Error!");
        qDebug() << "Error:" << errorMessage;
    } else {
        qDebug() << "Finished!";
        labelMessage("Finished!!");
    }

    checkModelState();
    if (windowTitle().contains("batch")) {
        // we are in automatic batch mode.
        // we should therefore close down the application.
        if (!errorMessage.isEmpty())
            batchLog(QString("error: %1").arg(errorMessage));
        batchLog(QString("%1 Finished!!! shutting down...").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

        qDebug() << "****************************";
        qDebug() << "Finished automated model run: " << errorMessage;
        qDebug() << "****************************";

        close(); // shut down the application....

    }
}

/// creates the iLand model
void MainWindow::setupModel()
{
    //recent file menu
    recentFileMenu();

    // load project xml file to global xml settings structure
    mRemoteControl.setFileName(ui->initFileName->text());
    //GlobalSettings::instance()->loadProjectFile(ui->initFileName->text());
    labelMessage("Creating model...");

    // setup logging
    setupFileLogging(true);

    QTimer update_label;
    connect(&update_label, SIGNAL(timeout()), this, SLOT(updateLabel()));
    update_label.start(100);


    // create the model
    mRemoteControl.create();
    if (!mRemoteControl.canRun())
        return;

    Model *model = mRemoteControl.model();
    if (model && model->isSetup()) {
        // set viewport of paintwidget
        vp = Viewport(model->grid()->metricRect(), ui->PaintWidget->rect());
        // dominance grid is the default view after model start
        ui->visDomGrid->setChecked(true);
        //ui->PaintWidget->update();
    }
    ui->treeChange->setProperty("tree",0);

    // setup dynamic output
    QString dout = GlobalSettings::instance()->settings().value("output.dynamic.columns");
    mRemoteControl.setupDynamicOutput(dout);
    mRemoteControl.setDynamicOutputEnabled(GlobalSettings::instance()->settings().valueBool("output.dynamic.enabled",false));

    ui->modelRunProgress->setValue(0);
    QSettings().setValue("project/lastxmlfile", ui->initFileName->text());
    // magic debug output number
    GlobalSettings::instance()->setDebugOutput((int) GlobalSettings::instance()->settings().valueDouble("system.settings.debugOutput"));

    // populate the tree species filter list
    ui->speciesFilterBox->clear();
    ui->speciesFilterBox->addItem("<all species>", "");
    QList<const Species*> list = mRemoteControl.availableSpecies();
    for (int i=0;i<list.size();++i)
        ui->speciesFilterBox->addItem(list[i]->name(), list[i]->id());

    // retrieve the active management script file
    if (mRemoteControl.model()->management())
        ui->scriptActiveScriptFile->setText(QString("%1").arg(mRemoteControl.model()->management()->scriptFile()));
    if (!mRemoteControl.loadedJavascriptFile().isEmpty())
        ui->scriptActiveScriptFile->setText(QString("%1").arg(mRemoteControl.loadedJavascriptFile()));
    labelMessage("Model created. Ready to run.");
    checkModelState();

}



void MainWindow::on_pbSetAsDebug_clicked()
{
    Tree *t = reinterpret_cast<Tree *>( ui->treeChange->property("tree").toLongLong() );
    if (!t)
        return;
    t->enableDebugging();

}

void MainWindow::on_openFile_clicked()
{
    QString fileName = Helper::fileDialog("select XML-project file", ui->initFileName->text(), "*.xml",this);
    if (fileName.isEmpty())
        return;
    ui->initFileName->setText(fileName);
    QString xmlFile = Helper::loadTextFile(ui->initFileName->text());
    ui->iniEdit->setPlainText(xmlFile);
    checkModelState();
}

void MainWindow::on_actionTreelist_triggered()
{
    QApplication::clipboard()->setText(dumpTreelist());
    qDebug() << "treelist copied to clipboard.";
}

void MainWindow::on_actionFON_grid_triggered()
{
    //if (!mRemoteControl.isRunning()) return;
    QString gr = gridToString(*mRemoteControl.model()->grid());
    QApplication::clipboard()->setText(gr);
    qDebug() << "grid copied to clipboard.";
}


void MainWindow::on_actionModelCreate_triggered()
{
    // create model
    setupModel();
    checkModelState();
}

void MainWindow::on_actionModelDestroy_triggered()
{
    mPaintNext.what = PaintObject::PaintNothing;
    mRemoteControl.destroy();
    mRegenerationGrid.clear();
    checkModelState();
}

void MainWindow::on_actionModelRun_triggered()
{
    if (!mRemoteControl.canRun())
        return;
    QString msg = QString("How many years to run?\nCurrent year: %1.").arg(mRemoteControl.currentYear());
    bool ok;
    int count = QInputDialog::getInt(this, "input value",
                                     msg, 10, 0, 100000, 10, &ok);
    if (!ok)
        return;
    QTimer update_label;
    connect(&update_label, SIGNAL(timeout()), this, SLOT(updateLabel()));
    update_label.start(100);

    count = count + mRemoteControl.currentYear();
    ui->treeChange->setProperty("tree",0);
    ui->modelRunProgress->setMaximum(count-1);
    mRemoteControl.run(count);
    GlobalSettings::instance()->executeJSFunction("onFinish");

}

void MainWindow::on_actionRun_one_year_triggered()
{
    if (!mRemoteControl.canRun())
        return;
    ui->treeChange->setProperty("tree",0);
    mRemoteControl.runYear();
    GlobalSettings::instance()->outputManager()->save(); // save output tables when stepping single year by year
    labelMessage(QString("Simulated a single year. year %1.").arg(mRemoteControl.currentYear()));

    ui->PaintWidget->update();
    checkModelState();
}

void MainWindow::on_actionReload_triggered()
{
    if (!mRemoteControl.canDestroy())
        return;
    mPaintNext.what = PaintObject::PaintNothing;
    mRemoteControl.destroy();
    mRegenerationGrid.clear();
    setupModel();
}

void MainWindow::on_actionPause_triggered()
{
    mRemoteControl.pause();
    if (!mRemoteControl.isRunning())
        labelMessage("Model execution paused...");

    checkModelState();

    if (!mRemoteControl.isPaused())
        mRemoteControl.continueRun();
}

void MainWindow::on_actionStop_triggered()
{
    mRemoteControl.cancel();
    labelMessage("Model stopped.");
}

void MainWindow::on_actionTree_Partition_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dTreePartition, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";
}

void MainWindow::on_action_debugSapling_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dSaplingGrowth, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";

}

void MainWindow::on_actionTree_Growth_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dTreeGrowth, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";
}

void MainWindow::on_actionTree_NPP_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dTreeNPP, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";
}

void MainWindow::on_actionWater_Output_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dWaterCycle, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";
}
void MainWindow::on_actionDaily_responses_Output_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dDailyResponses, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";
}

void MainWindow::on_action_debugEstablishment_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dEstablishment, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";
}

void MainWindow::on_actionSnag_Dynamics_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dCarbonCycle, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";

}

void MainWindow::on_actionPerformance_triggered()
{
    QStringList result = GlobalSettings::instance()->debugDataTable(GlobalSettings::dPerformance, ";");
    QApplication::clipboard()->setText(result.join("\n"));
    qDebug() << "copied" <<  result.count() << "lines of debug data to clipboard.";

}

QImage MainWindow::screenshot()
{
    return ui->PaintWidget->drawImage();
}

/// set the viewport of the main viewing window
/// @p center_point is the point to zoom to (world coordinates), and @p scael_px_per_m is the
/// pixel/m scaling.
void MainWindow::setViewport(QPointF center_point, double scale_px_per_m)
{
    if (scale_px_per_m>0.)
        vp.setViewPoint(center_point, scale_px_per_m);
    else
        vp.zoomToAll();

    //    double current_px = vp.pixelToMeter(1); // number of meters covered by one pixel
    //    if (current_px==0)
    //        return;

    //    vp.setCenterPoint(center_point);
    //    QPoint screen = vp.toScreen(center_point); // screen coordinates of the target point
    //    double target_scale = scale_px_per_m / current_px;

    //    vp.zoomTo(screen, target_scale);
    ui->PaintWidget->update();
    QCoreApplication::processEvents();
}

void MainWindow::setUIshortcuts(QVariantMap shortcuts)
{
    if (shortcuts.isEmpty()) {
        ui->lJSShortcuts->setText("(no shortcuts defined)");
        return;
    }
    QString msg = "<html><head/><body><p>Javascript shortcuts<br>";
    QVariantMap::const_iterator i;
    for (i = shortcuts.constBegin(); i != shortcuts.constEnd(); ++i) {
        QString line = QString("<a href =\"%1\"><span style=\" text-decoration: underline; color:#0000ff;\">%1</span></a>: %2<br>").arg(i.key(), i.value().toString());
        msg += line;
    }
    msg += "</body></html>";
    //qDebug() << msg;

    ui->lJSShortcuts->setText(msg);
    ui->lJSShortcuts->setTextInteractionFlags(Qt::TextBrowserInteraction);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}


void MainWindow::on_actionImageToClipboard_triggered()
{
    //QClipboard *clipboard = QApplication::clipboard();
    QImage my_img = screenshot();
    // my_img.convertToFormat(QImage::Format_RGB32);
    //clipboard->setImage( my_img, QClipboard::Clipboard );
    QString pth = GlobalSettings::instance()->path("screenshot.png", "temp");
    if (!QDir(pth).exists())
        pth= GlobalSettings::instance()->path("screenshot.png");
    screenshot().save(pth);
    qDebug() << "copied image to clipboard. Saved the image also to: " << pth;
    my_img.load(pth);
    QApplication::clipboard()->setImage(my_img);

}


void MainWindow::on_actionSelect_Data_Types_triggered()
{
    int value = GlobalSettings::instance()->currentDebugOutput();
    int newvalue = QInputDialog::getInt(this, "QInputDialog::getText()",
                                        "Enter code for desired outputs: add\n" \
                                        "1 ... Tree NPP\n" \
                                        "2 ... Tree partition\n" \
                                        "4 ... Tree growth (dbh,h)\n" \
                                        "8 ... Standlevel GPP\n" \
                                        "16...Water Cycle\n" \
                                        "32...Daily responses\n" \
                                        "64...Establishment\n" \
                                        "128...Sapling growth\n" \
                                        "256...Carbon cycle\n" \
                                        "512...Performance\n"
                                        "(e.g.: 5 = NPP + tree growth) or 0 for no debug outputs.", value);
    GlobalSettings::instance()->setDebugOutput(newvalue);
}




// Expression test
void MainWindow::on_pbCalculateExpression_clicked()
{
    QString expr_text=ui->expressionText->text();
    QString expr_filter=ui->expressionFilter->text();
    if (expr_text == "test") {
        on_actionTest_triggered();
        return;
    }
    if (expr_filter.isEmpty())
        expr_filter = "1"; // a constant true expression
    TreeWrapper wrapper;
    Expression expr(expr_text, &wrapper);
    Expression filter(expr_filter, &wrapper);
    AllTreeIterator at(GlobalSettings::instance()->model());
    int totalcount=0;
    QVector<double> datavector;
    try {

        while (Tree *tree=at.next()) {
            wrapper.setTree(tree);
            if (filter.execute()) {
                datavector << expr.execute();
            }
            totalcount++;
        }
    } catch (IException &e) {
        Helper::msg(e.message());
    }
    StatData stats(datavector);
    qDebug() << "Expression:" << expr_text << "filtered" << datavector.count() << "of" << totalcount;
    qDebug() << "sum:" << stats.sum() << "min" << stats.min() << "max" << stats.max() << "average" << stats.mean();
    qDebug() << "P25" << stats.percentile25() << "median" << stats.median() << "P75" << stats.percentile75() << "P90" << stats.percentile(90);

    //qDebug() << "Expression:" << expr_text << "results: count of total: " << count << "/" << totalcount
    //        << "sum:" << sum  << "average:" << (count>0?sum/double(count):0.) << "minval:" << minval << "maxval:" << maxval;
    // add to history
    if (!ui->expressionHistory->currentItem() || ui->expressionHistory->currentItem()->text() != expr_text) {
        ui->expressionHistory->insertItem(0, expr_text);
        ui->expressionHistory->setCurrentRow(0);
    }
}

void MainWindow::on_pbExecExpression_clicked()
{
    // just repaint...
    ui->PaintWidget->update();
}

void MainWindow::on_actionDynamic_Output_triggered()
{
    QApplication::clipboard()->setText(mRemoteControl.dynamicOutput());
    qDebug() << "copied dynamic output to clipboard";
}

void MainWindow::on_actionShow_Debug_Messages_triggered(bool checked)
{
    // enable/disble debug messages
    showDebugMessages=checked;
}

void MainWindow::on_reloadJavaScript_clicked()
{
    if (!GlobalSettings::instance()->model())
        MSGRETURN("no model available.");

    ScriptGlobal::loadScript(ui->scriptActiveScriptFile->text());
}

void MainWindow::on_selectJavaScript_clicked()
{
    if (!GlobalSettings::instance()->model())
        return;
    QString fileName = Helper::fileDialog("select a Javascript file:");
    if (fileName.isEmpty())
        return;
    ScriptGlobal::loadScript(fileName);

    ui->scriptActiveScriptFile->setText(QString("%1").arg(fileName));
    qDebug() << "loaded Javascript file" << fileName;

}

void MainWindow::on_scriptCommand_returnPressed()
{
    LogToWindow l; // force UI logging
    QString command = ui->scriptCommand->text();
    if (ui->scriptCommandHistory->currentText() != command) {
        ui->scriptCommandHistory->insertItem(0, command);
        ui->scriptCommandHistory->setCurrentIndex(0);
    }

    qDebug() << "executing" << command;
    try {

        QString result = ScriptGlobal::executeScript(command);
        if (!result.isEmpty()) {
            qDebug() << result;
        }
    } catch(const IException &e) {
        Helper::msg(e.message());
    }
}





void MainWindow::on_actionOutput_table_description_triggered()
{
    QString txt = GlobalSettings::instance()->outputManager()->wikiFormat();
    QApplication::clipboard()->setText(txt);
    qDebug() << "Description copied to clipboard!";
}

void MainWindow::on_actionTimers_triggered()
{
    LogToWindow l;
    DebugTimer::printAllTimers();
}

void MainWindow::on_actionOnline_ressources_triggered()
{
    QDesktopServices::openUrl(QUrl("https://iland-model.org/"));
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog;
    dialog.exec();
}


/* Logging and filtering of logging */
void MainWindow::on_pbLogToClipboard_clicked()
{
    // copy content of log window to clipboard
    QApplication::clipboard()->setText(ui->logOutput->toPlainText());

}

void MainWindow::on_pbLogClearText_clicked()
{
    ui->logOutput->clear();
    ui->logOutput->setProperty("fullText","");
    ui->pbLogFilterClear->setEnabled(false);
}

void MainWindow::on_pbFilterExecute_clicked()
{
    QStringList lines;
    QString search_for = ui->logFilterExpression->text();
    if (search_for.isEmpty())
        return;
    QString full_content;
    if (ui->logOutput->property("fullText").toString().isEmpty()) {
        full_content = ui->logOutput->toPlainText();
        ui->logOutput->setProperty("fullText",full_content);
    } else
        full_content = ui->logOutput->property("fullText").toString();
    QStringList debugLines = full_content.split("\n");
    int i=0;
    foreach(const QString &line, debugLines) {
        i++; // line counter
        if (line.contains(search_for))
            lines.push_back(QString("%1: %2").arg(i).arg(line) );
    }
    if (lines.count()>0) {
        ui->logOutput->setPlainText(lines.join("\n"));
    } else {
        ui->logOutput->setPlainText("Search term not found!");
    }
    ui->pbLogFilterClear->setEnabled(true);
}

void MainWindow::on_pbLogFilterClear_clicked()
{
    QString text = ui->logOutput->property("fullText").toString();
    if (text.isEmpty())
        return;
    //QString sel = ui->logOutput->textCursor().selectedText();
    //int line = sel.toInt();
    int line = atoi(ui->logOutput->textCursor().block().text().toLocal8Bit());
    ui->logOutput->setPlainText(text);
    ui->logOutput->setProperty("fullText","");
    //int bl = ui->logOutput->document()->findBlockByNumber(line).position();
    ui->logOutput->setTextCursor(QTextCursor(ui->logOutput->document()->findBlockByNumber(line)));
    ui->logOutput->ensureCursorVisible();
    ui->pbLogFilterClear->setEnabled(false);

}

void MainWindow::on_actionClearDebugOutput_triggered()
{
    GlobalSettings::instance()->clearDebugLists();
}


void MainWindow::on_actionDebug_triggered()
{
    //
    QObject *o = QObject::sender();
    int level = o->property("logLevel").toInt();
    ui->actionDebug->setChecked( level == 0);
    ui->actionInfo->setChecked( level == 1);
    ui->actionWarning->setChecked( level == 2);
    ui->actionError->setChecked( level == 3);

    setLogLevel(level);
}



void MainWindow::on_scriptCommandHistory_currentIndexChanged(int index)
{
    if (index>=0)
        ui->scriptCommand->setText(ui->scriptCommandHistory->itemText(index));
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
    // javascript commands
    settings.beginWriteArray("javascriptCommands");
    int size = qMin(ui->scriptCommandHistory->count(), 15); // max 15 entries in the history
    for (int i=0;i<size; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("item", ui->scriptCommandHistory->itemText(i));
    }
    settings.endArray();
    settings.beginGroup("project");
    settings.setValue("lastxmlfile", ui->initFileName->text());
    settings.endGroup();
    //recent files menu qsettings registry save
    settings.beginGroup("recent_files");
    for(int i = 0;i < mRecentFileList.size();i++){
        settings.setValue(QString("file-%1").arg(i),mRecentFileList[i]);
    }
    settings.endGroup();
    settings.setValue("javascript", ui->scriptCode->toPlainText());
}
void MainWindow::readSettings()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QCoreApplication::setOrganizationName("iLand");
    QCoreApplication::setOrganizationDomain("iland-model.org");
    QCoreApplication::setApplicationName("iLand");
    QSettings settings;
    qDebug() << "reading settings from" << settings.fileName();

    // window state and
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/windowState").toByteArray());

    // read javascript commands
    int size = settings.beginReadArray("javascriptCommands");
    for (int i=0;i<size; ++i) {
        settings.setArrayIndex(i);
        ui->scriptCommandHistory->addItem(settings.value("item").toString());
    }
    settings.endArray();
    //recent files menu qsettings registry load
    settings.beginGroup("recent_files");
    for(int i = 0;i < settings.childKeys().size();i++){
        //resize(settings.value("size", QSize(400, 400)).toSize());
        mRecentFileList.append(settings.value(QString("file-%1").arg(i)).toString());
    }
    for(int i = 0;i < ui->menuRecent_Files->actions().size();i++){
        if(i < mRecentFileList.size()){
            ui->menuRecent_Files->actions()[i]->setText(mRecentFileList[i]);
            connect(ui->menuRecent_Files->actions()[i],SIGNAL(triggered()),this,SLOT(menuRecent_Files()));
            ui->menuRecent_Files->actions()[i]->setVisible(true);
        }else{
            ui->menuRecent_Files->actions()[i]->setVisible(false);
        }
    }
    settings.endGroup();
    ui->scriptCode->setPlainText(settings.value("javascript").toString());
}



void MainWindow::on_actionTest_triggered()
{
    Tests t(this);
    int which = QInputDialog::getInt(this, "Which test",
                                     "which test?\n0: expression speed\n1: tree clear\n" \
                                     "2:kill trees\n3: climate\n4: multiple light automation\n" \
                                     "5: species response\n" \
                                     "6: watercycle\n" \
                                     "7: CSV File\n" \
                                     "8: Xml setters\n" \
                                     "9: random functions\n" \
                                     "10: seed dispersal.\n" \
                                     "11: multiple thread expression\n" \
                                     "12: linearized expressions\n" \
                                     "13: establishment\n" \
                                     "14: GridRunner\n" \
                                     "15: Soil (ICBM/2N)\n" \
                                     "16: load Map \n" \
                                     "17: test DEM \n" \
                                     "18: test fire module \n" \
                                     "19: test wind module\n" \
                                     "20: test rumple index\n" \
                                     "21: test FOME setup\n" \
                                     "22: test FOME step\n" \
                                     "23: test debug establishment\n" \
                                     "24: test grid special index hack",-1);
    switch (which) {
    case 0: t.speedOfExpression();break;
    case 1: t.clearTrees(); break;
    case 2: t.killTrees(); break;
    case 3: t.climate(); break;
    case 4: t.multipleLightRuns(GlobalSettings::instance()->path("automation.xml", "home")); break;
    case 5: t.climateResponse(); break;
    case 6: t.testWater(); break;
    case 7: t.testCSVFile(); break;
    case 8: t.testXml(); break;
    case 9: t.testRandom(); break;
    case 10: t.testSeedDispersal(); break;
    case 11: t.testMultithreadExecute(); break;
    case 12: t.testLinearExpressions(); break;
    case 13: t.testEstablishment(); break;
    case 14: t.testGridRunner(); break;
    case 15: t.testSoil(); break;
    case 16: t.testMap(); break;
    case 17: t.testDEM(); break;
    case 18: t.testFire(); break;
    case 19: t.testWind(); break;
    case 20: t.testRumple(); break;
    case 21: t.testFOMEsetup(); break;
    case 22: t.testFOMEstep(); break;
    case 23: t.testDbgEstablishment(); break;
    case 24: t.testGridIndexHack(); break;
    }

}

void MainWindow::on_pbReloadQml_clicked()
{
    //engine()->clearComponentCache();
    //setSource(source());
    if (!mRuler)
        return;
    mRuler->engine()->clearComponentCache();
    mRuler->setSource(mRuler->source());
}

void MainWindow::on_actionExit_triggered()
{
    if (Helper::question("Do you really want to quit?"))
        close();
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = Helper::fileDialog("select XML-project file", ui->initFileName->text(), "*.xml");
    if (fileName.isEmpty())
        return;
    ui->initFileName->setText(fileName);
    QString xmlFile = Helper::loadTextFile(ui->initFileName->text());
    ui->iniEdit->setPlainText(xmlFile);
    checkModelState();

}

void MainWindow::menuRecent_Files()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    if (action)
        ui->initFileName->setText(action->text());
}

void MainWindow::recentFileMenu(){
    if(mRecentFileList.size() > 9){
        mRecentFileList.removeAt(9);
    }
    if(mRecentFileList.contains(ui->initFileName->text())){
        mRecentFileList.removeAt(mRecentFileList.indexOf(ui->initFileName->text()));
    }
    mRecentFileList.prepend(ui->initFileName->text());

    for(int i = 0;i < ui->menuRecent_Files->actions().size();i++){
        if(i < mRecentFileList.size()){
            ui->menuRecent_Files->actions()[i]->setText(mRecentFileList[i]);
            connect(ui->menuRecent_Files->actions()[i],SIGNAL(triggered()),this,SLOT(menuRecent_Files()));
            ui->menuRecent_Files->actions()[i]->setVisible(true);
        }else{
            ui->menuRecent_Files->actions()[i]->setVisible(false);
        }
    }
}


void MainWindow::on_lJSShortcuts_linkActivated(const QString &link)
{
    qDebug() << "executing: " << link;
    try {

        qDebug() << ScriptGlobal::executeScript(link);

    } catch(const IException &e) {
        Helper::msg(e.message());
    }

}






void MainWindow::on_actionShow_full_extent_triggered()
{
    vp.zoomToAll();
    repaint();
    //ui->PaintWidget->update();
}

void MainWindow::on_actionRepaint_triggered()
{
    repaint();
    // ui->PaintWidget->update();
}

void MainWindow::on_checkXMLFile_clicked()
{
    SettingMetaData sm;
    sm.checkXMLFile(ui->initFileName->text());
}

void MainWindow::on_actionSave_regeneration_grid_triggered()
{
    if (!mRegenerationGrid.isEmpty()) {
        QString fileName = GlobalSettings::instance()->path("temp/regengrid.asc");
        QString result = gridToESRIRaster(mRegenerationGrid);
        Helper::saveToTextFile(fileName, result);
        qDebug() << "saved regeneration grid (current content) to " << fileName;

    }
}

void MainWindow::on_pbLoadTree_clicked()
{
    if (!mRemoteControl.canRun())
        return;

    QJSEngine *engine = GlobalSettings::instance()->scriptEngine();
    QStringList hideList = QStringList() << "Math" << "JSON" << "undefined" << "NaN" << "Infinity";

    QJSValue start;
    if (ui->lJSTree->text().isEmpty())
        start = engine->globalObject();
    else {
        start = engine->evaluate(ui->lJSTree->text());
        if (start.isError())
            return;
    }

    ui->JSTree->clear();
    QList<QTreeWidgetItem *> items;
    QStack<QTreeWidgetItem*> stack;
    QStack<QJSValueIterator*> iterators;
    QStack<QJSValue> js_obj;

    stack.push(nullptr);
    QStringList element;

    iterators.push_back(new QJSValueIterator(start));
    js_obj.push_back(start);
    int depth=0;
    do {
        QJSValueIterator *it = iterators.back();
        if (it->hasNext()) {
            it->next();
            if (js_obj.top().hasOwnProperty(it->name())) {
                bool hide=false;
                if (depth == 0 && it->value().isCallable() )
                    hide = true;
                if (depth==0 && hideList.contains(it->name()))
                    hide = true;
                if (!hide) {
                    element.clear();
                    QString oname = it->value().toString();
                    element << it->name() << oname;
                    items.append( new QTreeWidgetItem(stack.last(), element) );

                    if (it->value().isObject() && !it->value().isCallable() && depth<4) {
                        stack.push_back(items.back());
                        iterators.push_back(new QJSValueIterator(it->value()));
                        js_obj.push_back(it->value());
                        ++depth;
                    }
                }
            }


        } else {
            stack.pop(); --depth;
            QJSValueIterator *vi = iterators.pop();
            delete vi;
            js_obj.pop();
            if (stack.size()==0)
                break;
        }

    } while (true);


    ui->JSTree->addTopLevelItems(items);

}

void MainWindow::on_otherGridTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    if (!mRemoteControl.canRun())
        return;
    if (!current || current->data(0, Qt::UserRole+0).isNull())
        return;
    QString key = current->data(0, Qt::UserRole+0).toString();
    if (!mPaintList.contains(key))
        return;
    PaintObject &po = mPaintList[key];
    qDebug() << "clicked:" << key;

    mPaintNext = po;
    ui->visOtherGrid->setChecked(true); // to enable the right category
    repaint();
}

void MainWindow::on_dataTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    QString var_name = item->text(0);

    ui->lTreeExpr->setText( var_name );
    // refresh drawing
    ui->PaintWidget->update();
}
