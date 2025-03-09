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

/** @file SettingsApplication.hpp */

#ifndef SETTINGS_APPLICATION_HPP
#define SETTINGS_APPLICATION_HPP

// Include std.

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Settings Application. */
class EXPORT_EDITOR SettingsApplication
{
public:
    SettingsApplication();

    size_t cacheSizeMaximum() const;

private:
    size_t cacheSizeMaximum_;

    friend void fromJson(SettingsApplication &out, const Json &in);
    friend void toJson(Json &out, const SettingsApplication &in);

    friend std::string toString(const SettingsApplication &in);
};

void fromJson(SettingsApplication &out, const Json &in);
void toJson(Json &out, const SettingsApplication &in);

std::string toString(const SettingsApplication &in);

#include <WarningsEnable.hpp>

#endif /* SETTINGS_APPLICATION_HPP */
