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

/** @file FilterDescriptorWidget.hpp */

#ifndef FILTER_DESCRIPTOR_WIDGET_HPP
#define FILTER_DESCRIPTOR_WIDGET_HPP

// Include 3D Forest.
#include <Editor.hpp>
#include <Range.hpp>
#include <Region.hpp>
#include <Widget.hpp>
class DoubleRangeSliderWidget;
class Application;

/** Filter Descriptor Widget. */
class FilterDescriptorWidget : public Widget
{
public:
    FilterDescriptorWidget(Application *app);

    void setFilterEnabled(bool b);

    Size sizeHint() const override { return Size(300, 120); }

    void slotUpdate(void *sender, const std::set<Editor::Type> &target);

    void slotMinimumValueChanged(double val);
    void slotMaximumValueChanged(double val);
    void slotFinalValue();

protected:
    Application *app_;
    DoubleRangeSliderWidget *descriptorInput_;
    Range<double> descriptorRange_;

    void setDescriptor(const Range<double> &descriptorRange);
    void filterChanged(bool final);
};

#endif /* FILTER_DESCRIPTOR_WIDGET_HPP */
