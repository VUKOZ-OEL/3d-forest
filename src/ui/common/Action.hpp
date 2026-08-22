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

/** @file Action.hpp */

#ifndef ACTION_HPP
#define ACTION_HPP

// Include std.
#include <string>

// Include 3D Forest.
#include <Signal.hpp>
class Widget;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Action. */
class EXPORT_UI_COMMON Action
{
public:
    explicit Action(const std::string &text);
    ~Action();

    Action(const Action &) = delete;
    Action &operator=(const Action &) = delete;

    const std::string &text() const { return text_; }

    void setPanel(Widget *panel);
    Widget *panel() const { return panel_; }
    bool hasPanel() const { return panel_ != nullptr; }

    void trigger();

    Signal<> triggered;

private:
    std::string text_;
    Widget *panel_{nullptr};
};

#include <WarningsEnable.hpp>

#endif /* ACTION_HPP */
