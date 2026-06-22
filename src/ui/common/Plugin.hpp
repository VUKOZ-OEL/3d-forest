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

/** @file Plugin.hpp */

#ifndef PLUGIN_HPP
#define PLUGIN_HPP

// Include 3D Forest.
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Plugin. */
class EXPORT_UI_COMMON Plugin
{
public:
    virtual const char *name() const = 0;
    virtual void initialize(Application *app) = 0;
    virtual void release() = 0;

protected:
    virtual ~Plugin() = default;
};

#include <WarningsEnable.hpp>

#endif /* PLUGIN_HPP */
