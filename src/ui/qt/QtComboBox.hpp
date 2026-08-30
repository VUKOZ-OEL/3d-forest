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

/** @file QtComboBox.hpp */

#ifndef QT_COMBO_BOX_HPP
#define QT_COMBO_BOX_HPP

// Include 3D Forest.
#include <ComboBox.hpp>

// Include Qt.
#include <QComboBox>
class QWidget;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtComboBox. */
class EXPORT_UI_QT QtComboBox : public QComboBox
{
public:
    explicit QtComboBox(ComboBox *comboBox, QWidget *parent = nullptr);

private:
    ComboBox *comboBox_;
};

#include <WarningsEnable.hpp>

#endif /* QT_COMBO_BOX_HPP */
