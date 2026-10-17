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

#ifndef RADIO_BUTTON_GROUP_HPP
#define RADIO_BUTTON_GROUP_HPP

// Include std.
#include <vector>

// Include 3D Forest.
class RadioButton;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** RadioButton. */
class EXPORT_UI_COMMON RadioButtonGroup
{
public:
    RadioButtonGroup() = default;
    ~RadioButtonGroup();

    RadioButtonGroup(const RadioButtonGroup &) = delete;
    RadioButtonGroup &operator=(const RadioButtonGroup &) = delete;

    void addButton(RadioButton *button);
    void removeButton(RadioButton *button);

    RadioButton *checkedButton() const { return checkedButton_; }

private:
    friend class RadioButton;

    std::vector<RadioButton *> buttons_;
    RadioButton *checkedButton_{nullptr};
};

#include <WarningsEnable.hpp>

#endif /* RADIO_BUTTON_GROUP_HPP */
