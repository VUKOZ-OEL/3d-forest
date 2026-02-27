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

/** @file ViewerPlugin.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <ViewerPlugin.hpp>
#include <ViewerViewports.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewerPlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ViewerResources/", name))

ViewerPlugin::ViewerPlugin() : mainWindow_(nullptr)
{
}

void ViewerPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    viewports_ = new ViewerViewports(mainWindow_);
    mainWindow_->setCentralWidget(viewports_);

    connect(viewports_,
            SIGNAL(cameraChanged(size_t)),
            mainWindow_,
            SLOT(slotRenderViewport(size_t)));

    mainWindow_->createAction(&viewOrthographicAction_,
                              "Viewport",
                              "Viewport Projection",
                              tr("Orthographic"),
                              tr("Orthographic projection"),
                              ICON("orthographic-wire"),
                              this,
                              SLOT(slotViewOrthographic()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewPerspectiveAction_,
                              "Viewport",
                              "Viewport Projection",
                              tr("Perspective"),
                              tr("Perspective projection"),
                              ICON("perspective-wire"),
                              this,
                              SLOT(slotViewPerspective()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&view2dAction_,
                              "Viewport",
                              "Viewport Projection",
                              tr("2D DBH"),
                              tr("2D projection with DBH"),
                              ICON("view-2d"),
                              this,
                              SLOT(slotView2d()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&view3dAction_,
                              "Viewport",
                              "Viewport",
                              tr("3d view"),
                              tr("3d view"),
                              ICON("portraits-fill"),
                              this,
                              SLOT(slotView3d()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewTopAction_,
                              "Viewport",
                              "Viewport",
                              tr("Top view"),
                              tr("Top view"),
                              ICON("view-top"),
                              this,
                              SLOT(slotViewTop()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewFrontAction_,
                              "Viewport",
                              "Viewport",
                              tr("Front view"),
                              tr("Front view"),
                              ICON("view-front"),
                              this,
                              SLOT(slotViewFront()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewRightAction_,
                              "Viewport",
                              "Viewport",
                              tr("Right view"),
                              tr("Right view"),
                              ICON("view-right"),
                              this,
                              SLOT(slotViewRight()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewResetDistanceAction_,
                              "Viewport",
                              "Viewport",
                              tr("Reset distance"),
                              tr("Reset distance"),
                              ICON("fit-to-page"),
                              this,
                              SLOT(slotViewResetDistance()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewResetCenterAction_,
                              "Viewport",
                              "Viewport",
                              tr("Reset center"),
                              tr("Reset center"),
                              ICON("collect"),
                              this,
                              SLOT(slotViewResetCenter()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewLayoutSingleAction_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Single layout"),
                              tr("Single layout"),
                              ICON("layout-single"),
                              this,
                              SLOT(slotViewLayoutSingle()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewLayoutTwoColumnsAction_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Column layout"),
                              tr("Layout with two columns"),
                              ICON("layout-columns"),
                              this,
                              SLOT(slotViewLayoutTwoColumns()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewLayoutGridAction_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Grid layout"),
                              tr("Grid layout"),
                              ICON("layout-grid"),
                              this,
                              SLOT(slotViewLayoutGrid()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    mainWindow_->createAction(&viewLayoutThreeRowsRightAction_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Grid layout 3"),
                              tr("Grid layout with 3 rows"),
                              ICON("layout-grid-3-right"),
                              this,
                              SLOT(slotViewLayoutThreeRowsRight()),
                              MAIN_WINDOW_MENU_VIEWPORT_PRIORITY);

    // mainWindow_->hideToolBar("Viewport Projection");
    mainWindow_->hideToolBar("Viewport Layout");
}

std::vector<Camera> ViewerPlugin::camera(size_t viewportId) const
{
    return viewports_->camera(viewportId);
}

std::vector<Camera> ViewerPlugin::camera() const
{
    return viewports_->camera();
}

void ViewerPlugin::updateScene(Editor *editor)
{
    return viewports_->updateScene(editor);
}

void ViewerPlugin::resetScene(Editor *editor, bool resetView)
{
    return viewports_->resetScene(editor, resetView);
}

void ViewerPlugin::slotViewOrthographic()
{
    viewports_->setViewOrthographic();
    updateViewer();
}

void ViewerPlugin::slotViewPerspective()
{
    viewports_->setViewPerspective();
    updateViewer();
}

void ViewerPlugin::slotView2d()
{
    viewports_->setView2d();
    updateViewer();
}

void ViewerPlugin::slotViewTop()
{
    viewports_->setViewTop();
    updateViewer();
}

void ViewerPlugin::slotViewFront()
{
    viewports_->setViewFront();
    updateViewer();
}

void ViewerPlugin::slotViewRight()
{
    viewports_->setViewRight();
    updateViewer();
}

void ViewerPlugin::slotView3d()
{
    viewports_->setView3d();
    updateViewer();
}

void ViewerPlugin::slotViewResetDistance()
{
    viewports_->setViewResetDistance();
    updateViewer();
}

void ViewerPlugin::slotViewResetCenter()
{
    viewports_->setViewResetCenter();
    updateViewer();
}

void ViewerPlugin::slotViewLayoutSingle()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_SINGLE);
}

void ViewerPlugin::slotViewLayoutTwoColumns()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_TWO_COLUMNS);
}

void ViewerPlugin::slotViewLayoutGrid()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_GRID);
}

void ViewerPlugin::slotViewLayoutThreeRowsRight()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT);
}

void ViewerPlugin::slotViewLayout(ViewerViewports::ViewLayout layout)
{
    LOG_DEBUG(<< "Set layout <" << layout << ">.");

    mainWindow_->suspendThreads();

    if (layout == ViewerViewports::VIEW_LAYOUT_SINGLE)
    {
        mainWindow_->editor().viewportsResize(1);
        viewports_->setLayout(layout);
    }
    else if (layout == ViewerViewports::VIEW_LAYOUT_TWO_COLUMNS)
    {
        mainWindow_->editor().viewportsResize(2);
        viewports_->setLayout(layout);
        viewports_->resetViewport(&mainWindow_->editor(), 1, true);
    }
    else if ((layout == ViewerViewports::VIEW_LAYOUT_GRID) ||
             (layout == ViewerViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT))
    {
        mainWindow_->editor().viewportsResize(4);
        viewports_->setLayout(layout);
        viewports_->resetViewport(&mainWindow_->editor(), 1, true);
        viewports_->resetViewport(&mainWindow_->editor(), 2, true);
        viewports_->resetViewport(&mainWindow_->editor(), 3, true);
    }

    updateViewer();
}

void ViewerPlugin::updateViewer()
{
    mainWindow_->slotRenderViewports();
}
