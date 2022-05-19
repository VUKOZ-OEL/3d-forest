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

/** @file GuiPluginViewer.cpp */

#include <Log.hpp>

#include <GuiIconTheme.hpp>
#include <GuiPluginViewer.hpp>
#include <GuiViewports.hpp>
#include <GuiWindowMain.hpp>

#define ICON(name) (GuiIconTheme(":/viewer/", name))

GuiPluginViewer::GuiPluginViewer(GuiWindowMain *window)
    : QObject(window),
      window_(window)
{
    guiViewports_ = new GuiViewports(window_);
    window_->setCentralWidget(guiViewports_);

    window_->createAction(&actionViewOrthographic_,
                          "View",
                          "View Projection",
                          tr("Orthographic"),
                          tr("Orthographic projection"),
                          ICON("3d-house-ortho"),
                          this,
                          SLOT(slotViewOrthographic()));

    window_->createAction(&actionViewPerspective_,
                          "View",
                          "View Projection",
                          tr("Perspective"),
                          tr("Perspective projection"),
                          ICON("3d-house-depth"),
                          this,
                          SLOT(slotViewPerspective()));

    window_->createAction(&actionViewTop_,
                          "View",
                          "View",
                          tr("Top view"),
                          tr("Top view"),
                          ICON("top-view"),
                          this,
                          SLOT(slotViewTop()));

    window_->createAction(&actionViewFront_,
                          "View",
                          "View",
                          tr("Front view"),
                          tr("Front view"),
                          ICON("front-view"),
                          this,
                          SLOT(slotViewFront()));

    window_->createAction(&actionViewRight_,
                          "View",
                          "View",
                          tr("Right view"),
                          tr("Right view"),
                          ICON("right-view"),
                          this,
                          SLOT(slotViewRight()));

    window_->createAction(&actionView3d_,
                          "View",
                          "View",
                          tr("3d view"),
                          tr("3d view"),
                          ICON("perspective-view"),
                          this,
                          SLOT(slotView3d()));

    window_->createAction(&actionViewResetDistance_,
                          "View",
                          "View",
                          tr("Reset distance"),
                          tr("Reset distance"),
                          ICON("fit-to-width"),
                          this,
                          SLOT(slotViewResetDistance()));

    window_->createAction(&actionViewResetCenter_,
                          "View",
                          "View",
                          tr("Reset center"),
                          tr("Reset center"),
                          ICON("collect"),
                          this,
                          SLOT(slotViewResetCenter()));

    window_->createAction(&actionViewLayoutSingle_,
                          "View",
                          "View Layout",
                          tr("Single layout"),
                          tr("Single layout"),
                          ICON("unchecked-checkbox"),
                          this,
                          SLOT(slotViewLayoutSingle()));

    window_->createAction(&actionViewLayoutTwoColumns_,
                          "View",
                          "View Layout",
                          tr("Column layout"),
                          tr("Layout with two columns"),
                          ICON("columns"),
                          this,
                          SLOT(slotViewLayout2Columns()));

    window_->createAction(&actionViewLayoutGrid_,
                          "View",
                          "View Layout",
                          tr("Grid layout"),
                          tr("Grid layout"),
                          ICON("grid-2"),
                          this,
                          SLOT(slotViewLayoutGrid()));

    window_->createAction(&actionViewLayoutGridRight_,
                          "View",
                          "View Layout",
                          tr("Grid layout 3"),
                          tr("Grid layout with 3 rows"),
                          ICON("grid-2-right"),
                          this,
                          SLOT(slotViewLayout3RowsRight()));

    window_->hideToolBar("View Projection");
    window_->hideToolBar("View Layout");
}

void GuiPluginViewer::slotViewOrthographic()
{
    guiViewports_->setViewOrthographic();
    updateViewer();
}

void GuiPluginViewer::slotViewPerspective()
{
    guiViewports_->setViewPerspective();
    updateViewer();
}

void GuiPluginViewer::slotViewTop()
{
    guiViewports_->setViewTop();
    updateViewer();
}

void GuiPluginViewer::slotViewFront()
{
    guiViewports_->setViewFront();
    updateViewer();
}

void GuiPluginViewer::slotViewRight()
{
    guiViewports_->setViewRight();
    updateViewer();
}

void GuiPluginViewer::slotView3d()
{
    guiViewports_->setView3d();
    updateViewer();
}

void GuiPluginViewer::slotViewResetDistance()
{
    guiViewports_->setViewResetDistance();
    updateViewer();
}

void GuiPluginViewer::slotViewResetCenter()
{
    guiViewports_->setViewResetCenter();
    updateViewer();
}

void GuiPluginViewer::slotViewLayoutSingle()
{
    slotViewLayout(GuiViewports::VIEW_LAYOUT_SINGLE);
}

void GuiPluginViewer::slotViewLayout2Columns()
{
    slotViewLayout(GuiViewports::VIEW_LAYOUT_TWO_COLUMNS);
}

void GuiPluginViewer::slotViewLayoutGrid()
{
    slotViewLayout(GuiViewports::VIEW_LAYOUT_GRID);
}

void GuiPluginViewer::slotViewLayout3RowsRight()
{
    slotViewLayout(GuiViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT);
}

void GuiPluginViewer::slotViewLayout(GuiViewports::ViewLayout layout)
{
    window_->suspendThreads();

    if (layout == GuiViewports::VIEW_LAYOUT_SINGLE)
    {
        window_->editor().viewportsResize(1);
        guiViewports_->setLayout(layout);
    }
    else if (layout == GuiViewports::VIEW_LAYOUT_TWO_COLUMNS)
    {
        window_->editor().viewportsResize(2);
        guiViewports_->setLayout(layout);
        guiViewports_->resetScene(&window_->editor(), 1, true);
    }
    else if ((layout == GuiViewports::VIEW_LAYOUT_GRID) ||
             (layout == GuiViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT))
    {
        window_->editor().viewportsResize(4);
        guiViewports_->setLayout(layout);
        guiViewports_->resetScene(&window_->editor(), 1, true);
        guiViewports_->resetScene(&window_->editor(), 2, true);
        guiViewports_->resetScene(&window_->editor(), 3, true);
    }

    updateViewer();
}

void GuiPluginViewer::updateViewer()
{
    window_->slotRenderViewport();
}
