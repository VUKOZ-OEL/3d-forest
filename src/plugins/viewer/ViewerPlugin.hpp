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

/** @file ViewerPlugin.hpp */

#ifndef VIEWER_PLUGIN_HPP
#define VIEWER_PLUGIN_HPP

// Include 3D Forest.
#include <PluginInterface.hpp>
#include <ViewerInterface.hpp>
#include <ViewerViewports.hpp>

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestViewerPlugin)
        #define EXPORT_VIEWER_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_VIEWER_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_VIEWER_PLUGIN
#endif

/** Viewer Plugin. */
class EXPORT_VIEWER_PLUGIN ViewerPlugin : public QObject,
                                          public PluginInterface,
                                          public ViewerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    ViewerPlugin();

    virtual void initialize(MainWindow *mainWindow);

    virtual std::vector<Camera> camera(size_t viewportId) const;
    virtual std::vector<Camera> camera() const;

    virtual void updateScene(Editor *editor);
    virtual void resetScene(Editor *editor, bool resetView);

public slots:
    void slotViewOrthographic();
    void slotViewPerspective();

    void slotViewTop();
    void slotViewFront();
    void slotViewRight();
    void slotView3d();

    void slotViewResetDistance();
    void slotViewResetCenter();

    void slotViewLayoutSingle();
    void slotViewLayoutTwoColumns();
    void slotViewLayoutGrid();
    void slotViewLayoutThreeRowsRight();

    void slotViewLayout(ViewerViewports::ViewLayout layout);

private:
    MainWindow *mainWindow_;

    QAction *viewOrthographicAction_;
    QAction *viewPerspectiveAction_;

    QAction *viewTopAction_;
    QAction *viewFrontAction_;
    QAction *viewRightAction_;
    QAction *view3dAction_;

    QAction *viewResetDistanceAction_;
    QAction *viewResetCenterAction_;

    QAction *viewLayoutSingleAction_;
    QAction *viewLayoutTwoColumnsAction_;
    QAction *viewLayoutGridAction_;
    QAction *viewLayoutThreeRowsRightAction_;

    ViewerViewports *viewports_;

    void updateViewer();
};

#endif /* VIEWER_PLUGIN_HPP */
