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

/** @file ProjectFileInterface.hpp */

#ifndef PROJECT_FILE_INTERFACE_HPP
#define PROJECT_FILE_INTERFACE_HPP

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Project File Interface. */
class EXPORT_GUI ProjectFileInterface
{
public:
    virtual ~ProjectFileInterface() = default;
    virtual bool closeProject() = 0;
};

#include <WarningsEnable.hpp>

#endif /* PROJECT_FILE_INTERFACE_HPP */
