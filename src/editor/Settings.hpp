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

/** @file Settings.hpp */

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

// Include 3D Forest.
#include <SettingsImport.hpp>
#include <SettingsUnits.hpp>
#include <SettingsView.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Settings. */
class EXPORT_EDITOR Settings
{
public:
    SettingsView view;
    SettingsUnits units;
};

void fromJson(Settings &out, const Json &in);
void toJson(Json &out, const Settings &in);

#include <WarningsEnable.hpp>

#endif /* SETTINGS_HPP */
