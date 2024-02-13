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

/** @file SettingsImport.hpp */

#ifndef SETTINGS_IMPORT_HPP
#define SETTINGS_IMPORT_HPP

// Include std.
#include <cstddef>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Settings Import. */
class EXPORT_EDITOR SettingsImport
{
public:
    bool convertToVersion1Dot4;
    bool centerPointsOnScreen;
    bool randomizePoints;
    bool copyExtraBytes;
    bool terminalOutput;

    size_t maxIndexLevel1;
    size_t maxIndexLevel1Size;
    size_t maxIndexLevel2;
    size_t maxIndexLevel2Size;

    size_t bufferSize;

    SettingsImport();
};

#include <WarningsEnable.hpp>

#endif /* SETTINGS_IMPORT_HPP */
