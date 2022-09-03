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

#include <Log.hpp>

#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <ViewerPlugin.hpp>
#include <ViewerViewports.hpp>

#define ICON(name) (ThemeIcon(":/viewer/", name))

ViewerPlugin::ViewerPlugin() : mainWindow_(nullptr)
{
}

void ViewerPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    viewports_ = new ViewerViewports(mainWindow_);
    mainWindow_->setCentralWidget(viewports_);

    mainWindow_->createAction(&actionViewOrthographic_,
                              "Viewport",
                              "Viewport Projection",
                              tr("Orthographic"),
                              tr("Orthographic projection"),
                              ICON("orthographic_wire"),
                              this,
                              SLOT(slotViewOrthographic()));

    mainWindow_->createAction(&actionViewPerspective_,
                              "Viewport",
                              "Viewport Projection",
                              tr("Perspective"),
                              tr("Perspective projection"),
                              ICON("perspective_wire"),
                              this,
                              SLOT(slotViewPerspective()));

    mainWindow_->createAction(&actionViewTop_,
                              "Viewport",
                              "Viewport",
                              tr("Top view"),
                              tr("Top view"),
                              ICON("view_top"),
                              this,
                              SLOT(slotViewTop()));

    mainWindow_->createAction(&actionViewFront_,
                              "Viewport",
                              "Viewport",
                              tr("Front view"),
                              tr("Front view"),
                              ICON("view_front"),
                              this,
                              SLOT(slotViewFront()));

    mainWindow_->createAction(&actionViewRight_,
                              "Viewport",
                              "Viewport",
                              tr("Right view"),
                              tr("Right view"),
                              ICON("view_right"),
                              this,
                              SLOT(slotViewRight()));

    mainWindow_->createAction(&actionView3d_,
                              "Viewport",
                              "Viewport",
                              tr("3d view"),
                              tr("3d view"),
                              ICON("portraits"),
                              this,
                              SLOT(slotView3d()));

    mainWindow_->createAction(&actionViewResetDistance_,
                              "Viewport",
                              "Viewport",
                              tr("Reset distance"),
                              tr("Reset distance"),
                              ICON("fit_to_page"),
                              this,
                              SLOT(slotViewResetDistance()));

    mainWindow_->createAction(&actionViewResetCenter_,
                              "Viewport",
                              "Viewport",
                              tr("Reset center"),
                              tr("Reset center"),
                              ICON("collect"),
                              this,
                              SLOT(slotViewResetCenter()));

    mainWindow_->createAction(&actionViewLayoutSingle_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Single layout"),
                              tr("Single layout"),
                              ICON("layout_single"),
                              this,
                              SLOT(slotViewLayoutSingle()));

    mainWindow_->createAction(&actionViewLayoutTwoColumns_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Column layout"),
                              tr("Layout with two columns"),
                              ICON("layout_columns"),
                              this,
                              SLOT(slotViewLayout2Columns()));

    mainWindow_->createAction(&actionViewLayoutGrid_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Grid layout"),
                              tr("Grid layout"),
                              ICON("layout_grid_2"),
                              this,
                              SLOT(slotViewLayoutGrid()));

    mainWindow_->createAction(&actionViewLayoutGridRight_,
                              "Viewport",
                              "Viewport Layout",
                              tr("Grid layout 3"),
                              tr("Grid layout with 3 rows"),
                              ICON("layout_grid_3_right"),
                              this,
                              SLOT(slotViewLayout3RowsRight()));

    // mainWindow_->hideToolBar("Viewport Projection");
    // mainWindow_->hideToolBar("Viewport Layout");
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

void ViewerPlugin::slotViewLayout2Columns()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_TWO_COLUMNS);
}

void ViewerPlugin::slotViewLayoutGrid()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_GRID);
}

void ViewerPlugin::slotViewLayout3RowsRight()
{
    slotViewLayout(ViewerViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT);
}

void ViewerPlugin::slotViewLayout(ViewerViewports::ViewLayout layout)
{
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
        viewports_->resetScene(&mainWindow_->editor(), 1, true);
    }
    else if ((layout == ViewerViewports::VIEW_LAYOUT_GRID) ||
             (layout == ViewerViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT))
    {
        mainWindow_->editor().viewportsResize(4);
        viewports_->setLayout(layout);
        viewports_->resetScene(&mainWindow_->editor(), 1, true);
        viewports_->resetScene(&mainWindow_->editor(), 2, true);
        viewports_->resetScene(&mainWindow_->editor(), 3, true);
    }

    updateViewer();
}

void ViewerPlugin::updateViewer()
{
    mainWindow_->slotRenderViewport();
}
