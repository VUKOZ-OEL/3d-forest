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

/** @file QtLineEdit.hpp */

#ifndef QT_LINE_EDIT_HPP
#define QT_LINE_EDIT_HPP

// Include 3D Forest.
#include <LineEdit.hpp>

// Include Qt.
#include <QLineEdit>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtLineEdit. */
class EXPORT_UI_QT QtLineEdit : public QLineEdit
{
public:
    explicit QtLineEdit(LineEdit *lineEdit, QWidget *parent = nullptr);
    ~QtLineEdit() override;

private:
    LineEdit *lineEdit_;
};

#include <WarningsEnable.hpp>

#endif /* QT_LINE_EDIT_HPP */
