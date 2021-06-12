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
    @file WindowMain.cpp
*/

#include <PluginFile.hpp>
#include <PluginTool.hpp>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPluginLoader>
#include <QTextEdit>
#include <Time.hpp>
#include <WindowClipFilter.hpp>
#include <WindowDataSets.hpp>
#include <WindowLayers.hpp>
#include <WindowMain.hpp>
#include <WindowSettingsView.hpp>
#include <WindowViewports.hpp>
#include <iostream>

const QString WindowMain::APPLICATION_NAME = "3DForest";
const QString WindowMain::APPLICATION_VERSION = "1.0";
QTextEdit *WindowMain::log = nullptr;

static const char *WINDOW_MAIN_FILE_FILTER = "3DForest Project (*.json)";
#define WINDOW_MAIN_DOCK_MIN 80
#define WINDOW_MAIN_DOCK_MAX 500

WindowMain::WindowMain(QWidget *parent) : QMainWindow(parent)
{
    initializeWindow();
}

WindowMain::~WindowMain()
{
}

QSize WindowMain::minimumSizeHint() const
{
    return QSize(320, 200);
}

QSize WindowMain::sizeHint() const
{
    return QSize(1024, 768);
}

void WindowMain::initializeWindow()
{
    // Create
    createEditor();
    createMenus();
    createViewer();
    createWindows();
    createPlugins();

    // Update
    updateProject();
}

void WindowMain::createEditor()
{
    connect(&editor_,
            SIGNAL(renderRequested()),
            this,
            SLOT(actionEditorRender()));
}

void WindowMain::createViewer()
{
    windowViewports_ = new WindowViewports(this);
    connect(windowViewports_,
            SIGNAL(cameraChanged(size_t)),
            this,
            SLOT(actionCameraChanged(size_t)));

    setCentralWidget(windowViewports_);
}

void WindowMain::createMenus()
{
    // File
    QMenu *menuFile = menuBar()->addMenu(tr("File"));
    (void)menuFile->addAction(tr("New"), this, &WindowMain::actionProjectNew);
    (void)menuFile->addAction(tr("Open..."),
                              this,
                              &WindowMain::actionProjectOpen);
    (void)menuFile->addAction(tr("Save"), this, &WindowMain::actionProjectSave);
    (void)menuFile->addAction(tr("Save As..."),
                              this,
                              &WindowMain::actionProjectSaveAs);

    QAction *action;
    (void)menuFile->addSeparator();
    action = menuFile->addAction(tr("Add data set..."),
                                 this,
                                 &WindowMain::actionProjectImport);
    action->setEnabled(false);
    action = menuFile->addAction(tr("Export As..."),
                                 this,
                                 &WindowMain::actionProjectExportAs);
    action->setEnabled(false);

    (void)menuFile->addSeparator();
    (void)menuFile->addAction(tr("Exit"), this, &WindowMain::close);

    // View
    QMenu *menuView = menuBar()->addMenu(tr("View"));
    QMenu *menuViewCamera = menuView->addMenu(tr("Camera"));
    (void)menuViewCamera->addAction(tr("Orthographic"),
                                    this,
                                    &WindowMain::actionViewOrthographic);
    (void)menuViewCamera->addAction(tr("Perspective"),
                                    this,
                                    &WindowMain::actionViewPerspective);

    (void)menuViewCamera->addSeparator();
    (void)menuViewCamera->addAction(tr("Top"),
                                    this,
                                    &WindowMain::actionViewTop);
    (void)menuViewCamera->addAction(tr("Front"),
                                    this,
                                    &WindowMain::actionViewFront);
    (void)menuViewCamera->addAction(tr("Left"),
                                    this,
                                    &WindowMain::actionViewLeft);
    (void)menuViewCamera->addAction(tr("3D"), this, &WindowMain::actionView3d);

    (void)menuViewCamera->addSeparator();
    (void)menuViewCamera->addAction(tr("Reset distance"),
                                    this,
                                    &WindowMain::actionViewResetDistance);
    (void)menuViewCamera->addAction(tr("Reset center"),
                                    this,
                                    &WindowMain::actionViewResetCenter);

    QMenu *menuViewLayout = menuView->addMenu(tr("Layout"));
    (void)menuViewLayout->addAction(tr("Single"),
                                    this,
                                    &WindowMain::actionViewLayoutSingle);
    (void)menuViewLayout->addAction(tr("Two Columns"),
                                    this,
                                    &WindowMain::actionViewLayout2Columns);
    (void)menuViewLayout->addAction(tr("Three Rows Right"),
                                    this,
                                    &WindowMain::actionViewLayout3RowsRight);

    // Tools
    menuTools_ = menuBar()->addMenu(tr("Tools"));

    // Windows
    menuWindows_ = menuBar()->addMenu(tr("Windows"));

    // Help
    QMenu *menuHelp = menuBar()->addMenu(tr("Help"));
    (void)menuHelp->addAction(tr("About"), this, &WindowMain::actionAbout);
}

void WindowMain::createWindows()
{
    // Create data sets window
    windowDataSets_ = new WindowDataSets(this);
    connect(windowDataSets_,
            SIGNAL(itemChangedCheckState(size_t, bool)),
            this,
            SLOT(actionDataSetVisible(size_t, bool)));

    QDockWidget *dockDataSets = new QDockWidget(tr("Data Sets"), this);
    dockDataSets->setAllowedAreas(Qt::LeftDockWidgetArea |
                                  Qt::RightDockWidgetArea);
    dockDataSets->setMinimumWidth(WINDOW_MAIN_DOCK_MIN);
    dockDataSets->setMaximumWidth(WINDOW_MAIN_DOCK_MAX);
    dockDataSets->setWidget(windowDataSets_);
    addDockWidget(Qt::LeftDockWidgetArea, dockDataSets);

    // Create layers window
    windowLayers_ = new WindowLayers(this);
    connect(windowLayers_,
            SIGNAL(itemChangedCheckState(size_t, bool)),
            this,
            SLOT(actionLayerVisible(size_t, bool)));

    QDockWidget *dockLayers = new QDockWidget(tr("Layers"), this);
    dockLayers->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockLayers->setMinimumWidth(WINDOW_MAIN_DOCK_MIN);
    dockLayers->setMaximumWidth(WINDOW_MAIN_DOCK_MAX);
    dockLayers->setWidget(windowLayers_);
    dockLayers->setVisible(false);
    addDockWidget(Qt::LeftDockWidgetArea, dockLayers);

    // Create view settings window
    windowSettingsView_ = new WindowSettingsView(this);
    connect(windowSettingsView_,
            SIGNAL(settingsChanged()),
            this,
            SLOT(actionSettingsView()));

    QDockWidget *dockViewSettings = new QDockWidget(tr("View Settings"), this);
    dockViewSettings->setAllowedAreas(Qt::LeftDockWidgetArea |
                                      Qt::RightDockWidgetArea);
    dockViewSettings->setMinimumWidth(WINDOW_MAIN_DOCK_MIN);
    dockViewSettings->setMaximumWidth(WINDOW_MAIN_DOCK_MAX);
    dockViewSettings->setWidget(windowSettingsView_);
    addDockWidget(Qt::LeftDockWidgetArea, dockViewSettings);

    // Create clip filter window
    windowClipFilter_ = new WindowClipFilter(this);
    connect(windowClipFilter_,
            SIGNAL(filterChanged(const ClipFilter &)),
            this,
            SLOT(actionClipFilter(const ClipFilter &)));
    connect(windowClipFilter_,
            SIGNAL(filterReset()),
            this,
            SLOT(actionClipFilterReset()));

    QDockWidget *dockClipFilter = new QDockWidget(tr("Clip Filter"), this);
    dockClipFilter->setAllowedAreas(Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockClipFilter->setMinimumWidth(WINDOW_MAIN_DOCK_MIN);
    dockClipFilter->setMaximumWidth(WINDOW_MAIN_DOCK_MAX);
    dockClipFilter->setWidget(windowClipFilter_);
    addDockWidget(Qt::LeftDockWidgetArea, dockClipFilter);

    // Log
    log = new QTextEdit(this);
    log->setReadOnly(true);

    QDockWidget *dockLog = new QDockWidget(tr("Log"), this);
    dockLog->setMinimumHeight(200);
    dockLog->setWidget(log);
    dockLog->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockLog);

    // Add dock widgets to Windows menu
    menuWindows_->addAction(dockDataSets->toggleViewAction());
    menuWindows_->addAction(dockLayers->toggleViewAction());
    menuWindows_->addAction(dockViewSettings->toggleViewAction());
    menuWindows_->addAction(dockClipFilter->toggleViewAction());
    menuWindows_->addAction(dockLog->toggleViewAction());
}

void WindowMain::createPlugins()
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
            // Detect and register various plugins

            // Tool
            PluginTool *pluginToolInterface;
            pluginToolInterface = qobject_cast<PluginTool *>(plugin);
            if (pluginToolInterface)
            {
                pluginToolInterface->initialize(this, &editor_);
                pluginsTool_.push_back(pluginToolInterface);
                (void)menuTools_->addAction(pluginToolInterface->windowTitle(),
                                            this,
                                            &WindowMain::actionPluginToolShow);

                EditorFilter *filter;
                filter = dynamic_cast<EditorFilter *>(pluginToolInterface);
                if (filter)
                {
                    editor_.addFilter(filter);
                }

                continue;
            }

            // File
            PluginFile *pluginFileInterface;
            pluginFileInterface = qobject_cast<PluginFile *>(plugin);
            if (pluginFileInterface)
            {
                pluginsFile_.push_back(pluginFileInterface);
                continue;
            }
        }
    }
}

void WindowMain::actionProjectNew()
{
    if (projectClose())
    {
        updateProject();
    }
}

void WindowMain::actionProjectOpen()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open"),
                                            "",
                                            tr(WINDOW_MAIN_FILE_FILTER));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectOpen(fileName);
}

void WindowMain::actionProjectSave()
{
    (void)projectSave();
}

void WindowMain::actionProjectSaveAs()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
                                            tr("Save As"),
                                            "",
                                            tr(WINDOW_MAIN_FILE_FILTER));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectSave(fileName);
}

void WindowMain::actionProjectImport()
{
}

void WindowMain::actionProjectExportAs()
{
}

void WindowMain::actionViewOrthographic()
{
    windowViewports_->setViewOrthographic();
}

void WindowMain::actionViewPerspective()
{
    windowViewports_->setViewPerspective();
}

void WindowMain::actionViewTop()
{
    windowViewports_->setViewTop();
}

void WindowMain::actionViewFront()
{
    windowViewports_->setViewFront();
}

void WindowMain::actionViewLeft()
{
    windowViewports_->setViewLeft();
}

void WindowMain::actionView3d()
{
    windowViewports_->setView3d();
}

void WindowMain::actionViewResetDistance()
{
    windowViewports_->setViewResetDistance();
}

void WindowMain::actionViewResetCenter()
{
    windowViewports_->setViewResetCenter();
}

void WindowMain::actionViewLayoutSingle()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setNumberOfViewports(1);
    windowViewports_->setLayout(WindowViewports::VIEW_LAYOUT_SINGLE);
    editor_.unlock();
    updateViewer();
}

void WindowMain::actionViewLayout2Columns()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setNumberOfViewports(2);
    windowViewports_->setLayout(WindowViewports::VIEW_LAYOUT_TWO_COLUMNS);
    windowViewports_->resetScene(&editor_, 1);
    editor_.unlock();
    updateViewer();
}

void WindowMain::actionViewLayout3RowsRight()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setNumberOfViewports(4);
    windowViewports_->setLayout(WindowViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT);
    windowViewports_->resetScene(&editor_, 1);
    windowViewports_->resetScene(&editor_, 2);
    windowViewports_->resetScene(&editor_, 3);
    editor_.unlock();
    updateViewer();
}

void WindowMain::actionPluginToolShow()
{
    QObject *obj = sender();
    QAction *action = qobject_cast<QAction *>(obj);

    if (action)
    {
        // Lookup and run a plugin according to action text from plugin menu
        for (auto &it : pluginsTool_)
        {
            if (it->windowTitle() == action->text())
            {
                try
                {
                    it->show(this);
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

void WindowMain::actionDataSetVisible(size_t id, bool checked)
{
    editor_.cancelThreads();
    editor_.setVisibleDataSet(id, checked);
    updateViewer();
    // setWindowModified(true);
}

void WindowMain::actionLayerVisible(size_t id, bool checked)
{
    editor_.cancelThreads();
    editor_.setVisibleLayer(id, checked);
    updateViewer();
}

void WindowMain::actionClipFilter(const ClipFilter &clipFilter)
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setClipFilter(clipFilter);
    editor_.tileViewClear();
    editor_.unlock();
    editor_.restartThreads();
}

void WindowMain::actionClipFilterReset()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.resetClipFilter();
    editor_.tileViewClear();
    editor_.unlock();
    editor_.restartThreads();
    windowClipFilter_->setClipFilter(editor_);
}

void WindowMain::actionSettingsView()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setSettingsView(windowSettingsView_->settings());
    editor_.unlock();
    editor_.restartThreads();
}

void WindowMain::actionAbout()
{
    QMessageBox::about(this,
                       tr("About 3D Forest, version ") +
                           WindowMain::APPLICATION_VERSION,
                       tr("3D Forest is software for analysis of Lidar data"
                          " from forest environment.\n\n"
                          "Copyright 2020-2021 VUKOZ\n"
                          "Blue Cat team and other authors\n"
                          "https://www.3dforest.eu/"));
}

bool WindowMain::projectOpen(const QString &path)
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

bool WindowMain::projectClose()
{
    editor_.cancelThreads();

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
    try
    {
        editor_.close();
    }
    catch (std::exception &e)
    {
        // showError(e.what());
    }

    return true; // Closed
}

bool WindowMain::projectSave(const QString &path)
{
    std::string writePath;

    editor_.cancelThreads();

    if (path.isEmpty())
    {
        // Save
        writePath = editor_.path();
        if (writePath.empty())
        {
            // First time save
            QString fileName;
            fileName =
                QFileDialog::getSaveFileName(this,
                                             tr("Save As"),
                                             "",
                                             tr(WINDOW_MAIN_FILE_FILTER));

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

void WindowMain::updateProject()
{
    editor_.cancelThreads();
    editor_.lock();
    windowViewports_->resetScene(&editor_);
    editor_.unlock();

    windowDataSets_->updateEditor(editor_);
    windowLayers_->updateEditor(editor_);
    windowSettingsView_->setSettings(editor_.settings().view());
    windowClipFilter_->setClipFilter(editor_);
    updateViewer();
    updateWindowTitle(QString::fromStdString(editor_.path()));
}

void WindowMain::actionCameraChanged(size_t viewportId)
{
    editor_.render(viewportId, windowViewports_->camera(viewportId));
}

void WindowMain::actionEditorRender()
{
    editor_.lock();
    windowViewports_->updateScene(&editor_);
    editor_.unlock();
}

void WindowMain::updateViewer()
{
    actionCameraChanged(0);
}

void WindowMain::showError(const char *message)
{
    (void)QMessageBox::critical(this, tr("Error"), message);
}

void WindowMain::updateWindowTitle(const QString &path)
{
    QString newtitle = APPLICATION_NAME;

    if (path != "")
    {
        newtitle = newtitle + " - " + path;
    }

    setWindowTitle(newtitle + " [*]");
}

void WindowMain::closeEvent(QCloseEvent *event)
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
