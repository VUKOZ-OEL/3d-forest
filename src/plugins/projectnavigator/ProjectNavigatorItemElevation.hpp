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

/** @file ProjectNavigatorItemElevation.hpp */

#ifndef PROJECT_NAVIGATOR_ITEM_ELEVATION_HPP
#define PROJECT_NAVIGATOR_ITEM_ELEVATION_HPP

#include <Editor.hpp>
#include <ProjectNavigatorItem.hpp>
#include <Range.hpp>

class MainWindow;
class RangeSliderWidget;

/** Project Navigator Elevation. */
class ProjectNavigatorItemElevation : public ProjectNavigatorItem
{
    Q_OBJECT

public:
    ProjectNavigatorItemElevation(MainWindow *mainWindow);

    virtual bool hasColorSource() const { return true; }
    virtual SettingsView::ColorSource colorSource() const
    {
        return SettingsView::COLOR_SOURCE_ELEVATION;
    }

    virtual bool hasFilter() const { return false; }

public slots:
    void slotUpdate(const QSet<Editor::Type> &target);

    void slotRangeIntermediateMinimumValue();
    void slotRangeIntermediateMaximumValue();

protected:
    MainWindow *mainWindow_;

    RangeSliderWidget *rangeInput_;
    Range<double> elevationRange_;

    void elevationInputChanged();
};

#endif /* PROJECT_NAVIGATOR_ITEM_ELEVATION_HPP */
