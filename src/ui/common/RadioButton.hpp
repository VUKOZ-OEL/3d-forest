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

/** @file RadioButton.hpp */

#ifndef RADIO_BUTTON_HPP
#define RADIO_BUTTON_HPP

// Include 3D Forest.
#include <Widget.hpp>
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** RadioButton. */
class EXPORT_UI_COMMON RadioButton : public Widget
{
public:
    RadioButton(const std::string &str = "");
    virtual ~RadioButton();

    void setChecked(bool b, bool notify = false);
    bool isChecked() const;

    Signal<int> stateChanged;

private:
    bool checked_{false};
};

#include <WarningsEnable.hpp>

#endif /* RADIO_BUTTON_HPP */
