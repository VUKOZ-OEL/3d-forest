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
#include <GuiUtil.hpp>
#include <MainWindow.hpp>

// Include 3D Forest plugins.
#include <ImportFileInterface.hpp>
#include <PluginInterface.hpp>
#include <ProjectFileInterface.hpp>
#include <ViewerInterface.hpp>
#include <HelpPlugin.hpp>

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
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#if !defined(EXPORT_GUI_IMPORT)
const char *MainWindow::APPLICATION_NAME = "3D Forest";
const char *MainWindow::APPLICATION_VERSION = "1.0";
const int MainWindow::ICON_SIZE = 16;
const int MainWindow::ICON_SIZE_TEXT = 16;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      threadRender_(&editor_),
      projectFilePlugin_(nullptr),
      importFilePlugin_(nullptr),
      viewerPlugin_(nullptr),
      helpPlugin_(nullptr)
{
    LOG_DEBUG(<< "Start creating the main window.");

    // Status bar.
    statusBar()->showMessage(tr("Ready"));

    // Plugins.
    helpPlugin_ = new HelpPlugin();
    helpPlugin_->initialize(this);

    loadPlugins();

    // Exit.
    createAction(&exitAction_,
                 "File",
                 "",
                 tr("E&xit"),
                 tr("Exit the application"),
                 QIcon(),
                 this,
                 SLOT(close()),
                 MAIN_WINDOW_MENU_FILE_PRIORITY,
                 100);
    exitAction_->setShortcuts(QKeySequence::Quit);

    // Menu.
    createMenu();

    // Rendering.
    connect(this,
            SIGNAL(signalRender()),
            this,
            SLOT(slotRender()),
            Qt::QueuedConnection);

    threadRender_.setCallback(this);
    threadRender_.create();

    // Update.
    setWindowTitle(QString::fromStdString(editor_.projectPath()));

    LOG_DEBUG_UPDATE(<< "Emit update.");
    emit signalUpdate(this, {});

    if (viewerPlugin_)
    {
        viewerPlugin_->resetScene(&editor_, true);
    }
    else
    {
        LOG_ERROR(<< "The viewer plugin is not loaded to perform the action.");
    }

    LOG_DEBUG(<< "Finished creating the main window.");
}

MainWindow::~MainWindow()
{
    LOG_DEBUG(<< "Start destroying the main window.");
    threadRender_.stop();
    LOG_DEBUG(<< "Finished destroying the main window.");
}

QSize MainWindow::minimumSizeHint() const
{
    return QSize(320, 200);
}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Paint event.");
    QWidget::paintEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Resize event.");
    QWidget::resizeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    QWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide.");
    QWidget::hideEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!projectFilePlugin_)
    {
        LOG_ERROR(<< "The project file plugin is not loaded"
                     " to perform the action.");
        event->accept();
        return;
    }

    if (projectFilePlugin_->closeProject())
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

void MainWindow::importFile()
{
    if (!importFilePlugin_)
    {
        LOG_ERROR(<< "The import file plugin is not loaded"
                     " to perform the action.");
        return;
    }

    importFilePlugin_->importFile();
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
                              const QString &menuTitle,
                              const QString &toolBarTitle,
                              const QString &text,
                              const QString &toolTip,
                              const QIcon &icon,
                              const QObject *receiver,
                              const char *member,
                              int menuPriority,
                              int menuItemPriority)
{
    LOG_DEBUG(<< "Create action menu <" << menuTitle.toStdString()
              << "> toolBar <" << toolBarTitle.toStdString() << "> text <"
              << text.toStdString() << "> priority <" << menuPriority << "/"
              << menuItemPriority << "> icon sizes <"
              << icon.availableSizes().count() << ">.");

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
    MainWindow::MenuItem menuItem;
    menuItem.action = action;
    menuItem.title = text;
    menuItem.toolBarTitle = toolBarTitle;
    menuItem.priority = menuItemPriority;

    if (!menuIndex_.contains(menuTitle))
    {
        MainWindow::Menu menu;
        menu.menu = nullptr;
        menu.title = menuTitle;
        menu.priority = menuPriority;
        menu.items.push_back(std::move(menuItem));

        if (menuItem.priority < 0)
        {
            menuItem.priority = 0;
        }

        menuIndex_[menuTitle] = menus_.size();
        menus_.push_back(std::move(menu));
    }
    else
    {
        MainWindow::Menu &menu = menus_[menuIndex_[menuTitle]];

        if (menuItem.priority < 0)
        {
            menuItem.priority = static_cast<int>(menu.items.size()) * 10;
        }

        menu.items.push_back(std::move(menuItem));
    }

    // Optional return value for further customization of new action.
    if (result)
    {
        *result = action;
    }
}

void MainWindow::createMenu()
{
    // Sort menu.
    std::sort(menus_.begin(),
              menus_.end(),
              [](const MainWindow::Menu &a, const MainWindow::Menu &b)
              {
                  return (a.priority < b.priority) ||
                         (a.priority == b.priority && a.title < b.title);
              });

    // Sort menu items.
    for (auto &menu : menus_)
    {
        std::sort(
            menu.items.begin(),
            menu.items.end(),
            [](const MainWindow::MenuItem &a, const MainWindow::MenuItem &b)
            {
                return (a.priority < b.priority) ||
                       ((a.priority == b.priority) &&
                        ((a.toolBarTitle < b.toolBarTitle) ||
                         (a.toolBarTitle == b.toolBarTitle &&
                          a.title < b.title)));
            });
    }

    // Create menu.
    for (auto &menu : menus_)
    {
        menu.menu = menuBar()->addMenu(menu.title);

        QString previousToolBarTitle;
        size_t i = 0;
        for (const auto &item : menu.items)
        {
            if (i > 0 && item.toolBarTitle != previousToolBarTitle)
            {
                menu.menu->addSeparator();
            }

            menu.menu->addAction(item.action);

            if (!item.toolBarTitle.isEmpty() && !item.action->icon().isNull())
            {
                if (!toolBars_.contains(item.toolBarTitle))
                {
                    toolBars_[item.toolBarTitle] =
                        addToolBar(item.toolBarTitle);
                    toolBars_[item.toolBarTitle]->setIconSize(
                        QSize(ICON_SIZE, ICON_SIZE));
                }
                toolBars_[item.toolBarTitle]->addAction(item.action);
            }

            previousToolBarTitle = menu.items[i].toolBarTitle;
            i++;
        }
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

void MainWindow::hideToolBar(const QString &menu)
{
    if (toolBars_.contains(menu))
    {
        toolBars_[menu]->close();
    }
}

void MainWindow::loadPlugins()
{
    LOG_DEBUG(<< "Start loading all plugins.");

    QString pluginsDirPath =
        QCoreApplication::applicationDirPath() + "/plugins/";
    LOG_DEBUG(<< "Load plugins from directory <" << pluginsDirPath.toStdString()
              << ">.");

    // Process all files in the application "exe" directory.
    QDir pluginsDir(pluginsDirPath);
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    qsizetype n = entries.count();
    LOG_DEBUG(<< "Found number of files <" << n << ">.");

    qsizetype i = 0;
    for (const QString &fileName : entries)
    {
        i++;

        // Try to load the file as a plugin.
        QString pluginPath = pluginsDir.absoluteFilePath(fileName);

        if (!(fileName.endsWith(".dll") || fileName.endsWith(".so")))
        {
            LOG_DEBUG(<< "Skip file <" << i << "/" << n << "> path <"
                      << pluginPath.toStdString() << ">.");
            continue;
        }

        LOG_DEBUG(<< "Load file <" << i << "/" << n << "> path <"
                  << pluginPath.toStdString() << ">.");

        QPluginLoader pluginLoader(pluginPath);
        QObject *plugin = pluginLoader.instance();

        if (plugin)
        {
            if (!loadPlugin(plugin))
            {
                LOG_WARNING(<< "Skip unknown plugin <"
                            << pluginPath.toStdString() << ">.");
            }
        }
        else
        {
            LOG_ERROR(<< "Unable to get instance of plugin <"
                      << pluginPath.toStdString() << ">.");
        }
    }

    LOG_DEBUG(<< "Finished loading all plugins.");
}

bool MainWindow::loadPlugin(QObject *plugin)
{
    // Detect and register various plugins.
    PluginInterface *pluginInterface;
    pluginInterface = qobject_cast<PluginInterface *>(plugin);
    if (!pluginInterface)
    {
        LOG_DEBUG(<< "Plugin interface not recognized.");
        return false;
    }

    pluginInterface->initialize(this);
    plugins_.push_back(pluginInterface);

    // Modifier.
    ModifierInterface *modifierInterface;
    modifierInterface = dynamic_cast<ModifierInterface *>(pluginInterface);
    if (modifierInterface)
    {
        LOG_DEBUG(<< "Add modifier plugin.");
        editor_.addModifier(modifierInterface);
    }

    // Project file.
    ProjectFileInterface *projectFileInterface;
    projectFileInterface =
        dynamic_cast<ProjectFileInterface *>(pluginInterface);
    if (projectFileInterface)
    {
        LOG_DEBUG(<< "Set project file plugin.");
        projectFilePlugin_ = projectFileInterface;
    }

    // Import file.
    ImportFileInterface *importFileInterface;
    importFileInterface = dynamic_cast<ImportFileInterface *>(pluginInterface);
    if (importFileInterface)
    {
        LOG_DEBUG(<< "Set import file plugin.");
        importFilePlugin_ = importFileInterface;
    }

    // Viewer.
    ViewerInterface *viewerInterface;
    viewerInterface = dynamic_cast<ViewerInterface *>(pluginInterface);
    if (viewerInterface)
    {
        LOG_DEBUG(<< "Set viewer plugin.");
        viewerPlugin_ = viewerInterface;
    }

    return true;
}

void MainWindow::suspendThreads()
{
    LOG_DEBUG_RENDER(<< "Suspend threads.");

    threadRender_.cancel();
}

void MainWindow::resumeThreads()
{
    LOG_DEBUG_RENDER(<< "Resume threads.");

    slotRenderViewports();
}

void MainWindow::threadProgress(bool finished)
{
    (void)finished;

    LOG_DEBUG_RENDER(<< "Thread progress finished <" << finished << ">.");

    emit signalRender();
}

void MainWindow::slotRender()
{
    if (viewerPlugin_)
    {
        std::unique_lock<std::mutex> mutexlock(editor_.mutex_);
        viewerPlugin_->updateScene(&editor_);
    }
}

void MainWindow::slotRenderViewport(size_t viewportId)
{
    LOG_DEBUG_RENDER(<< "Render viewport <" << viewportId << ">.");

    if (viewerPlugin_)
    {
        threadRender_.render(viewerPlugin_->camera(viewportId));
    }
}

void MainWindow::slotRenderViewports()
{
    LOG_DEBUG_RENDER(<< "Render viewports.");

    if (viewerPlugin_)
    {
        threadRender_.render(viewerPlugin_->camera());
    }
}

void MainWindow::update(void *sender, const QSet<Editor::Type> &target)
{
    LOG_DEBUG_UPDATE(<< "Update target <" << target << "> emit.");

    emit signalUpdate(sender, target);
}

void MainWindow::update(const QSet<Editor::Type> &target,
                        Page::State viewPortsCacheState,
                        bool resetCamera)
{
    LOG_DEBUG_UPDATE(<< "Update target <" << target << "> set page state <"
                     << viewPortsCacheState << "> reset camera <"
                     << static_cast<int>(resetCamera) << ">.");

    suspendThreads();

    editor_.viewports().setState(viewPortsCacheState);
    // editor_.viewports().clearContent();

    if (resetCamera)
    {
        if (viewerPlugin_)
        {
            viewerPlugin_->resetScene(&editor_, false);
        }
    }

    update(this, target);

    resumeThreads();
}

void MainWindow::updateNewProject()
{
    LOG_DEBUG(<< "Start updating new project.");

    setWindowTitle(QString::fromStdString(editor_.projectPath()));

    if (viewerPlugin_)
    {
        std::unique_lock<std::mutex> mutexlock(editor_.mutex_);
        viewerPlugin_->resetScene(&editor_, true);
    }

    LOG_DEBUG(<< "Emit update.");
    emit signalUpdate(this, {});

    LOG_DEBUG(<< "Finished updating new project.");
}

void MainWindow::updateData()
{
    LOG_DEBUG_UPDATE(<< "Update data.");

    suspendThreads();

    if (viewerPlugin_)
    {
        viewerPlugin_->resetScene(&editor_, false);
    }

    editor_.viewports().clearContent();
    editor_.applyFilters();

    resumeThreads();
}

void MainWindow::updateFilter()
{
    LOG_DEBUG_UPDATE(<< "Update filter.");

    suspendThreads();

    if (viewerPlugin_)
    {
        viewerPlugin_->resetScene(&editor_, false);
    }

    editor_.viewports().setState(Page::STATE_SELECT);

    resumeThreads();
}

void MainWindow::updateModifiers()
{
    LOG_DEBUG_UPDATE(<< "Update modifiers.");

    suspendThreads();

    editor_.viewports().setState(Page::STATE_RUN_MODIFIERS);

    resumeThreads();
}

void MainWindow::updateRender()
{
    LOG_DEBUG_UPDATE(<< "Update render.");

    suspendThreads();

    editor_.viewports().setState(Page::STATE_RENDER);

    resumeThreads();
}
