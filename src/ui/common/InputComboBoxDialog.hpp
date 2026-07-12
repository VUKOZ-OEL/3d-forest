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

/** @file EditPluginSetClassification.hpp */

#ifndef INPUT_COMBO_BOX_DIALOG_HPP
#define INPUT_COMBO_BOX_DIALOG_HPP

// Include 3D Forest.
#include <Dialog.hpp>
class ComboBox;
class PushButton;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Input Combo Box Dialog. */
class EXPORT_UI_COMMON InputComboBoxDialog : public Dialog
{
public:
    explicit InputComboBoxDialog(Application *app);

    void setOkButtonText(const std::string &text);

    void addItem(const std::string &text);

    /// Returns current combo box item index.
    int currentIndex() const;

    /// Returns current combo box item text.
    std::string currentText() const;

private:
    ComboBox *comboBox_;
    PushButton *okButton_;
    PushButton *cancelButton_;
};

#include <WarningsEnable.hpp>

#endif /* INPUT_COMBO_BOX_DIALOG_HPP */
