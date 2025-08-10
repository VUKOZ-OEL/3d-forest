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

/** @file UnitsSettings.cpp */

// Include 3D Forest.
#include <UnitsSettings.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "UnitsSettings"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void UnitsSettings::setLasFileScaling(const Vector3<double> &scaling)
{
    pointsPerMeterLas = safeDivide(1.0, scaling);

    LOG_DEBUG(<< "Dataset scaling <" << scaling << "> sets las points/m <"
              << pointsPerMeterLas << ">.");
}

const Vector3<double> &UnitsSettings::pointsPerMeter() const
{
    if (userDefined)
    {
        return pointsPerMeterUser;
    }

    return pointsPerMeterLas;
}

bool UnitsSettings::apply(const UnitsSettings &newSettings)
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

void fromJson(UnitsSettings &out, const Json &in)
{
    fromJson(out.userDefined, in, "userDefined", false);
    fromJson(out.pointsPerMeterUser,
             in,
             "pointsPerMeterUser",
             {1000.0, 1000.0, 1000.0});
    fromJson(out.pointsPerMeterLas,
             in,
             "pointsPerMeter",
             {1000.0, 1000.0, 1000.0});
}

void toJson(Json &out, const UnitsSettings &in)
{
    toJson(out["userDefined"], in.userDefined);
    toJson(out["pointsPerMeterUser"], in.pointsPerMeterUser);
    toJson(out["pointsPerMeter"], in.pointsPerMeter());
}

std::string toString(const UnitsSettings &in)
{
    return "{\"pointsPerMeterLas\": " + toString(in.pointsPerMeterLas) +
           ",\"pointsPerMeterUser\": " + toString(in.pointsPerMeterUser) +
           ",\"userDefined\": " + toString(in.userDefined) + "}";
}
