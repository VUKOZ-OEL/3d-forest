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

static const char *SETTINGS_KEY_APPLICATION = "applicationSettings";
static const char *SETTINGS_KEY_RENDERING = "renderingSettings";
static const char *SETTINGS_KEY_TREE = "treeSettings";
static const char *SETTINGS_KEY_UNITS = "unitsSettings";
static const char *SETTINGS_KEY_VIEW = "viewSettings";
static const char *SETTINGS_KEY_SPECIES = "defaultSpecies";
static const char *SETTINGS_KEY_MANAGEMENT_STATUS = "defaultManagementStatus";

Settings::Settings()
{
    defaultSpeciesList_.setDefault();
    defaultManagementStatusList_.setDefault();
}

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
    if (in.contains(SETTINGS_KEY_APPLICATION))
    {
        fromJson(out.applicationSettings_, in[SETTINGS_KEY_APPLICATION]);
    }

    if (in.contains(SETTINGS_KEY_RENDERING))
    {
        fromJson(out.renderingSettings_, in[SETTINGS_KEY_RENDERING]);
    }

    if (in.contains(SETTINGS_KEY_TREE))
    {
        fromJson(out.treeSettings_, in[SETTINGS_KEY_TREE]);
    }

    if (in.contains(SETTINGS_KEY_UNITS))
    {
        fromJson(out.unitsSettings_, in[SETTINGS_KEY_UNITS]);
    }

    if (in.contains(SETTINGS_KEY_VIEW))
    {
        fromJson(out.viewSettings_, in[SETTINGS_KEY_VIEW]);
    }

    if (in.contains(SETTINGS_KEY_SPECIES))
    {
        fromJson(out.defaultSpeciesList_, in[SETTINGS_KEY_SPECIES]);
    }

    if (in.contains(SETTINGS_KEY_MANAGEMENT_STATUS))
    {
        fromJson(out.defaultManagementStatusList_,
                 in[SETTINGS_KEY_MANAGEMENT_STATUS]);
    }
}

void toJson(Json &out, const Settings &in)
{
    toJson(out[SETTINGS_KEY_APPLICATION], in.applicationSettings_);
    toJson(out[SETTINGS_KEY_RENDERING], in.renderingSettings_);
    toJson(out[SETTINGS_KEY_TREE], in.treeSettings_);
    toJson(out[SETTINGS_KEY_UNITS], in.unitsSettings_);
    toJson(out[SETTINGS_KEY_VIEW], in.viewSettings_);
    toJson(out[SETTINGS_KEY_SPECIES], in.defaultSpeciesList_);
    toJson(out[SETTINGS_KEY_MANAGEMENT_STATUS],
           in.defaultManagementStatusList_);
}

void toJsonProjectSettings(Json &out, const Settings &in)
{
    toJson(out[SETTINGS_KEY_RENDERING], in.renderingSettings_);
    toJson(out[SETTINGS_KEY_TREE], in.treeSettings_);
    toJson(out[SETTINGS_KEY_UNITS], in.unitsSettings_);
    toJson(out[SETTINGS_KEY_VIEW], in.viewSettings_);
}

std::string toString(const Settings &in)
{
    Json json;
    toJson(json, in);
    return json.serialize();
}
