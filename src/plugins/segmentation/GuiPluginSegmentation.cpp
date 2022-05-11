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

/** @file GuiPluginSegmentation.cpp */

#include <Segmentation.hpp>

#include <GuiPluginSegmentation.hpp>
#include <GuiWindowMain.hpp>

#define ICON(name) (QIcon(":/segmentation/" name GUI_ICON_THEME ".png"))

GuiPluginSegmentation::GuiPluginSegmentation()
    : window_(nullptr),
      dockWindow_(nullptr)
{
}

void GuiPluginSegmentation::initialize(GuiWindowMain *window)
{
    window_ = window;

    window_->createAction(nullptr,
                          "Tools",
                          "Tools",
                          tr("Segmentation"),
                          tr("Segmentation"),
                          ICON("segmentation"),
                          this,
                          SLOT(slotPlugin()));
}

void GuiPluginSegmentation::slotPlugin()
{
}
