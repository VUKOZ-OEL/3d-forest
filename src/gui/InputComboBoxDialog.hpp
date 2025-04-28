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

// Include Qt.
#include <QDialog>
class QComboBox;
class QPushButton;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Input Combo Box Dialog. */
class EXPORT_GUI InputComboBoxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputComboBoxDialog(QWidget *parent = nullptr);

    void setOkButtonText(const QString &text);

    void addItem(const QString &text);

    /// Returns current combo box item index.
    int currentIndex() const;

    /// Returns current combo box item text.
    QString currentText() const;

private:
    QComboBox *comboBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#include <WarningsEnable.hpp>

#endif /* INPUT_COMBO_BOX_DIALOG_HPP */
