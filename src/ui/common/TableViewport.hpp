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

/** @file TableWidget.hpp */

#ifndef TABLE_VIEWPORT_HPP
#define TABLE_VIEWPORT_HPP

// Include std.
#include <functional>

// Include 3D Forest.
#include <Point.hpp>
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TableWidget. */
class EXPORT_UI_COMMON TableViewport : public Widget
{
public:
public:
    Point mapToGlobal(const Point &point) const;

private:
    friend class TableWidget;

    std::function<Point(const Point &)> mapToGlobal_;
};

#include <WarningsEnable.hpp>

#endif /* TABLE_VIEWPORT_HPP */
