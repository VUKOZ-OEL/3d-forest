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

/** @file Settings.cpp */

// Include 3D Forest.
#include <Settings.hpp>

// Include local.
#define LOG_MODULE_NAME "Settings"
#include <Log.hpp>

void Settings::view(const SettingsView &view)
{
    view_ = view;
}

bool Settings::units(const SettingsUnits &units)
{
    return units_.apply(units);
}

void fromJson(Settings &out, const Json &in)
{
    if (in.contains("view"))
    {
        fromJson(out.view_, in["view"]);
    }

    if (in.contains("units"))
    {
        fromJson(out.units_, in["units"]);
    }
}

void toJson(Json &out, const Settings &in)
{
    toJson(out["view"], in.view_);
    toJson(out["units"], in.units_);
}

std::string toString(const Settings &in)
{
    return "{\"view\": " + toString(in.view_) +
           ",\"units\": " + toString(in.units_) + "}";
}
