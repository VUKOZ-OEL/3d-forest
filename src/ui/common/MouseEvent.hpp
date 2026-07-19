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

/** @file MouseEvent.hpp */

#ifndef MOUSE_EVENT_HPP
#define MOUSE_EVENT_HPP

// Include 3D Forest.
#include <Event.hpp>
#include <Point.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** MouseEvent. */
class EXPORT_UI_COMMON MouseEvent : public Event
{
public:
    Point pos() const { return pos_; }

private:
    Point pos_;
};

#include <WarningsEnable.hpp>

#endif /* MOUSE_EVENT_HPP */
