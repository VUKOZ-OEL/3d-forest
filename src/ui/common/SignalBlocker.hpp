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

/** @file SignalBlocker.hpp */

#ifndef SIGNAL_BLOCKER_HPP
#define SIGNAL_BLOCKER_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** SignalBlocker. */
class EXPORT_UI_COMMON SignalBlocker
{
public:
    explicit SignalBlocker(Widget *object)
        : object_(object),
          previousState_(object->blockSignals(true))
    {
    }

    ~SignalBlocker() { object_->blockSignals(previousState_); }

    SignalBlocker(const SignalBlocker &) = delete;
    SignalBlocker &operator=(const SignalBlocker &) = delete;

private:
    Widget *object_;
    bool previousState_;
};

#include <WarningsEnable.hpp>

#endif /* SIGNAL_BLOCKER_HPP */
