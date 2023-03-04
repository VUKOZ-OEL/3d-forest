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

/** @file ProjectNavigatorItemClippingCylinder.hpp */

#ifndef PROJECT_NAVIGATOR_ITEM_CLIPPING_CYLINDER_HPP
#define PROJECT_NAVIGATOR_ITEM_CLIPPING_CYLINDER_HPP

#include <Region.hpp>
class DoubleSliderWidget;
class MainWindow;

#include <QWidget>

/** Project Navigator Clipping Cylinder. */
class ProjectNavigatorItemClippingCylinder : public QWidget
{
    Q_OBJECT

public:
    ProjectNavigatorItemClippingCylinder(MainWindow *mainWindow);

    void setRegion(const Region &region);

signals:
    void signalRegionChanged(const Region &region);

protected slots:
    void slotInputChanged();

protected:
    DoubleSliderWidget *pointAInput_[3];
    DoubleSliderWidget *pointBInput_[3];
    DoubleSliderWidget *radiusInput_;
};

#endif /* PROJECT_NAVIGATOR_ITEM_CLIPPING_CYLINDER_HPP */
