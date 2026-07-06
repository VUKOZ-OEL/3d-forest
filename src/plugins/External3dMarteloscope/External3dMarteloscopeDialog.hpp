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

/** @file External3dMarteloscopeDialog.hpp */

#ifndef EXTERNAL_3D_MARTELOSCOPE_DIALOG_HPP
#define EXTERNAL_3D_MARTELOSCOPE_DIALOG_HPP

// Include Std.
#include <string>

// Include 3D Forest.
class Application;

// Include Qt.
#include <Dialog>
class QLineEdit;
class PushButton;

/** External 3d-Marteloscope Dialog. */
class External3dMarteloscopeDialog : public Dialog
{


public:
    External3dMarteloscopeDialog(Application *app);

    const std::string &path() const { return path_; }


    void slotBrowse();
    void slotAccept();
    void slotReject();

protected:
    Application *app_;

    std::string path_;

    QLineEdit *fileNameLineEdit_;
    PushButton *browseButton_;

    PushButton *acceptButton_;
    PushButton *rejectButton_;
};

#endif /* EXTERNAL_3D_MARTELOSCOPE_DIALOG_HPP */
