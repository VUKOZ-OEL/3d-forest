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

/** @file InfoDialog.hpp */

#ifndef INFO_DIALOG_HPP
#define INFO_DIALOG_HPP

// Include 3D Forest.
#include <Dialog.hpp>
class TextEdit;
class PushButton;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Info Dialog. */
class EXPORT_UI_COMMON InfoDialog : public Dialog
{
public:
    InfoDialog(Application *app, int w = 0, int h = 0);

    Size sizeHint() const override;
    Size minimumSizeHint() const override;

    void setText(const std::string &text);

    void slotClose();

private:
    int defaultWidth_;
    int defaultHeight_;
    TextEdit *textEdit_;
    PushButton *closeButton_;
};

#include <WarningsEnable.hpp>

#endif /* INFO_DIALOG_HPP */
