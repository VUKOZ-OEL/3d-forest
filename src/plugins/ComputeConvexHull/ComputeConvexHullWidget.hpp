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

/** @file ComputeConvexHullWidget.hpp */

#ifndef COMPUTE_CONVEX_HULL_WIDGET_HPP
#define COMPUTE_CONVEX_HULL_WIDGET_HPP

// Include 3D Forest.
#include <ComputeConvexHullAction.hpp>
#include <ComputeConvexHullParameters.hpp>
class MainWindow;
class DoubleSliderWidget;

// Include Qt.
#include <QWidget>
class QPushButton;

/** Compute Convex Hull Widget. */
class ComputeConvexHullWidget : public QWidget
{
    Q_OBJECT

public:
    ComputeConvexHullWidget(MainWindow *mainWindow);

protected slots:
    void slotApply();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    MainWindow *mainWindow_;

    ComputeConvexHullParameters parameters_;
    ComputeConvexHullAction action_;

    DoubleSliderWidget *voxelRadiusSlider_;

    QPushButton *applyButton_;
};

#endif /* COMPUTE_CONVEX_HULL_WIDGET_HPP */
