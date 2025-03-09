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

/** @file SettingsImport.cpp */

// Include 3D Forest.
#include <SettingsImport.hpp>

// Include local.
#define LOG_MODULE_NAME "SettingsImport"
#include <Log.hpp>

SettingsImport::SettingsImport()
    : importFilesAsSeparateTrees(false),
      translateToOrigin(false),
      convertToVersion1Dot4(false),
      randomizePoints(true),
      copyExtraBytes(true),
      terminalOutput(false),
      maxIndexLevel1(0),
      maxIndexLevel1Size(10 * 1000),
      maxIndexLevel2(5),
      maxIndexLevel2Size(32),
      bufferSize(5 * 1024 * 1024)
{
}
