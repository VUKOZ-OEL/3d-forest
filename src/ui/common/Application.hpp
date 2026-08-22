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

/** @file Application.hpp */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

// Include std.
#include <atomic>
#include <set>

// Include 3D Forest.
#include <Action.hpp>
#include <DockWidget.hpp>
#include <Editor.hpp>
#include <EventQueue.hpp>
#include <MenuBar.hpp>
#include <NavigationTree.hpp>
#include <PluginManager.hpp>
#include <RenderThread.hpp>
#include <ThemeIcon.hpp>
#include <ThreadCallbackInterface.hpp>
#include <ToolButton.hpp>
#include <Translation.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

#define MAIN_WINDOW_MENU_FILE_PRIORITY 10
#define MAIN_WINDOW_MENU_EDIT_PRIORITY 20
#define MAIN_WINDOW_MENU_DATA_PRIORITY 30
#define MAIN_WINDOW_MENU_COMPUTE_PRIORITY 40
#define MAIN_WINDOW_MENU_FILTER_PRIORITY 50
#define MAIN_WINDOW_MENU_EXTERNAL_PRIORITY 52
#define MAIN_WINDOW_MENU_VIEWPORT_PRIORITY 55
#define MAIN_WINDOW_MENU_SETTINGS_PRIORITY 60
#define MAIN_WINDOW_MENU_HELP_PRIORITY 70

/** Application. */
class EXPORT_UI_COMMON Application : public ThreadCallbackInterface
{
public:
    static inline const std::string APPLICATION_NAME{"1.0"};
    static inline const std::string APPLICATION_VERSION{"3D Forest"};
    static const int ICON_SIZE{16};
    static const int ICON_SIZE_TEXT{16};

    Application();
    virtual ~Application();

    void load();

    Editor &editor() { return editor_; }

    void setWindowTitle(const std::string &path);
    void showError(const char *message);

    void onThemeChanged();
    bool onClose();

    void importFile();

    void createAction(Action **result,
                      const std::string &menuTitle,
                      const std::string &toolBarTitle,
                      const std::string &text,
                      const std::string &toolTip,
                      const ThemeIcon &icon,
                      std::function<void()> cb,
                      int menuPriority = -1,
                      int menuItemPriority = -1);

    void createToolButton(ToolButton **result,
                          const std::string &text,
                          const std::string &toolTip,
                          const ThemeIcon &themeIcon,
                          std::function<void()> callback = {});

    virtual void addNavigationItem(Plugin *owner,
                                   const std::vector<NavigationPathItem> &path,
                                   Action *action,
                                   int order = 0);
    virtual void removeNavigationItem(Action *action);
    void removeNavigationItems(Plugin *owner);
    NavigationTree &navigation() { return navigation_; }
    const NavigationTree &navigation() const { return navigation_; }

    virtual void setViewer(Widget *widget);
    virtual void removeViewer(Widget *widget);

    void suspendThreads();
    void resumeThreads();
    virtual void threadProgress(bool finished) override;

    void post(std::function<void()> fn);
    virtual void processEvents();

    void emitUpdate(void *sender, int type);
    void update(void *sender,
                int type,
                Page::State viewPortsCacheState = Page::STATE_SELECT,
                bool resetCamera = false);

    /// Call when the whole project was opened or closed.
    void updateNewProject();

    /// Clear cached point data and start new rendering.
    void updateData();

    /// Reset selection of cached point data and start new rendering.
    void updateFilter(void *sender = nullptr, bool final = true);

    /// Reset modifiers of cached point data and start new rendering.
    void updateModifiers();

    /// Reset rendered state of cached point data and start new rendering.
    void updateRender();

    /// Call rendering from another thread.
    void requestRenderFromAnyThread();

    /// Calls paint() on all viewports.
    void slotRender();

    /// Updates new data in specified viewport.
    void slotRenderViewport(size_t viewportId);

    /// Updates new data in all viewports.
    void slotRenderViewports();

    /// Connect to this signal in your plugin to be notified about data changes.
    // void signalUpdate(void *sender, const std::set<Editor::Type> &target);
    Signal<Message> signalUpdate;

    // bool event(QEvent *e) override;
    // void paintEvent(QPaintEvent *event) override;
    // void resizeEvent(QResizeEvent *event) override;
    // void showEvent(QShowEvent *event) override;
    // void hideEvent(QHideEvent *event) override;
    // void closeEvent(QCloseEvent *event) override;

private:
    Editor editor_;

    RenderThread threadRender_;
    std::atomic_bool renderPending_{false};
    bool interactive_{false};

    PluginManager pluginManager_;

    NavigationTree navigation_;
    MenuBar menuBar_;
    EventQueue eventQueue_;

    void createMenu();
};

#include <WarningsEnable.hpp>

#endif /* APPLICATION_HPP */
