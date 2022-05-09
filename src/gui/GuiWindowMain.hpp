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
#include <QtWidgets/QAction>

class GuiPluginInterface;
class GuiPluginImport;
class GuiPluginProjectFile;
class GuiPluginViewer;

#define GUI_ICON_THEME "-fluency-48"

/** Gui Window Main. */
class GuiWindowMain : public QMainWindow, public ThreadCallbackInterface
{
    Q_OBJECT

public:
    static const char *APPLICATION_NAME;
    static const char *APPLICATION_VERSION;

    GuiWindowMain(QWidget *parent = nullptr);
    virtual ~GuiWindowMain();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void showError(const char *message);
    void setWindowTitle(const QString &path);

    void createAction(QAction **result,
                      const QString &menu,
                      const QString &text,
                      const QString &toolTip,
                      const QIcon &icon,
                      const QObject *receiver,
                      const char *member,
                      bool useToolBar);

    void createSeparator(const QString &menu);
    void hideToolBar(const QString &menu);

    void cancelThreads();
    void resumeThreads();
    virtual void threadProgress(bool finished);

    void updateEverything();

    Editor &editor() { return editor_; }

public slots:
    void slotRender();
    void slotRenderViewport();
    void slotRenderViewport(size_t viewportId);

signals:
    void signalRender();

protected:
    void closeEvent(QCloseEvent *event) override;
    void loadPlugins();
    void loadPlugin(QObject *plugin);

    // Editor
    Editor editor_;
    EditorThreadRender threadRender_;

    // Gui
    GuiPluginImport *guiPluginImport_;
    GuiPluginProjectFile *guiPluginProjectFile_;
    GuiPluginViewer *guiPluginViewer_;
    std::vector<GuiPluginInterface *> plugins_;

    // Menu
    QHash<QString, QMenu *> menu_;
    QHash<QString, QToolBar *> toolBar_;

    QAction *actionExit_;
};

#endif /* GUI_WINDOW_MAIN_HPP */
