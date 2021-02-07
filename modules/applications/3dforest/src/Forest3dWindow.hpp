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
    @file Forest3dWindow.hpp
*/

#ifndef FOREST_3D_WINDOW_HPP
#define FOREST_3D_WINDOW_HPP

#include <Editor.hpp>
#include <QMainWindow>
#include <vector>

class Forest3dClipFilter;
class Forest3dDataSets;
class Forest3dLayers;
class Forest3dPluginFile;
class Forest3dPluginTool;
class GLViewer;

/** Forest 3d Main Window. */
class Forest3dWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const QString APPLICATION_NAME;
    static const QString APPLICATION_VERSION;

    explicit Forest3dWindow(QWidget *parent = nullptr);
    ~Forest3dWindow();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

public slots:
    void actionProjectNew();
    void actionProjectOpen();
    void actionProjectSave();
    void actionProjectSaveAs();
    void actionProjectImport();
    void actionProjectExportAs();
    void actionViewLayoutSingle();
    void actionViewLayoutTwoColumns();
    void actionPluginToolStart();
    void actionDataSetVisible(size_t id, bool checked);
    void actionLayerVisible(size_t id, bool checked);
    void actionClipFilter(const ClipFilter &clipFilter);
    void actionAbout();

protected:
    // Data
    Editor editor_;

    // View
    GLViewer *viewer_;

    // Plugins
    std::vector<Forest3dPluginFile *> pluginsFile_;
    std::vector<Forest3dPluginTool *> pluginsTool_;

    // Windows
    Forest3dDataSets *windowDataSets_;
    Forest3dLayers *windowLayers_;
    Forest3dClipFilter *windowClipFilter_;

    // Menus
    QMenu *menuTools_;
    QMenu *menuWindows_;

    // Timers
    int timerNewData_;

    // Initialization
    void initializeWindow();
    void createViewer();
    void createMenus();
    void createWindows();
    void createPlugins();

    // Project
    bool projectOpen(const QString &path);
    bool projectClose();
    bool projectSave(const QString &path = "");

    // Update
    void updateProject();
    void updateViewer();

    // Utilities
    void showError(const char *message);
    void updateWindowTitle(const QString &path);

    // Events
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif /* FOREST_3D_WINDOW_HPP */
