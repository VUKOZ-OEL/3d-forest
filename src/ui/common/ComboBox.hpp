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

    int count() const { return static_cast<int>(items_.size()); }
    std::string itemText(int index) const;

    void setCurrentText(const std::string &str, bool notify = false);
    std::string currentText() const;
    int currentIndex() const;

    void setValue(int value, bool notify = false);

    Signal<int> currentIndexChanged;
    Signal<const std::string &> itemAdded;

    Signal<int> activated;

private:
    std::vector<std::string> items_;
    int value_{-1};
};

#include <WarningsEnable.hpp>

#endif /* COMBO_BOX_HPP */
