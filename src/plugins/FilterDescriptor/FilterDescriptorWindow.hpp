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

/** @file FilterDescriptorWindow.hpp */

#ifndef FILTER_DESCRIPTOR_WINDOW_HPP
#define FILTER_DESCRIPTOR_WINDOW_HPP

// Include 3D Forest.
#include <DockWidget.hpp>
class Application;
class FilterDescriptorWidget;

/** Filter Descriptor Window. */
class FilterDescriptorWindow : public DockWidget
{
public:
    FilterDescriptorWindow(Application *app);

protected:
    FilterDescriptorWidget *widget_;

    void showEvent(ShowEvent *event) override;
    void hideEvent(HideEvent *event) override;
    void closeEvent(CloseEvent *event) override;
};

#endif /* FILTER_DESCRIPTOR_WINDOW_HPP */
