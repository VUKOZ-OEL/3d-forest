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

/** @file PluginHandle.hpp */

#ifndef PLUGIN_HANDLE_HPP
#define PLUGIN_HANDLE_HPP

// Include std.
#if defined(_MSC_VER)
    #include <windows.h>
#endif

// Include 3D Forest.
#include <Plugin.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Plugin Handle. */
class EXPORT_UI_COMMON PluginHandle
{
public:
    Plugin *plugin = nullptr;

#if defined(_MSC_VER)
    HMODULE handle = nullptr;
#else
    void *handle = nullptr;
#endif
};

#include <WarningsEnable.hpp>

#endif /* PLUGIN_HANDLE_HPP */
