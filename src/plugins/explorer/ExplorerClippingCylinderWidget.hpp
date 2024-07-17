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

/** @file ExplorerClippingCylinderWidget.hpp */

#ifndef EXPLORER_CLIPPING_CYLINDER_WIDGET_HPP
#define EXPLORER_CLIPPING_CYLINDER_WIDGET_HPP

// Include 3D Forest.
#include <Region.hpp>
class DoubleSliderWidget;
class MainWindow;

// Include Qt.
#include <QWidget>

/** Explorer Clipping Cylinder Widget. */
class ExplorerClippingCylinderWidget : public QWidget
{
    Q_OBJECT

public:
    ExplorerClippingCylinderWidget(MainWindow *mainWindow);

    void setRegion(const Region &region);

signals:
    void signalRegionChanged(const Region &region);

protected slots:
    void slotInputChanged();

protected:
    MainWindow *mainWindow_;
    DoubleSliderWidget *pointAInput_[3];
    DoubleSliderWidget *pointBInput_[3];
    DoubleSliderWidget *radiusInput_;
};

#endif /* EXPLORER_CLIPPING_CYLINDER_WIDGET_HPP */
