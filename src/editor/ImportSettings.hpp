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

/** @file ImportSettings.hpp */

#ifndef IMPORT_SETTINGS_HPP
#define IMPORT_SETTINGS_HPP

// Include std.
#include <cstddef>
#include <vector>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Import Settings. */
class EXPORT_EDITOR ImportSettings
{
public:
    bool importFilesAsSeparateTrees;
    bool translateToOrigin;

    bool convertToVersion1Dot4;
    bool randomizePoints;
    bool copyExtraBytes;

    bool terminalOutput;

    size_t maxIndexLevel1;
    std::vector<size_t> maxIndexLevel1Size;
    size_t maxIndexLevel2;
    std::vector<size_t> maxIndexLevel2Size;

    size_t bufferSize;

    ImportSettings();
};

#include <WarningsEnable.hpp>

#endif /* IMPORT_SETTINGS_HPP */
