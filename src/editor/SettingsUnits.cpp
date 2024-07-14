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

/** @file SettingsUnits.cpp */

// Include 3D Forest.
#include <SettingsUnits.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "SettingsUnits"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

SettingsUnits::SettingsUnits()
    : pointsPerMeterLas(1000.0, 1000.0, 1000.0),
      pointsPerMeterUser(1000.0, 1000.0, 1000.0),
      userDefined(false)
{
}

void SettingsUnits::setLasFileScaling(const Vector3<double> &scaling)
{
    pointsPerMeterLas = safeDivide(1.0, scaling);

    LOG_DEBUG(<< "Dataset scaling <" << scaling << "> sets las points/m <"
              << pointsPerMeterLas << ">.");
}

const Vector3<double> &SettingsUnits::pointsPerMeter() const
{
    if (userDefined)
    {
        return pointsPerMeterUser;
    }

    return pointsPerMeterLas;
}

bool SettingsUnits::apply(const SettingsUnits &newSettings)
{
    bool unsavedChanges{false};

    pointsPerMeterLas = newSettings.pointsPerMeterLas;

    if (pointsPerMeterUser != newSettings.pointsPerMeterUser)
    {
        unsavedChanges = true;
    }
    pointsPerMeterUser = newSettings.pointsPerMeterUser;

    if (userDefined != newSettings.userDefined)
    {
        unsavedChanges = true;
    }
    userDefined = newSettings.userDefined;

    return unsavedChanges;
}

void fromJson(SettingsUnits &out, const Json &in)
{
    fromJson(out.userDefined, in["userDefined"]);
    fromJson(out.pointsPerMeterUser, in["pointsPerMeterUser"]);
}

void toJson(Json &out, const SettingsUnits &in)
{
    toJson(out["userDefined"], in.userDefined);
    toJson(out["pointsPerMeterUser"], in.pointsPerMeterUser);
}
