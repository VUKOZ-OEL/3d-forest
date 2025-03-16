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

void Settings::setRenderingSettings(const RenderingSettings &renderingSettings)
{
    renderingSettings_ = renderingSettings;
}

void Settings::setTreeSettings(const TreeSettings &treeSettings)
{
    treeSettings_ = treeSettings;
}

bool Settings::setUnitsSettings(const UnitsSettings &unitsSettings)
{
    return unitsSettings_.apply(unitsSettings);
}

void Settings::setViewSettings(const ViewSettings &viewSettings)
{
    viewSettings_ = viewSettings;
}

void fromJson(Settings &out, const Json &in)
{
    if (in.contains("renderingSettings"))
    {
        fromJson(out.renderingSettings_, in["renderingSettings"]);
    }

    if (in.contains("treeSettings"))
    {
        fromJson(out.treeSettings_, in["treeSettings"]);
    }

    if (in.contains("unitsSettings"))
    {
        fromJson(out.unitsSettings_, in["unitsSettings"]);
    }

    if (in.contains("viewSettings"))
    {
        fromJson(out.viewSettings_, in["viewSettings"]);
    }
}

void toJson(Json &out, const Settings &in)
{
    toJson(out["renderingSettings"], in.renderingSettings_);
    toJson(out["treeSettings"], in.treeSettings_);
    toJson(out["unitsSettings"], in.unitsSettings_);
    toJson(out["viewSettings"], in.viewSettings_);
}

std::string toString(const Settings &in)
{
    return "{\"renderingSettings\": " + toString(in.renderingSettings_) +
           ",\"treeSettings\": " + toString(in.treeSettings_) +
           ",\"unitsSettings\": " + toString(in.unitsSettings_) +
           ",\"viewSettings\": " + toString(in.viewSettings_) + "}";
}
