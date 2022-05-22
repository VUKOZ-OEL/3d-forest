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

/** @file GuiWindowMain.hpp */

#ifndef GUI_WINDOW_MAIN_HPP
#define GUI_WINDOW_MAIN_HPP

#include <Editor.hpp>
#include <EditorThreadRender.hpp>
#include <ThreadCallbackInterface.hpp>

#include <QHash>
#include <QIcon>
#include <QMainWindow>
#include <QAction>
//#include <QtWidgets/QAction> Fix for qt5 which has two QAction classes

class GuiPluginInterface;
class GuiPluginImport;
class GuiProjectPlugin;
class GuiPluginProjectFile;
class GuiPluginViewer;

class QToolButton;

/** Gui Window Main. */
class GuiWindowMain : public QMainWindow, public ThreadCallbackInterface
{
    Q_OBJECT

public:
    static const char *APPLICATION_NAME;
    static const char *APPLICATION_VERSION;
    static const int ICON_SIZE;

    GuiWindowMain(QWidget *parent = nullptr);
    virtual ~GuiWindowMain();

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
    virtual void threadProgress(bool finished);

    void updateEverything();
    void updateData();
    void updateSelection();

    Editor &editor() { return editor_; }

public slots:
    void slotRender();
    void slotRenderViewport();
    void slotRenderViewport(size_t viewportId);

signals:
    void signalRender();
    void signalUpdate();

protected:
    void closeEvent(QCloseEvent *event) override;
    void loadPlugins();
    void loadPlugin(QObject *plugin);
    void createMenuSeparator(const QString &menu);

    // Editor
    Editor editor_;
    EditorThreadRender threadRender_;

    // Gui
    GuiPluginImport *guiPluginImport_;
    GuiProjectPlugin *guiPluginProject_;
    GuiPluginProjectFile *guiPluginProjectFile_;
    GuiPluginViewer *guiPluginViewer_;
    std::vector<GuiPluginInterface *> plugins_;

    // Menu
    QHash<QString, QMenu *> menu_;
    QHash<QString, QToolBar *> toolBar_;

    QAction *actionExit_;
};

#endif /* GUI_WINDOW_MAIN_HPP */
