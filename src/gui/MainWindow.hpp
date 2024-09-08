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

/** @file MainWindow.hpp */

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

// Include std.
#include <set>

// Include 3D Forest.
#include <Editor.hpp>
#include <RenderThread.hpp>
#include <ThreadCallbackInterface.hpp>

// Include 3D Forest plugins.
class PluginInterface;
class ExportFilePlugin;
class ImportFilePlugin;
class MessageLogPlugin;
class ExplorerPlugin;
class ProjectFilePlugin;
class SettingsPlugin;
class ViewerPlugin;
class HelpPlugin;

// Include Qt.
#include <QHash>
#include <QIcon>
#include <QMainWindow>
#include <QSet>

#if QT_VERSION_MAJOR == 5
    // Fix for qt5 which has two QAction classes.
    #include <QtWidgets/QAction>
#else
    #include <QAction>
#endif

class QToolButton;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Main Window. */
class EXPORT_GUI MainWindow : public QMainWindow, public ThreadCallbackInterface
{
    Q_OBJECT

public:
    static const char *APPLICATION_NAME;
    static const char *APPLICATION_VERSION;
    static const int ICON_SIZE;
    static const int ICON_SIZE_TEXT;

    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void showError(const char *message);
    void setWindowTitle(const QString &path);

    void createAction(QAction **result,
                      const QString &menu,
                      const QString &toolBar,
                      const QString &text,
                      const QString &toolTip,
                      const QIcon &icon,
                      const QObject *receiver,
                      const char *member);

    static void createToolButton(QToolButton **result,
                                 const QString &text,
                                 const QString &toolTip,
                                 const QIcon &icon,
                                 const QObject *receiver,
                                 const char *member);

    void hideToolBar(const QString &menu);

    void suspendThreads();
    void resumeThreads();
    virtual void threadProgress(bool finished) override;

    void update(void *sender, const QSet<Editor::Type> &target);
    void update(const QSet<Editor::Type> &target,
                Page::State viewPortsCacheState = Page::STATE_READ,
                bool resetCamera = false);

    /// Call when the whole project was opened or closed.
    void updateNewProject();

    /// Clear cached point data and start new rendering.
    void updateData();

    /// Reset selection of cached point data and start new rendering.
    void updateFilter();

    /// Reset modifiers of cached point data and start new rendering.
    void updateModifiers();

    /// Reset rendered state of cached point data and start new rendering.
    void updateRender();

    Editor &editor() { return editor_; }

public slots:
    /// Calls paint() on all viewports.
    void slotRender();

    /// Updates new data in specified viewport.
    void slotRenderViewport(size_t viewportId);

    /// Updates new data in all viewports.
    void slotRenderViewports();

signals:
    /// Call rendering from another thread.
    void signalRender();

    /// Connect to this signal in your plugin to be notified about data changes.
    void signalUpdate(void *sender, const QSet<Editor::Type> &target);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void loadPlugins();
    void loadPlugin(QObject *plugin);
    void createMenuSeparator(const QString &menu);

    // Editor.
    Editor editor_;
    RenderThread threadRender_;

    // Gui.
    ImportFilePlugin *importFilePlugin_;
    ExportFilePlugin *exportFilePlugin_;
    MessageLogPlugin *messageLogPlugin_;
    ExplorerPlugin *explorerPlugin_;
    ProjectFilePlugin *projectFilePlugin_;
    SettingsPlugin *settingsPlugin_;
    ViewerPlugin *viewerPlugin_;
    HelpPlugin *helpPlugin_;
    std::vector<PluginInterface *> plugins_;

    // Menu.
    QHash<QString, QMenu *> menu_;
    QHash<QString, QToolBar *> toolBar_;

    QAction *actionExit_;
};

#include <WarningsEnable.hpp>

#endif /* MAIN_WINDOW_HPP */
