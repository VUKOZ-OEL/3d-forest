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

/** @file WindowMain.cpp */

#include <PluginFile.hpp>
#include <PluginTool.hpp>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QPluginLoader>
#include <QTextEdit>
#include <Time.hpp>
#include <WindowClassification.hpp>
#include <WindowClipFilter.hpp>
#include <WindowDataSets.hpp>
#include <WindowDock.hpp>
#include <WindowFileImport.hpp>
#include <WindowHelp.hpp>
#include <WindowLayers.hpp>
#include <WindowMain.hpp>
#include <WindowScreenshot.hpp>
#include <WindowSettingsView.hpp>
#include <ribbon.h>

const QString WindowMain::APPLICATION_NAME = "3DForest";
const QString WindowMain::APPLICATION_VERSION = "1.0";
QTextEdit *WindowMain::log = nullptr;

#define WINDOW_MAIN_FILTER_PRJ "3DForest Project (*.json)"
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
    createWindows();
    createViewer();
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

QToolButton *WindowMain::createMenuButton(const QString &text,
                                          const QString &toolTip,
                                          const QIcon &icon)
{
    QToolButton *button = new QToolButton;

    button->setText(text);
    button->setToolTip(toolTip);
    button->setIcon(icon);
    button->setIconSize(QSize(40, 40)); // Same size as Office ribbon icons
    button->setEnabled(true);

    return button;
}

QToolButton *WindowMain::createMenuButton(const QString &text,
                                          const QString &toolTip,
                                          const QString &icon)
{
    QToolButton *button;

    button = createMenuButton(text, toolTip, QIcon(":/icons/" + icon));

    return button;
}

QToolButton *WindowMain::createMenuButton(const QString &title,
                                          const QString &text,
                                          const QString &toolTip,
                                          const QString &icon,
                                          QDockWidget *dockWidget)
{
    QToolButton *button;
    button = createMenuButton(text, toolTip, icon);

    QAction *action;
    action = dockWidget->toggleViewAction();
    action->setText(title);
    action->setToolTip(toolTip);
    action->setIconText(text);
    action->setIcon(button->icon());
    button->setDefaultAction(action);

    return button;
}

WindowDock *WindowMain::createMenuTool(const QString &windowTitle,
                                       const QString &text,
                                       const QString &toolTip,
                                       const QString &icon,
                                       QWidget *dockWidget,
                                       Qt::DockWidgetAreas areas,
                                       Qt::DockWidgetArea area,
                                       bool floating)
{
    WindowDock *dock = new WindowDock(this);

    dock->setWidget(dockWidget);
    dock->setWindowTitle(windowTitle);
    dock->setWindowIcon(QIcon(":/icons/" + icon));
    dock->setAllowedAreas(areas);
    dock->setVisible(false);
    dock->setFloating(floating);
    addDockWidget(area, dock);

    QToolButton *button;

    button = createMenuButton(windowTitle, text, toolTip, icon, dock);
    ribbon_->addButton("Tools", "Windows", button);

    return dock;
}

void WindowMain::createMenus()
{
    QToolButton *button = nullptr;
    ribbon_ = new Ribbon;

    // Project
    ribbon_->addTab(QIcon(":/icons/icons8-briefcase-40.png"), "File");
    ribbon_->setIconSize(QSize(20, 20));

    // New
    button = createMenuButton(tr("New\nproject"),
                              tr("Create new project"),
                              "icons8-edit-file-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionProjectNew()));
    ribbon_->addButton("File", "Project", button);

    // Open
    button = createMenuButton(tr("Open\nproject"),
                              tr("Open existing project"),
                              "icons8-live-folder-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionProjectOpen()));
    ribbon_->addButton("File", "Project", button);

    // Save
    button = createMenuButton(tr("Save\nproject"),
                              tr("Save project"),
                              "icons8-save-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionProjectSave()));
    ribbon_->addButton("File", "Project", button);

    // Save As
    button = createMenuButton(tr("Save As\nproject"),
                              tr("Save project to a different file"),
                              "icons8-save-as-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionProjectSaveAs()));
    ribbon_->addButton("File", "Project", button);

    // Import
    button = createMenuButton(tr("Import"),
                              tr("Append existing file to data sets"),
                              "icons8-add-file-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionProjectImport()));
    ribbon_->addButton("File", "File", button);

    // Export
    button = createMenuButton(tr("Export"),
                              tr("Export visible points to a file"),
                              "icons8-send-file-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionProjectExportAs()));
    button->setEnabled(false);
    ribbon_->addButton("File", "File", button);

    button = createMenuButton(tr("Capture\nscreenshot"),
                              tr("Take a snapshot of rendered data"),
                              "icons8-picture-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionScreenshot()));
    ribbon_->addButton("File", "File", button);

    // View
    ribbon_->addTab(QIcon(":/icons/icons8-monitor-40.png"), "View");
    ribbon_->setIconSize(QSize(20, 20));

    button = createMenuButton(tr("Ortho\ngraphic"),
                              tr("Ortho\ngraphic"),
                              "icons8-orthogonal-view-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewOrthographic()));
    ribbon_->addButton("View", "Projection", button);

    button = createMenuButton(tr("Perspective"),
                              tr("Perspective"),
                              "perspective-view-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewPerspective()));
    ribbon_->addButton("View", "Projection", button);

    button = createMenuButton(tr("Top"), tr("Top"), "icons8-top-view-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewTop()));
    ribbon_->addButton("View", "View", button);

    button =
        createMenuButton(tr("Front"), tr("Front"), "icons8-front-view-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewFront()));
    ribbon_->addButton("View", "View", button);

    button =
        createMenuButton(tr("Right"), tr("Right"), "icons8-right-view-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewRight()));
    ribbon_->addButton("View", "View", button);

    button = createMenuButton(tr("3D"), tr("3D"), "icons8-portraits-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionView3d()));
    ribbon_->addButton("View", "View", button);

    button = createMenuButton(tr("Distance"),
                              tr("Distance"),
                              "icons8-fit-to-width-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewResetDistance()));
    ribbon_->addButton("View", "Reset", button);

    button =
        createMenuButton(tr("Center"), tr("Center"), "icons8-collect-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewResetCenter()));
    ribbon_->addButton("View", "Reset", button);

    button =
        createMenuButton(tr("Single"), tr("Single"), "layout-single-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewLayoutSingle()));
    ribbon_->addButton("View", "Layout", button);

    button = createMenuButton(tr("Columns"),
                              tr("Columns"),
                              "layout-two-columns-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewLayout2Columns()));
    ribbon_->addButton("View", "Layout", button);

    button = createMenuButton(tr("Grid"), tr("Grid"), "layout-grid-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionViewLayoutGrid()));
    ribbon_->addButton("View", "Layout", button);

    button = createMenuButton(tr("3 Right"),
                              tr("3 Right"),
                              "layout-grid-right-40.png");
    connect(button,
            SIGNAL(clicked()),
            this,
            SLOT(actionViewLayout3RowsRight()));
    ribbon_->addButton("View", "Layout", button);

    // Tools
    ribbon_->addTab(QIcon(":/icons/icons8-support-40.png"), "Tools");
    ribbon_->setIconSize(QSize(20, 20));

    // Help
    ribbon_->addTab(QIcon(":/icons/icons8-information-40.png"), "Help");
    ribbon_->setIconSize(QSize(20, 20));

    button = createMenuButton(tr("About"),
                              tr("About this application"),
                              "icons8-about-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionAbout()));
    ribbon_->addButton("Help", "Info", button);

    button = createMenuButton(tr("Manual"),
                              tr("Open User Manual"),
                              "icons8-user-manual-40.png");
    connect(button, SIGNAL(clicked()), this, SLOT(actionHelp()));
    ribbon_->addButton("Help", "Info", button);

    // Ribbon
    QWidget *ribbonDockWidgetContents = new QWidget;
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(ribbon_, 0, 0);
    gridLayout->setMargin(0);
    ribbonDockWidgetContents->setLayout(gridLayout);

    QDockWidget *ribbonDockWidget = new QDockWidget(tr("Ribbon"), this);
    ribbonDockWidget->setTitleBarWidget(new QWidget());
    ribbonDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ribbonDockWidget->setAllowedAreas(Qt::TopDockWidgetArea);
    ribbonDockWidget->setFixedHeight(113);
    ribbonDockWidget->setWidget(ribbonDockWidgetContents);
    addDockWidget(Qt::TopDockWidgetArea, ribbonDockWidget);
}

void WindowMain::createWindows()
{
    // Create data sets window
    windowDataSets_ = new WindowDataSets(this);
    connect(windowDataSets_,
            SIGNAL(itemChangedCheckState(size_t, bool)),
            this,
            SLOT(actionDataSetVisible(size_t, bool)));

    (void)createMenuTool(tr("Data Sets"),
                         tr("Data\nSets"),
                         tr("Show and modify data sets"),
                         "icons8-open-box-40.png",
                         windowDataSets_);

    // Create layers window
    windowLayers_ = new WindowLayers(this);
    connect(windowLayers_,
            SIGNAL(selectionChanged()),
            this,
            SLOT(actionLayers()));

    (void)createMenuTool(tr("Layers"),
                         tr("Layers"),
                         tr("Show and modify layers"),
                         "icons8-animated-40.png",
                         windowLayers_);

    // Create classification window
    windowClassification_ = new WindowClassification(this);
    connect(windowClassification_,
            SIGNAL(selectionChanged()),
            this,
            SLOT(actionClassification()));

    (void)createMenuTool(tr("Classification"),
                         tr("Classifi\ncation"),
                         tr("Show classification"),
                         "icons8-variation-40.png",
                         windowClassification_);

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

    (void)createMenuTool(tr("Clip Filter"),
                         tr("Clip\nFilter"),
                         tr("Setup and apply clip filter"),
                         "icons8-crop-40.png",
                         windowClipFilter_);

    // Create view settings window
    windowSettingsView_ = new WindowSettingsView(this);
    connect(windowSettingsView_,
            SIGNAL(settingsChanged()),
            this,
            SLOT(actionSettingsView()));
    connect(windowSettingsView_,
            SIGNAL(settingsChangedApply()),
            this,
            SLOT(actionSettingsViewColor()));

    (void)createMenuTool(tr("View Settings"),
                         tr("View\nSettings"),
                         tr("Change view settings"),
                         "icons8-tune-40.png",
                         windowSettingsView_);

    // Log
    log = new QTextEdit(this);
    log->setReadOnly(true);

    (void)createMenuTool(tr("Application Log"),
                         tr("Log"),
                         tr("Display application log"),
                         "icons8-pass-fail-40.png",
                         log,
                         Qt::BottomDockWidgetArea,
                         Qt::BottomDockWidgetArea,
                         false);
}

void WindowMain::createPlugins()
{
    // Process all files in the application "exe" directory
    QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins/");
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

                // Create menu button
                QToolButton *button = nullptr;
                button = createMenuButton(pluginToolInterface->buttonText(),
                                          pluginToolInterface->toolTip(),
                                          pluginToolInterface->icon());
                ribbon_->addButton("Tools", "Plugins", button);

                QString name = pluginToolInterface->windowTitle();
                QAction *action = new QAction(name, this);
                action->setText(pluginToolInterface->windowTitle());
                action->setToolTip(pluginToolInterface->toolTip());
                action->setIconText(pluginToolInterface->buttonText());
                action->setIcon(pluginToolInterface->icon());
                connect(action,
                        SIGNAL(triggered()),
                        this,
                        SLOT(actionPluginToolShow()));
                button->setDefaultAction(action);

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
                                            tr("Open Project"),
                                            "",
                                            tr(WINDOW_MAIN_FILTER_PRJ));

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
                                            tr("Save Project As"),
                                            "",
                                            tr(WINDOW_MAIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectSave(fileName);
}

void WindowMain::actionProjectImport()
{
    WindowFileImport::import(this, &editor_);
}

void WindowMain::actionProjectExportAs()
{
}

void WindowMain::actionViewOrthographic()
{
    windowViewports_->setViewOrthographic();
    updateViewer();
}

void WindowMain::actionViewPerspective()
{
    windowViewports_->setViewPerspective();
    updateViewer();
}

void WindowMain::actionViewTop()
{
    windowViewports_->setViewTop();
    updateViewer();
}

void WindowMain::actionViewFront()
{
    windowViewports_->setViewFront();
    updateViewer();
}

void WindowMain::actionViewRight()
{
    windowViewports_->setViewRight();
    updateViewer();
}

void WindowMain::actionView3d()
{
    windowViewports_->setView3d();
    updateViewer();
}

void WindowMain::actionViewResetDistance()
{
    windowViewports_->setViewResetDistance();
    updateViewer();
}

void WindowMain::actionViewResetCenter()
{
    windowViewports_->setViewResetCenter();
    updateViewer();
}

void WindowMain::actionViewLayoutSingle()
{
    actionViewLayout(WindowViewports::VIEW_LAYOUT_SINGLE);
}

void WindowMain::actionViewLayout2Columns()
{
    actionViewLayout(WindowViewports::VIEW_LAYOUT_TWO_COLUMNS);
}

void WindowMain::actionViewLayoutGrid()
{
    actionViewLayout(WindowViewports::VIEW_LAYOUT_GRID);
}

void WindowMain::actionViewLayout3RowsRight()
{
    actionViewLayout(WindowViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT);
}

void WindowMain::actionViewLayout(WindowViewports::ViewLayout layout)
{
    editor_.cancelThreads();
    editor_.lock();

    if (layout == WindowViewports::VIEW_LAYOUT_SINGLE)
    {
        editor_.setNumberOfViewports(1);
        windowViewports_->setLayout(layout);
    }
    else if (layout == WindowViewports::VIEW_LAYOUT_TWO_COLUMNS)
    {
        editor_.setNumberOfViewports(2);
        windowViewports_->setLayout(layout);
        windowViewports_->resetScene(&editor_, 1);
    }
    else if ((layout == WindowViewports::VIEW_LAYOUT_GRID) ||
             (layout == WindowViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT))
    {
        editor_.setNumberOfViewports(4);
        windowViewports_->setLayout(layout);
        windowViewports_->resetScene(&editor_, 1);
        windowViewports_->resetScene(&editor_, 2);
        windowViewports_->resetScene(&editor_, 3);
    }

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

                    // Update action to toggle view
                    QList<QWidget *> widgets = action->associatedWidgets();
                    if (widgets.count() > 0)
                    {
                        QWidget *widget = widgets.at(0);
                        QToolButton *button;
                        button = qobject_cast<QToolButton *>(widget);
                        if (button)
                        {
                            action = it->toggleViewAction();
                            action->setText(it->windowTitle());
                            action->setIconText(it->buttonText());
                            action->setToolTip(it->toolTip());
                            action->setIcon(it->icon());
                            button->setDefaultAction(action);
                        }
                    }
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

void WindowMain::actionClassification()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setClassification(windowClassification_->classification());
    editor_.tileViewClear();
    editor_.unlock();
    editor_.restartThreads();
}

void WindowMain::actionLayers()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setLayers(windowLayers_->layers());
    editor_.tileViewClear();
    editor_.unlock();
    editor_.restartThreads();
}

void WindowMain::actionClipFilter(const ClipFilter &clipFilter)
{
    editor_.cancelThreads();
    editor_.lock();
    /** @todo There is a bug when clip filter is disabled. */
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

void WindowMain::actionSettingsViewColor()
{
    editor_.cancelThreads();
    editor_.lock();
    editor_.setSettingsView(windowSettingsView_->settings());
    editor_.tileViewClear();
    editor_.unlock();
    editor_.restartThreads();
}

void WindowMain::actionScreenshot()
{
    try
    {
        WindowScreenshot::capture(this, windowViewports_, &editor_);
    }
    catch (std::exception &e)
    {
        showError(e.what());
    }
    catch (...)
    {
        showError("Unknown");
    }
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
                          "https://www.3dforest.eu/\n"
                          "\n"
                          "Additional icons: https://icons8.com"));
}

void WindowMain::actionHelp()
{
    WindowHelp *help = new WindowHelp(this);
    help->show();
    help->raise();
    help->activateWindow();
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
            fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"),
                                                    "",
                                                    tr(WINDOW_MAIN_FILTER_PRJ));

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
    windowLayers_->setLayers(editor_.layers());
    windowClassification_->setClassification(editor_.classification());
    windowClipFilter_->setClipFilter(editor_);
    windowSettingsView_->setSettings(editor_.settings().view());

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
    actionCameraChanged(windowViewports_->selectedViewportId());
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
