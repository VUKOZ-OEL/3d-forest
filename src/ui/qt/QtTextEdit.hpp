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

/** @file QtTextEdit.hpp */

#ifndef QT_TEXT_EDIT_HPP
#define QT_TEXT_EDIT_HPP

// Include 3D Forest.
#include <TextEdit.hpp>

// Include Qt.
#include <QTextEdit>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtTextEdit. */
class EXPORT_UI_QT QtTextEdit : public QTextEdit
{
public:
    explicit QtTextEdit(TextEdit *textEdit, QWidget *parent = nullptr);
    ~QtTextEdit() override;

private:
    TextEdit *textEdit_;
    bool updatingCommon_{false};
};

#include <WarningsEnable.hpp>

#endif /* QT_TEXT_EDIT_HPP */
