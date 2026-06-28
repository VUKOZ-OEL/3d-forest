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

/** @file MessageBox.hpp */

#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

// Include std.
#include <string>

// Include 3D Forest.

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Message Box. */
class EXPORT_UI_COMMON MessageBox
{
public:
    enum StandardButton
    {
        NoButton = 0x00000000,
        Ok = 0x00000400,
        Save = 0x00000800,
        SaveAll = 0x00001000,
        Open = 0x00002000,
        Yes = 0x00004000,
        YesToAll = 0x00008000,
        No = 0x00010000,
        NoToAll = 0x00020000,
        Abort = 0x00040000,
        Retry = 0x00080000,
        Ignore = 0x00100000,
        Close = 0x00200000,
        Cancel = 0x00400000,
        Discard = 0x00800000,
        Help = 0x01000000,
        Apply = 0x02000000,
        Reset = 0x04000000,
        RestoreDefaults = 0x08000000
    };

    MessageBox();

    void setText(const std::string &text);
    void setInformativeText(const std::string &text);
    void setStandardButtons(int buttons);
    void setDefaultButton(int button);

    int exec();

private:
    int defaultButton_{0};
};

#include <WarningsEnable.hpp>

#endif /* MESSAGE_BOX_HPP */
