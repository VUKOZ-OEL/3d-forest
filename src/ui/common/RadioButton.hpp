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
class RadioButtonGroup;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** RadioButton. */
class EXPORT_UI_COMMON RadioButton : public Widget
{
public:
    explicit RadioButton(const std::string &text = "");
    ~RadioButton() override;

    void setText(const std::string &text);
    const std::string &text() const { return text_; }

    void setChecked(bool checked, bool notify = false);
    bool isChecked() const { return checked_; }

    void setGroup(RadioButtonGroup *group);

    Signal<const std::string &> textChanged;
    Signal<bool> checkedUpdated;
    Signal<int> stateChanged;

private:
    friend class RadioButtonGroup;

    std::string text_;
    bool checked_{false};
    RadioButtonGroup *group_{nullptr};
};

#include <WarningsEnable.hpp>

#endif /* RADIO_BUTTON_HPP */
