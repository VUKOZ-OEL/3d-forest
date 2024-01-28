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

// Include 3D Forest.
#include <MainWindow.hpp>
#include <PluginInterface.hpp>
#include <ViewerViewports.hpp>

// Include 3D Forest plugins.
#include <ExplorerPlugin.hpp>
#include <ExportFilePlugin.hpp>
#include <HelpPlugin.hpp>
#include <ImportFilePlugin.hpp>
#include <MessageLogPlugin.hpp>
#include <ProjectFilePlugin.hpp>
#include <SettingsPlugin.hpp>
#include <ViewerPlugin.hpp>

// Include Qt.
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QPluginLoader>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>

// Include local.
#define LOG_MODULE_NAME "MainWindow"
#include <Log.hpp>

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
    LOG_DEBUG(<< "Create.");

    // Status bar.
    statusBar()->showMessage(tr("Ready"));

    // Menu.
    projectFilePlugin_ = new ProjectFilePlugin();
    projectFilePlugin_->initialize(this);

    importFilePlugin_ = new ImportFilePlugin();
    importFilePlugin_->initialize(this);

    exportFilePlugin_ = new ExportFilePlugin();
    exportFilePlugin_->initialize(this);

    explorerPlugin_ = new ExplorerPlugin();
    explorerPlugin_->initialize(this);

    settingsPlugin_ = new SettingsPlugin();
    settingsPlugin_->initialize(this);

    messageLogPlugin_ = new MessageLogPlugin();
    messageLogPlugin_->initialize(this);

    viewerPlugin_ = new ViewerPlugin();
    viewerPlugin_->initialize(this);

    loadPlugins();

    helpPlugin_ = new HelpPlugin();
    helpPlugin_->initialize(this);

    // Exit.
    createAction(&actionExit_,
                 "File",
                 "",
                 tr("E&xit"),
                 tr("Exit the application"),
                 QIcon(),
                 this,
                 SLOT(close()));
    actionExit_->setShortcuts(QKeySequence::Quit);

    // Show windows.
    explorerPlugin_->slotPlugin();
    settingsPlugin_->slotPlugin();

    // Rendering.
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
    LOG_DEBUG(<< "Destroy.");
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

void MainWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG(<< "Show.");

    resizeDocks({explorerPlugin_->window(), settingsPlugin_->window()},
                {80, 20},
                Qt::Vertical);

    QWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    QWidget::hideEvent(event);
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
    LOG_DEBUG(<< "Create action menu <" << menu.toStdString() << "> toolBar <"
              << toolBar.toStdString() << "> text <" << text.toStdString()
              << "> icon sizes <" << icon.availableSizes().count() << ">.");

    QAction *action;

    // Create action.
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

    // Connect action.
    if (receiver && member)
    {
        connect(action, SIGNAL(triggered()), receiver, member);
    }

    // Add action to menu.
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

    // Optional return value for further customization of new action.
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

    // Create button.
    button = new QToolButton;
    button->setText(text);
    button->setToolTip(toolTip);
    button->setStatusTip(toolTip);
    button->setIcon(icon);
    button->setEnabled(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Connect button.
    if (receiver && member)
    {
        connect(button, SIGNAL(clicked()), receiver, member);
    }

    // Return value.
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

void MainWindow::loadPlugins()
{
    // Process all files in the application "exe" directory.
    QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins/");
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entries)
    {
        // Try to load the file as a plugin.
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

    // Detect and register various plugins.

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
    LOG_TRACE_THREAD(<< "Suspend threads.");
    threadRender_.cancel();
}

void MainWindow::resumeThreads()
{
    LOG_TRACE_THREAD(<< "Resume threads.");
    slotRenderViewport();
}

void MainWindow::threadProgress(bool finished)
{
    (void)finished;
    LOG_TRACE_THREAD(<< "Thread progress finished <" << finished << ">.");
    emit signalRender();
}

void MainWindow::slotRender()
{
    LOG_TRACE_UPDATE_VIEW(<< "Render all viewports.");
    editor_.lock();
    viewerPlugin_->viewports()->updateScene(&editor_);
    editor_.unlock();
}

void MainWindow::slotRenderViewport()
{
    LOG_TRACE_UPDATE_VIEW(<< "Render active viewport.");
    slotRenderViewport(viewerPlugin_->viewports()->selectedViewportId());
}

void MainWindow::slotRenderViewport(size_t viewportId)
{
    LOG_TRACE_UPDATE_VIEW(<< "Render viewport <" << viewportId << ">.");
    ViewerViewports *viewports = viewerPlugin_->viewports();
    threadRender_.render(viewportId, viewports->camera(viewportId));
}

void MainWindow::update(void *sender, const QSet<Editor::Type> &target)
{
    emit signalUpdate(sender, target);
}

void MainWindow::update(const QSet<Editor::Type> &target,
                        Page::State viewPortsCacheState,
                        bool resetCamera)
{
    LOG_TRACE_UPDATE(<< "Update number of targets <" << target.count() << ">.");
    suspendThreads();

    editor_.viewports().setState(viewPortsCacheState);
    // editor_.viewports().clearContent();

    if (resetCamera)
    {
        ViewerViewports *viewports = viewerPlugin_->viewports();
        viewports->resetScene(&editor_, false);
    }

    update(this, target);

    resumeThreads();
}

void MainWindow::updateEverything()
{
    LOG_TRACE_UPDATE(<< "Update everything.");
    suspendThreads();

    ViewerViewports *viewports = viewerPlugin_->viewports();

    editor_.lock();
    viewports->resetScene(&editor_, true);
    editor_.unlock();

    emit signalUpdate(this, {});

    size_t viewportId = viewports->selectedViewportId();
    threadRender_.render(viewportId, viewports->camera(viewportId));

    setWindowTitle(QString::fromStdString(editor_.projectPath()));
}

void MainWindow::updateData()
{
    LOG_TRACE_UPDATE(<< "Update data.");
    suspendThreads();

    ViewerViewports *viewports = viewerPlugin_->viewports();
    viewports->resetScene(&editor_, false);
    editor_.viewports().clearContent();

    resumeThreads();
}

void MainWindow::updateFilter()
{
    LOG_TRACE_UPDATE(<< "Update filter.");
    suspendThreads();

    ViewerViewports *viewports = viewerPlugin_->viewports();
    viewports->resetScene(&editor_, false);
    editor_.viewports().setState(Page::STATE_SELECT);

    resumeThreads();
}

void MainWindow::updateModifiers()
{
    LOG_TRACE_UPDATE(<< "Update modifiers.");
    suspendThreads();

    editor_.viewports().setState(Page::STATE_RUN_MODIFIERS);

    resumeThreads();
}

void MainWindow::updateRender()
{
    LOG_TRACE_UPDATE(<< "Update render.");
    suspendThreads();

    editor_.viewports().setState(Page::STATE_RENDER);

    resumeThreads();
}
