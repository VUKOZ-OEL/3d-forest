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

/** @file ComboBox.hpp */

#ifndef COMBO_BOX_HPP
#define COMBO_BOX_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** ComboBox. */
class EXPORT_UI_COMMON ComboBox : public Widget
{
public:
    ComboBox();
    virtual ~ComboBox();

    void addItem(const std::string &str);
    void setCurrentText(const std::string &str, bool notify = false);
    std::string itemText(int index) const;

    void setValue(int value, bool notify = false);

    Signal<int> activated;

private:
    int value_{0};
};

#include <WarningsEnable.hpp>

#endif /* COMBO_BOX_HPP */
