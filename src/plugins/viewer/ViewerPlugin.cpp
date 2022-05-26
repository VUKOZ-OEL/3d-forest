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

#include <GuiIconTheme.hpp>
#include <GuiMainWindow.hpp>
#include <ViewerPlugin.hpp>
#include <ViewerViewports.hpp>

#define ICON(name) (GuiIconTheme(":/viewer/", name))

ViewerPlugin::ViewerPlugin(GuiMainWindow *mainWindow)
    : QObject(mainWindow),
      mainWindow_(mainWindow)
{
    guiViewports_ = new ViewerViewports(mainWindow_);
    mainWindow_->setCentralWidget(guiViewports_);

    mainWindow_->createAction(&actionViewOrthographic_,
                              "View",
                              "View Projection",
                              tr("Orthographic"),
                              tr("Orthographic projection"),
                              ICON("empty_box_ortho"),
                              this,
                              SLOT(slotViewOrthographic()));

    mainWindow_->createAction(&actionViewPerspective_,
                              "View",
                              "View Projection",
                              tr("Perspective"),
                              tr("Perspective projection"),
                              ICON("empty_box"),
                              this,
                              SLOT(slotViewPerspective()));

    mainWindow_->createAction(&actionViewTop_,
                              "View",
                              "View",
                              tr("Top view"),
                              tr("Top view"),
                              ICON("view_top"),
                              this,
                              SLOT(slotViewTop()));

    mainWindow_->createAction(&actionViewFront_,
                              "View",
                              "View",
                              tr("Front view"),
                              tr("Front view"),
                              ICON("view_front"),
                              this,
                              SLOT(slotViewFront()));

    mainWindow_->createAction(&actionViewRight_,
                              "View",
                              "View",
                              tr("Right view"),
                              tr("Right view"),
                              ICON("view_right"),
                              this,
                              SLOT(slotViewRight()));

    mainWindow_->createAction(&actionView3d_,
                              "View",
                              "View",
                              tr("3d view"),
                              tr("3d view"),
                              ICON("portraits"),
                              this,
                              SLOT(slotView3d()));

    mainWindow_->createAction(&actionViewResetDistance_,
                              "View",
                              "View",
                              tr("Reset distance"),
                              tr("Reset distance"),
                              ICON("fit_to_page"),
                              this,
                              SLOT(slotViewResetDistance()));

    mainWindow_->createAction(&actionViewResetCenter_,
                              "View",
                              "View",
                              tr("Reset center"),
                              tr("Reset center"),
                              ICON("collect"),
                              this,
                              SLOT(slotViewResetCenter()));

    mainWindow_->createAction(&actionViewLayoutSingle_,
                              "View",
                              "View Layout",
                              tr("Single layout"),
                              tr("Single layout"),
                              ICON("layout_single"),
                              this,
                              SLOT(slotViewLayoutSingle()));

    mainWindow_->createAction(&actionViewLayoutTwoColumns_,
                              "View",
                              "View Layout",
                              tr("Column layout"),
                              tr("Layout with two columns"),
                              ICON("layout_columns"),
                              this,
                              SLOT(slotViewLayout2Columns()));

    mainWindow_->createAction(&actionViewLayoutGrid_,
                              "View",
                              "View Layout",
                              tr("Grid layout"),
                              tr("Grid layout"),
                              ICON("layout_grid_2"),
                              this,
                              SLOT(slotViewLayoutGrid()));

    mainWindow_->createAction(&actionViewLayoutGridRight_,
                              "View",
                              "View Layout",
                              tr("Grid layout 3"),
                              tr("Grid layout with 3 rows"),
                              ICON("layout_grid_3_right"),
                              this,
                              SLOT(slotViewLayout3RowsRight()));

    mainWindow_->hideToolBar("View Projection");
    mainWindow_->hideToolBar("View Layout");
}

void ViewerPlugin::slotViewOrthographic()
{
    guiViewports_->setViewOrthographic();
    updateViewer();
}

void ViewerPlugin::slotViewPerspective()
{
    guiViewports_->setViewPerspective();
    updateViewer();
}

void ViewerPlugin::slotViewTop()
{
    guiViewports_->setViewTop();
    updateViewer();
}

void ViewerPlugin::slotViewFront()
{
    guiViewports_->setViewFront();
    updateViewer();
}

void ViewerPlugin::slotViewRight()
{
    guiViewports_->setViewRight();
    updateViewer();
}

void ViewerPlugin::slotView3d()
{
    guiViewports_->setView3d();
    updateViewer();
}

void ViewerPlugin::slotViewResetDistance()
{
    guiViewports_->setViewResetDistance();
    updateViewer();
}

void ViewerPlugin::slotViewResetCenter()
{
    guiViewports_->setViewResetCenter();
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
        guiViewports_->setLayout(layout);
    }
    else if (layout == ViewerViewports::VIEW_LAYOUT_TWO_COLUMNS)
    {
        mainWindow_->editor().viewportsResize(2);
        guiViewports_->setLayout(layout);
        guiViewports_->resetScene(&mainWindow_->editor(), 1, true);
    }
    else if ((layout == ViewerViewports::VIEW_LAYOUT_GRID) ||
             (layout == ViewerViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT))
    {
        mainWindow_->editor().viewportsResize(4);
        guiViewports_->setLayout(layout);
        guiViewports_->resetScene(&mainWindow_->editor(), 1, true);
        guiViewports_->resetScene(&mainWindow_->editor(), 2, true);
        guiViewports_->resetScene(&mainWindow_->editor(), 3, true);
    }

    updateViewer();
}

void ViewerPlugin::updateViewer()
{
    mainWindow_->slotRenderViewport();
}
