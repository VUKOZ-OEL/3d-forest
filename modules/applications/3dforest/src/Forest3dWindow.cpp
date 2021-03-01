/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/**
    @file Forest3dWindow.cpp
*/

#include <Forest3dClipFilter.hpp>
#include <Forest3dDataSets.hpp>
#include <Forest3dLayers.hpp>
#include <Forest3dPluginFile.hpp>
#include <Forest3dPluginTool.hpp>
#include <Forest3dWindow.hpp>
#include <GLViewer.hpp>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPluginLoader>

const QString Forest3dWindow::APPLICATION_NAME = "3DForest";
const QString Forest3dWindow::APPLICATION_VERSION = "1.0";
static const char *FOREST3D_FILE_FILTER = "3DForest Project (*.json)";
#define FOREST3D_DOCK_MIN 80
#define FOREST3D_DOCK_MAX 500

Forest3dWindow::Forest3dWindow(QWidget *parent) : QMainWindow(parent)
{
    initializeWindow();
}

Forest3dWindow::~Forest3dWindow()
{
    killTimer(timerNewData_);
}

QSize Forest3dWindow::minimumSizeHint() const
{
    return QSize(320, 200);
}

QSize Forest3dWindow::sizeHint() const
{
    return QSize(800, 600);
}

void Forest3dWindow::initializeWindow()
{
    // Create
    createMenus();
    createViewer();
    createWindows();
    createPlugins();

    // Update
    updateProject();

    // Start timers
    timerNewData_ = startTimer(1000);
}

void Forest3dWindow::createViewer()
{
    viewer_ = new GLViewer(this);
    setCentralWidget(viewer_);
}

void Forest3dWindow::createMenus()
{
    // File
    QMenu *menuFile = menuBar()->addMenu(tr("File"));
    (void)menuFile->addAction(tr("New"),
                              this,
                              &Forest3dWindow::actionProjectNew);
    (void)menuFile->addAction(tr("Open..."),
                              this,
                              &Forest3dWindow::actionProjectOpen);
    (void)menuFile->addAction(tr("Save"),
                              this,
                              &Forest3dWindow::actionProjectSave);
    (void)menuFile->addAction(tr("Save As..."),
                              this,
                              &Forest3dWindow::actionProjectSaveAs);

    QAction *action;
    (void)menuFile->addSeparator();
    action = menuFile->addAction(tr("Add data set..."),
                                 this,
                                 &Forest3dWindow::actionProjectImport);
    action->setEnabled(false);
    action = menuFile->addAction(tr("Export As..."),
                                 this,
                                 &Forest3dWindow::actionProjectExportAs);
    action->setEnabled(false);

    (void)menuFile->addSeparator();
    (void)menuFile->addAction(tr("Exit"), this, &Forest3dWindow::close);

    // View
    QMenu *menuView = menuBar()->addMenu(tr("View"));
    QMenu *menuViewLayout = menuView->addMenu(tr("Layout"));
    (void)menuViewLayout->addAction(tr("Single"),
                                    this,
                                    &Forest3dWindow::actionViewLayoutSingle);
    (void)menuViewLayout->addAction(
        tr("Two Columns"),
        this,
        &Forest3dWindow::actionViewLayoutTwoColumns);

    // Tools
    menuTools_ = menuBar()->addMenu(tr("Tools"));

    // Windows
    menuWindows_ = menuBar()->addMenu(tr("Windows"));

    // Help
    QMenu *menuHelp = menuBar()->addMenu(tr("Help"));
    (void)menuHelp->addAction(tr("About"), this, &Forest3dWindow::actionAbout);
}

void Forest3dWindow::createWindows()
{
    // Create data sets window
    windowDataSets_ = new Forest3dDataSets(this);
    connect(windowDataSets_,
            SIGNAL(itemChangedCheckState(size_t, bool)),
            this,
            SLOT(actionDataSetVisible(size_t, bool)));

    QDockWidget *dockDataSets = new QDockWidget(tr("Data sets"), this);
    dockDataSets->setAllowedAreas(Qt::LeftDockWidgetArea |
                                  Qt::RightDockWidgetArea);
    dockDataSets->setMinimumWidth(FOREST3D_DOCK_MIN);
    dockDataSets->setMaximumWidth(FOREST3D_DOCK_MAX);
    dockDataSets->setWidget(windowDataSets_);
    addDockWidget(Qt::LeftDockWidgetArea, dockDataSets);

    // Create layers window
    windowLayers_ = new Forest3dLayers(this);
    connect(windowLayers_,
            SIGNAL(itemChangedCheckState(size_t, bool)),
            this,
            SLOT(actionLayerVisible(size_t, bool)));

    QDockWidget *dockLayers = new QDockWidget(tr("Layers"), this);
    dockLayers->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockLayers->setMinimumWidth(FOREST3D_DOCK_MIN);
    dockLayers->setMaximumWidth(FOREST3D_DOCK_MAX);
    dockLayers->setWidget(windowLayers_);
    addDockWidget(Qt::LeftDockWidgetArea, dockLayers);

    // Create clip filter window
    windowClipFilter_ = new Forest3dClipFilter(this);
    connect(windowClipFilter_,
            SIGNAL(filterChanged(const ClipFilter &)),
            this,
            SLOT(actionClipFilter(const ClipFilter &)));

    QDockWidget *dockClipFilter = new QDockWidget(tr("Clip filter"), this);
    dockClipFilter->setAllowedAreas(Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockClipFilter->setMinimumWidth(FOREST3D_DOCK_MIN);
    dockClipFilter->setMaximumWidth(FOREST3D_DOCK_MAX);
    dockClipFilter->setWidget(windowClipFilter_);
    addDockWidget(Qt::LeftDockWidgetArea, dockClipFilter);

    // Add dock widgets to Windows menu
    menuWindows_->addAction(dockClipFilter->toggleViewAction());
    menuWindows_->addAction(dockDataSets->toggleViewAction());
    menuWindows_->addAction(dockLayers->toggleViewAction());
}

void Forest3dWindow::createPlugins()
{
    // Process all files in the application "exe" directory
    QDir pluginsDir(QCoreApplication::applicationDirPath());
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entries)
    {
        // Try to load the file as a plugin
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();

        if (plugin)
        {
            // Detect and register various 3d Forest plugins

            // Tool
            Forest3dPluginTool *pluginToolInterface;
            pluginToolInterface = qobject_cast<Forest3dPluginTool *>(plugin);
            if (pluginToolInterface)
            {
                pluginsTool_.push_back(pluginToolInterface);
                (void)menuTools_->addAction(
                    pluginToolInterface->name(),
                    this,
                    &Forest3dWindow::actionPluginToolStart);
                continue;
            }

            // File
            Forest3dPluginFile *pluginFileInterface;
            pluginFileInterface = qobject_cast<Forest3dPluginFile *>(plugin);
            if (pluginFileInterface)
            {
                pluginsFile_.push_back(pluginFileInterface);
                continue;
            }
        }
    }
}

void Forest3dWindow::actionProjectNew()
{
    (void)projectClose();
}

void Forest3dWindow::actionProjectOpen()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open"),
                                            "",
                                            tr(FOREST3D_FILE_FILTER));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectOpen(fileName);
}

void Forest3dWindow::actionProjectSave()
{
    (void)projectSave();
}

void Forest3dWindow::actionProjectSaveAs()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
                                            tr("Save As"),
                                            "",
                                            tr(FOREST3D_FILE_FILTER));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectSave(fileName);
}

void Forest3dWindow::actionProjectImport()
{
}

void Forest3dWindow::actionProjectExportAs()
{
}

void Forest3dWindow::actionViewLayoutSingle()
{
    viewer_->setViewLayout(GLViewer::VIEW_LAYOUT_SINGLE);
}

void Forest3dWindow::actionViewLayoutTwoColumns()
{
    viewer_->setViewLayout(GLViewer::VIEW_LAYOUT_TWO_COLUMNS);
    updateViewer();
}

void Forest3dWindow::actionPluginToolStart()
{
    QObject *obj = sender();
    QAction *action = qobject_cast<QAction *>(obj);

    if (action)
    {
        // Lookup and run a plugin according to action text from plugin menu
        for (auto &it : pluginsTool_)
        {
            if (it->name() == action->text())
            {
                try
                {
                    it->compute(this, editor_);
                }
                catch (std::exception &e)
                {
                    showError(e.what());
                }
                catch (...)
                {
                    showError("Unknown");
                }
                break;
            }
        }
    }
}

void Forest3dWindow::actionDataSetVisible(size_t id, bool checked)
{
    editor_.setVisibleDataSet(id, checked);
    updateViewer();
}

void Forest3dWindow::actionLayerVisible(size_t id, bool checked)
{
    editor_.setVisibleLayer(id, checked);
    updateViewer();
}

void Forest3dWindow::actionClipFilter(const ClipFilter &clipFilter)
{
    editor_.setClipFilter(clipFilter);
    updateViewer();
}

void Forest3dWindow::actionAbout()
{
    QMessageBox::about(this,
                       tr("About 3D Forest, version ") +
                           Forest3dWindow::APPLICATION_VERSION,
                       tr("3D Forest is software for analysis of Lidar data"
                          " from forest environment.\n\n"
                          "Copyright 2020 VUKOZ\n"
                          "Blue Cat team and other authors\n"
                          "https://www.3dforest.eu/"));
}

bool Forest3dWindow::projectOpen(const QString &path)
{
    // Close the current project
    if (!projectClose())
    {
        return false;
    }

    // Open new project
    try
    {
        editor_.open(path.toStdString());
    }
    catch (std::exception &e)
    {
        showError(e.what());
        return false;
    }

    updateProject();

    return true; // Opened
}

bool Forest3dWindow::projectClose()
{
    // Save changes
    if (editor_.hasUnsavedChanges())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                                  QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        bool canClose = true;

        switch (ret)
        {
            case QMessageBox::Save:
                // Save was clicked
                canClose = projectSave();
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                canClose = false;
                break;
            default:
                // should never be reached
                Q_UNREACHABLE();
                break;
        }

        if (canClose == false)
        {
            return false;
        }
    }

    // Close
    editor_.close();
    updateProject();

    return true; // Closed
}

bool Forest3dWindow::projectSave(const QString &path)
{
    std::string writePath;

    if (path.isEmpty())
    {
        // Save
        writePath = editor_.project().path();
        if (writePath.empty())
        {
            // First time save
            QString fileName;
            fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"),
                                                    "",
                                                    tr(FOREST3D_FILE_FILTER));

            if (fileName.isEmpty())
            {
                return false;
            }
            writePath = fileName.toStdString();
        }
    }
    else
    {
        // Save As
        writePath = path.toStdString();
    }

    // Write
    try
    {
        editor_.write(writePath);
    }
    catch (std::exception &e)
    {
        showError(e.what());
        return false;
    }

    return true; // Saved
}

void Forest3dWindow::updateProject()
{
    windowDataSets_->updateEditor(editor_);
    windowLayers_->updateEditor(editor_);
    windowClipFilter_->updateEditor(editor_);
    updateViewer();
    updateWindowTitle(QString::fromStdString(editor_.project().path()));
}

void Forest3dWindow::updateViewer()
{
    viewer_->updateScene(&editor_);
}

void Forest3dWindow::showError(const char *message)
{
    (void)QMessageBox::critical(this, tr("Error"), message);
}

void Forest3dWindow::updateWindowTitle(const QString &path)
{
    if (path == "")
    {
        setWindowTitle(APPLICATION_NAME);
    }
    else
    {
        QString newtitle = APPLICATION_NAME + " - " + path;
        setWindowTitle(newtitle);
    }
}

void Forest3dWindow::timerEvent(QTimerEvent *event)
{
    (void)event;

    // TBD thread + wait condition instead of a timer
    editor_.updateView();
    updateViewer();
}

void Forest3dWindow::closeEvent(QCloseEvent *event)
{
    if (projectClose())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
