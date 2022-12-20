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

/** @file ProjectNavigatorItemIntensity.hpp */

#ifndef PROJECT_NAVIGATOR_ITEM_INTENSITY_HPP
#define PROJECT_NAVIGATOR_ITEM_INTENSITY_HPP

#include <Editor.hpp>
#include <ProjectNavigatorItem.hpp>

class MainWindow;

/** Project Navigator Intensity. */
class ProjectNavigatorItemIntensity : public ProjectNavigatorItem
{
    Q_OBJECT

public:
    ProjectNavigatorItemIntensity(MainWindow *mainWindow,
                                  const QIcon &icon,
                                  const QString &text);

    virtual bool hasColorSource() const { return true; }
    virtual SettingsView::ColorSource colorSource() const
    {
        return SettingsView::COLOR_SOURCE_INTENSITY;
    }

    virtual bool hasFilter() const { return false; }

protected:
};

#endif /* PROJECT_NAVIGATOR_ITEM_INTENSITY_HPP */
