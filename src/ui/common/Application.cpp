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

/** @file Application.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <Time.hpp>
#include <Widget.hpp>

// Include local.
#define LOG_MODULE_NAME "Application"
#include <Log.hpp>

Application::Application() : threadRender_(&editor_)
{
}

Application::~Application()
{
    LOG_DEBUG(<< "Start destroying the application.");
    threadRender_.stop();
    pluginManager_.unload();
    LOG_DEBUG(<< "Finished destroying the application.");
}

void Application::load()
{
    LOG_DEBUG(<< "Start creating the application.");

    // Status bar.
    // statusBar()->showMessage(tr("Ready"));

    // Plugins.
    pluginManager_.load(this);

    // Exit.
    // createAction(&exitAction_,
    //              "File",
    //              "",
    //              tr("E&xit"),
    //              tr("Exit the application"),
    //              ThemeIcon(),
    //              this,
    //              SLOT(close()),
    //              MAIN_WINDOW_MENU_FILE_PRIORITY,
    //              100);
    // exitAction_->setShortcuts(QKeySequence::Quit);

    // Menu.
    createMenu();

    // initialize icons according to current theme
    onThemeChanged();

    // update when theme changes
    // connect(qApp->styleHints(),
    //        &QStyleHints::colorSchemeChanged,
    //        this,
    //        &Application::onThemeChanged);

    // Rendering.
    threadRender_.setCallback(this);
    threadRender_.create();

    // Update.
    setWindowTitle(editor_.projectPath());

    emitUpdate(this, {});

    if (pluginManager_.viewer())
    {
        if (interactive_)
        {
            pluginManager_.viewer()->resetSceneView();
        }
    }
    else
    {
        LOG_ERROR(<< "The viewer plugin is not loaded to perform the action.");
    }

    LOG_DEBUG(<< "Finished creating the application.");
}

void Application::setWindowTitle(const std::string &path)
{
#if 0
    std::string newtitle = APPLICATION_NAME;

    if (path != "")
    {
        newtitle = newtitle + " - " + path;
    }

    QMainWindow::setWindowTitle(newtitle + " [*]");
#endif
}

void Application::showError(const char *message)
{
    //(void)QMessageBox::critical(this, tr("Error"), message);
}

void Application::onThemeChanged()
{
}

bool Application::onClose()
{
#if 0
    if (!projectFilePlugin_)
    {
        LOG_ERROR(<< "The project file plugin is not loaded"
                     " to perform the action.");
        event->accept();
        return;
    }

    if (projectFilePlugin_->closeProject())
    {
        return true;
    }

    return false;
#else
    return true;
#endif
}

void Application::importFile()
{
#if 0
    if (!importFilePlugin_)
    {
        LOG_ERROR(<< "The import file plugin is not loaded"
                     " to perform the action.");
        return;
    }

    importFilePlugin_->importFile();
#endif
}

void Application::suspendThreads()
{
    LOG_DEBUG_RENDER(<< "Suspend threads.");

    threadRender_.cancel();
}

void Application::resumeThreads()
{
    LOG_DEBUG_RENDER(<< "Resume threads.");

    slotRenderViewports();
}

void Application::threadProgress(bool finished)
{
    (void)finished;

    LOG_DEBUG_RENDER(<< "Thread progress finished <" << finished << ">.");

    requestRenderFromAnyThread();
}

void Application::requestRenderFromAnyThread()
{
    if (renderPending_.exchange(true))
    {
        return;
    }

    // QMetaObject::invokeMethod(
    //     this,
    //     [this]
    //     {
    //         renderPending_.store(false);
    //         slotRender();
    //     },
    //     Qt::QueuedConnection);
}

void Application::slotRender()
{
    LOG_DEBUG_RENDER(<< "Start rendering.");
    double t1 = Time::realTime();

    if (interactive_ && pluginManager_.viewer())
    {
        pluginManager_.viewer()->updateScene();
    }

    double t2 = Time::realTime();
    LOG_DEBUG_RENDER(<< "Finished rendering after <" << (t2 - t1)
                     << "> seconds.");
}

void Application::slotRenderViewport(size_t viewportId)
{
    LOG_DEBUG_RENDER(<< "Render viewport <" << viewportId << ">.");

    if (interactive_ && pluginManager_.viewer())
    {
        threadRender_.render(pluginManager_.viewer()->camera(viewportId));
    }
}

void Application::slotRenderViewports()
{
    LOG_DEBUG_RENDER(<< "Render viewports.");

    if (interactive_ && pluginManager_.viewer())
    {
        threadRender_.render(pluginManager_.viewer()->camera());
    }
}

void Application::post(std::function<void()> fn)
{
    eventQueue_.post(fn);
}

void Application::processEvents()
{
    // eventQueue_.runOne();
    // QCoreApplication::processEvents();
}

void Application::emitUpdate(void *sender, int type)
{
    LOG_DEBUG_UPDATE(<< "Update target <" << type << "> emit.");

    // emit signalUpdate(sender, target);
}

void Application::update(void *sender,
                         int type,
                         Page::State viewPortsCacheState,
                         bool resetCamera)
{
    LOG_DEBUG_UPDATE(<< "Update target <" << type << "> set page state <"
                     << viewPortsCacheState << "> reset camera <"
                     << static_cast<int>(resetCamera) << ">.");

    suspendThreads();

    editor_.viewports().setState(viewPortsCacheState);
    // editor_.viewports().clearContent();

    if (resetCamera)
    {
        if (interactive_ && pluginManager_.viewer())
        {
            pluginManager_.viewer()->resetScene();
        }
    }

    emitUpdate(sender, type);

    resumeThreads();
}

void Application::updateNewProject()
{
    LOG_DEBUG(<< "Start updating new project.");

    setWindowTitle(editor_.projectPath());

    if (interactive_ && pluginManager_.viewer())
    {
        std::unique_lock<std::mutex> mutexlock(editor_.editorMutex_);
        pluginManager_.viewer()->resetSceneView();
    }

    emitUpdate(this, {});

    LOG_DEBUG(<< "Finished updating new project.");
}

void Application::updateData()
{
    LOG_DEBUG_UPDATE(<< "Update data.");

    suspendThreads();

    if (interactive_ && pluginManager_.viewer())
    {
        pluginManager_.viewer()->resetScene();
    }

    editor_.viewports().clearContent();
    editor_.applyFilters();

    resumeThreads();
}

void Application::updateFilter(void *sender, bool final)
{
    LOG_DEBUG_UPDATE(<< "Update filter.");

    suspendThreads();

    if (interactive_ && pluginManager_.viewer())
    {
        pluginManager_.viewer()->resetScene();
    }

    editor_.viewports().setState(Page::STATE_SELECT);

    if (final)
    {
        emitUpdate(sender, {Message::TYPE_FILTER});
    }

    resumeThreads();
}

void Application::updateModifiers()
{
    LOG_DEBUG_UPDATE(<< "Update modifiers.");

    suspendThreads();

    editor_.viewports().setState(Page::STATE_RUN_MODIFIERS);

    resumeThreads();
}

void Application::updateRender()
{
    LOG_DEBUG_UPDATE(<< "Update render.");

    suspendThreads();

    editor_.viewports().setState(Page::STATE_RENDER);

    resumeThreads();
}

void Application::createAction(Action **result,
                               const std::string &menuTitle,
                               const std::string &toolBarTitle,
                               const std::string &text,
                               const std::string &toolTip,
                               const ThemeIcon &icon,
                               std::function<void()> cb,
                               int menuPriority,
                               int menuItemPriority)
{
#if 0
    LOG_DEBUG(<< "Create action menu <" << menuTitle.toStdString()
              << "> toolBar <" << toolBarTitle.toStdString() << "> text <"
              << text.toStdString() << "> priority <" << menuPriority << "/"
              << menuItemPriority << "> icons <" << themeIcon.toQString()
              << ">.");

    QAction *action;

    // Create action.
    action = new QAction(text, this);

    if (!toolTip.isEmpty())
    {
        action->setToolTip(toolTip);
        action->setStatusTip(toolTip);
    }

    QIcon icon = themeIcon.icon(isDarkMode());
    if (!icon.isNull())
    {
        action->setIcon(icon);

        IconEntry ie;
        ie.action = action;
        ie.themeIcon = themeIcon;

        icons_.push_back(ie);
    }

    // Connect action.
    if (receiver && member)
    {
        connect(action, SIGNAL(triggered()), receiver, member);
    }

    // Add action to menu.
    Application::MenuItem menuItem;
    menuItem.action = action;
    menuItem.title = text;
    menuItem.toolBarTitle = toolBarTitle;
    menuItem.priority = menuItemPriority;

    if (!menuIndex_.contains(menuTitle))
    {
        Application::Menu menu;
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
        Application::Menu &menu = menus_[menuIndex_[menuTitle]];

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
#endif
}

void Application::createToolButton(ToolButton **result,
                                   const std::string &text,
                                   const std::string &toolTip,
                                   const ThemeIcon &themeIcon,
                                   std::function<void()> callback)
{
#if 0
    QToolButton *button;

    // Create button.
    button = new QToolButton;
    button->setText(text);
    button->setToolTip(toolTip);
    button->setStatusTip(toolTip);
    button->setEnabled(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QIcon icon = themeIcon.icon(isDarkMode());
    if (!icon.isNull())
    {
        button->setIcon(icon);

        IconEntry ie;
        ie.button = button;
        ie.themeIcon = themeIcon;

        icons_.push_back(ie);
    }

    // Connect button.
    if (receiver && member)
    {
        connect(button, SIGNAL(clicked()), receiver, member);
    }

    // Return value.
    *result = button;
#endif
}

void Application::createMenu()
{
#if 0
    // Sort menu.
    std::sort(menus_.begin(),
              menus_.end(),
              [](const Application::Menu &a, const Application::Menu &b)
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
            [](const Application::MenuItem &a, const Application::MenuItem &b)
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

        std::string previousToolBarTitle;
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
#endif
}

void Application::addNavigationItem(Plugin *owner,
                                    const std::vector<NavigationPathItem> &path,
                                    Action *action,
                                    int order)
{
    navigation_.addItem(owner, path, action, order);
}

void Application::removeNavigationItem(Action *action)
{
    navigation_.removeItem(action);
}

void Application::removeNavigationItems(Plugin *owner)
{
    navigation_.removeItems(owner);
}

void Application::setViewer(Widget *widget)
{
}

void Application::removeViewer(Widget *widget)
{
}
