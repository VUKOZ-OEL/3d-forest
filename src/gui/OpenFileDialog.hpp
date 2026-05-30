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

/** @file OpenFileDialog.hpp */

#ifndef OPEN_FILE_DIALOG_HPP
#define OPEN_FILE_DIALOG_HPP

// Include std.
#include <string>

// Include 3D Forest.
class MainWindow;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Open File Dialog. */
class EXPORT_GUI OpenFileDialog
{
public:
    static std::string dialog(MainWindow *mainWindow,
                              const std::string &filter = "(*.*)");
};

#include <WarningsEnable.hpp>

#endif /* OPEN_FILE_DIALOG_HPP */
