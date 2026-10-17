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

/** @file ComputeHullWidget.hpp */

#ifndef COMPUTE_HULL_WIDGET_HPP
#define COMPUTE_HULL_WIDGET_HPP

// Include 3D Forest.
#include <ComputeHullAction.hpp>
#include <ComputeHullParameters.hpp>
#include <Widget.hpp>
class Application;
class DoubleSliderWidget;
class CheckBox;
class PushButton;

/** Compute Hull Widget. */
class ComputeHullWidget : public Widget
{
public:
    ComputeHullWidget(Application *app);

    void slotApply();
    void slotFindOptimalAlphaChanged(int index);

    void hideEvent(HideEvent *event) override;

private:
    Application *app_;

    ComputeHullParameters parameters_;
    ComputeHullAction action_;

    CheckBox *computeConvexHullCheckBox_;
    CheckBox *computeConvexHullProjectionCheckBox_;
    CheckBox *computeConcaveHullCheckBox_;
    CheckBox *computeConcaveHullProjectionCheckBox_;
    CheckBox *findOptimalAlphaCheckBox_;
    DoubleSliderWidget *alphaSlider_;
    DoubleSliderWidget *voxelRadiusSlider_;
    PushButton *applyButton_;
};

#endif /* COMPUTE_HULL_WIDGET_HPP */
