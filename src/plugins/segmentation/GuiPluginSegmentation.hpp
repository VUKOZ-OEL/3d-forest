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

/** @file GuiPluginSegmentation.hpp */

#ifndef GUI_PLUGIN_SEGMENTATION_HPP
#define GUI_PLUGIN_SEGMENTATION_HPP

#include <GuiPluginInterface.hpp>

class GuiPluginSegmentationWindow;

/** Gui Plugin Segmentation. */
class GuiPluginSegmentation : public QObject, public GuiPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GuiPluginInterface_iid)
    Q_INTERFACES(GuiPluginInterface)

public:
    GuiPluginSegmentation();

    virtual void initialize(GuiWindowMain *window);

public slots:
    void slotPlugin();

protected:
    GuiWindowMain *window_;
    GuiPluginSegmentationWindow *dockWindow_;
    QAction *action_;
};

#endif /* GUI_PLUGIN_SEGMENTATION_HPP */
