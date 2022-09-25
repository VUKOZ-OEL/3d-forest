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

/** @file MainWindow.cpp */

#include <Log.hpp>

#include <HelpPlugin.hpp>
#include <ImportPlugin.hpp>
#include <MainWindow.hpp>
#include <PluginInterface.hpp>
#include <ProjectFilePlugin.hpp>
#include <ProjectNavigatorPlugin.hpp>
#include <SettingsPlugin.hpp>
#include <ViewerPlugin.hpp>
#include <ViewerViewports.hpp>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QPluginLoader>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>

#define MODULE_NAME "MainWindow"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

#if !defined(EXPORT_GUI_IMPORT)
const char *MainWindow::APPLICATION_NAME = "3D Forest";
const char *MainWindow::APPLICATION_VERSION = "1.0";
const int MainWindow::ICON_SIZE = 24;
const int MainWindow::ICON_SIZE_TEXT = 16;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      threadRender_(&editor_)
{
    LOG_DEBUG_LOCAL("");

    // Status bar
    statusProgressBar_ = new QProgressBar;
    statusProgressBar_->setRange(0, 100);
    statusProgressBar_->setTextVisible(false);
    statusProgressBar_->setFixedWidth(100);
    statusProgressBar_->setFixedHeight(MainWindow::ICON_SIZE_TEXT);
    setStatusProgressBarPercent(0);

    QHBoxLayout *statusLayout = new QHBoxLayout;
    statusLayout->addStretch();
    statusLayout->addWidget(statusProgressBar_);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(0);

    QWidget *statusWidget = new QWidget;
    statusWidget->setLayout(statusLayout);
    statusWidget->setContentsMargins(0, 0, 0, 0);

    statusBar()->addPermanentWidget(statusWidget);
    statusBar()->showMessage(tr("Ready"));

    // Menu
    projectFilePlugin_ = new ProjectFilePlugin();
    projectFilePlugin_->initialize(this);

    importPlugin_ = new ImportPlugin();
    importPlugin_->initialize(this);

    projectNavigatorPlugin_ = new ProjectNavigatorPlugin();
    projectNavigatorPlugin_->initialize(this);

    settingsPlugin_ = new SettingsPlugin();
    settingsPlugin_->initialize(this);

    viewerPlugin_ = new ViewerPlugin();
    viewerPlugin_->initialize(this);

    loadPlugins();

    helpPlugin_ = new HelpPlugin();
    helpPlugin_->initialize(this);

    // Exit
    createAction(&actionExit_,
                 "File",
                 "",
                 tr("E&xit"),
                 tr("Exit the application"),
                 QIcon(),
                 this,
                 SLOT(close()));
    actionExit_->setShortcuts(QKeySequence::Quit);

    // Show windows
    projectNavigatorPlugin_->slotPlugin();
    settingsPlugin_->slotPlugin();

    // Rendering
    connect(viewerPlugin_->viewports(),
            SIGNAL(cameraChanged(size_t)),
            this,
            SLOT(slotRenderViewport(size_t)));

    connect(this,
            SIGNAL(signalRender()),
            this,
            SLOT(slotRender()),
            Qt::QueuedConnection);

    threadRender_.setCallback(this);
    threadRender_.create();

    updateEverything();
}

MainWindow::~MainWindow()
{
    LOG_DEBUG_LOCAL("");
    threadRender_.stop();
}

QSize MainWindow::minimumSizeHint() const
{
    return QSize(320, 200);
}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}

void MainWindow::showError(const char *message)
{
    (void)QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::setWindowTitle(const QString &path)
{
    QString newtitle = APPLICATION_NAME;

    if (path != "")
    {
        newtitle = newtitle + " - " + path;
    }

    QMainWindow::setWindowTitle(newtitle + " [*]");
}

void MainWindow::createAction(QAction **result,
                              const QString &menu,
                              const QString &toolBar,
                              const QString &text,
                              const QString &toolTip,
                              const QIcon &icon,
                              const QObject *receiver,
                              const char *member)
{
    QAction *action;

    // Create action
    action = new QAction(text, this);

    if (!toolTip.isEmpty())
    {
        action->setToolTip(toolTip);
        action->setStatusTip(toolTip);
    }

    if (!icon.isNull())
    {
        action->setIcon(icon);
    }

    // Connect action
    if (receiver && member)
    {
        connect(action, SIGNAL(triggered()), receiver, member);
    }

    // Add action to menu
    if (!menu_.contains(menu))
    {
        menu_[menu] = menuBar()->addMenu(menu);
    }
    else if (!toolBar_.contains(toolBar))
    {
        createMenuSeparator(menu);
    }
    menu_[menu]->addAction(action);

    if (!toolBar.isEmpty() && !icon.isNull())
    {
        if (!toolBar_.contains(toolBar))
        {
            toolBar_[toolBar] = addToolBar(toolBar);
            toolBar_[toolBar]->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        }
        toolBar_[toolBar]->addAction(action);
    }

    // Optional return value for further customization of new action
    if (result)
    {
        *result = action;
    }
}

void MainWindow::createToolButton(QToolButton **result,
                                  const QString &text,
                                  const QString &toolTip,
                                  const QIcon &icon,
                                  const QObject *receiver,
                                  const char *member)
{
    QToolButton *button;

    // Create button
    button = new QToolButton;
    button->setText(text);
    button->setToolTip(toolTip);
    button->setStatusTip(toolTip);
    button->setIcon(icon);
    button->setEnabled(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Connect button
    if (receiver && member)
    {
        connect(button, SIGNAL(clicked()), receiver, member);
    }

    // Return value
    *result = button;
}

void MainWindow::createMenuSeparator(const QString &menu)
{
    menu_[menu]->addSeparator();
}

void MainWindow::hideToolBar(const QString &menu)
{
    if (toolBar_.contains(menu))
    {
        toolBar_[menu]->close();
    }
}

void MainWindow::setStatusProgressBarPercent(int percent)
{
    statusProgressBar_->setValue(percent);

    if (percent > 0 && percent < 100)
    {
        statusProgressBar_->setVisible(true);
    }
    else
    {
        statusProgressBar_->setVisible(false);
    }
}

void MainWindow::loadPlugins()
{
    // Process all files in the application "exe" directory
    QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins/");
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entries)
    {
        // Try to load the file as a plugin
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();

        loadPlugin(plugin);
    }
}

void MainWindow::loadPlugin(QObject *plugin)
{
    if (!plugin)
    {
        return;
    }

    // Detect and register various plugins

    PluginInterface *pluginInterface;
    pluginInterface = qobject_cast<PluginInterface *>(plugin);
    if (pluginInterface)
    {
        pluginInterface->initialize(this);
        plugins_.push_back(pluginInterface);

        ModifierInterface *modifier;
        modifier = dynamic_cast<ModifierInterface *>(pluginInterface);
        if (modifier)
        {
            editor_.addModifier(modifier);
        }

        return;
    }
}

void MainWindow::suspendThreads()
{
    LOG_DEBUG_LOCAL("");
    threadRender_.cancel();
}

void MainWindow::resumeThreads()
{
    LOG_DEBUG_LOCAL("");
    slotRenderViewport();
}

void MainWindow::threadProgress(bool finished)
{
    (void)finished;
    LOG_DEBUG_LOCAL("finished=" << finished);
    emit signalRender();
}

void MainWindow::slotRender()
{
    LOG_DEBUG_LOCAL("");
    editor_.lock();
    viewerPlugin_->viewports()->updateScene(&editor_);
    editor_.unlock();
}

void MainWindow::slotRenderViewport()
{
    LOG_DEBUG_LOCAL("");
    slotRenderViewport(viewerPlugin_->viewports()->selectedViewportId());
}

void MainWindow::slotRenderViewport(size_t viewportId)
{
    LOG_DEBUG_LOCAL("");
    ViewerViewports *viewports = viewerPlugin_->viewports();
    threadRender_.render(viewportId, viewports->camera(viewportId));
}

void MainWindow::update(const QString &target)
{
    LOG_DEBUG_LOCAL("");
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    emit signalUpdate(target);
}

void MainWindow::update(const std::set<std::string> &target)
{
    LOG_DEBUG_LOCAL("target <" << target << ">");

    suspendThreads();
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    editor_.viewports().setState(Page::STATE_READ);
    // editor_.viewports().clearContent();

    emit signalUpdate("Test");

    // ViewerViewports *viewports = viewerPlugin_->viewports();
    // viewports->resetScene(&editor_, false);

    resumeThreads();
}

void MainWindow::updateEverything()
{
    LOG_DEBUG_LOCAL("");

    suspendThreads();
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    ViewerViewports *viewports = viewerPlugin_->viewports();

    editor_.lock();
    viewports->resetScene(&editor_, true);
    editor_.unlock();

    emit signalUpdate("");

    size_t viewportId = viewports->selectedViewportId();
    threadRender_.render(viewportId, viewports->camera(viewportId));

    setWindowTitle(QString::fromStdString(editor_.projectPath()));
}

void MainWindow::updateData()
{
    suspendThreads();
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    ViewerViewports *viewports = viewerPlugin_->viewports();
    viewports->resetScene(&editor_, false);
    editor_.viewports().clearContent();

    resumeThreads();
}

void MainWindow::updateFilter()
{
    suspendThreads();
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    ViewerViewports *viewports = viewerPlugin_->viewports();
    viewports->resetScene(&editor_, false);
    editor_.viewports().setState(Page::STATE_SELECT);

    resumeThreads();
}

void MainWindow::updateModifiers()
{
    suspendThreads();
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    editor_.viewports().setState(Page::STATE_RUN_MODIFIERS);

    resumeThreads();
}

void MainWindow::updateRender()
{
    suspendThreads();
    LOG_UPDATE_VIEW(MODULE_NAME, "");

    editor_.viewports().setState(Page::STATE_RENDER);

    resumeThreads();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (projectFilePlugin_->projectClose())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
