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

/** @file FileDialog.hpp */

#ifndef FILE_DIALOG_HPP
#define FILE_DIALOG_HPP

// Include std.
#include <string>

// Include 3D Forest.
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** File Dialog. */
class EXPORT_UI_COMMON FileDialog
{
public:
    enum Option
    {
        ShowDirsOnly = 0x00000001,
        DontResolveSymlinks = 0x00000002,
        DontConfirmOverwrite = 0x00000004,
        DontUseNativeDialog = 0x00000008,
        ReadOnly = 0x00000010,
        HideNameFilterDetails = 0x00000020,
        DontUseCustomDirectoryIcons = 0x00000040
    };

    static std::vector<std::string> selectFiles(Application *app,
                                                const std::string &dialogTitle,
                                                const std::string &filter);

    static std::string getOpenFileName(Application *app,
                                       const std::string &dialogTitle,
                                       const std::string &filter);

    static std::string getSaveFileName(Application *app,
                                       const std::string &dialogTitle,
                                       const std::string &filter);

    static std::string getSaveFileName(Application *app,
                                       const std::string &caption,
                                       const std::string &dir,
                                       const std::string &filter,
                                       std::string *selectedFilter = nullptr,
                                       int options = 0);

private:
};

#include <WarningsEnable.hpp>

#endif /* FILE_DIALOG_HPP */
