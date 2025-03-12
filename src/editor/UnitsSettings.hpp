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

/** @file UnitsSettings.hpp */

#ifndef UNITS_SETTINGS_HPP
#define UNITS_SETTINGS_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Units Settings. */
class EXPORT_EDITOR UnitsSettings
{
public:
    Vector3<double> pointsPerMeterLas{1000.0, 1000.0, 1000.0};
    Vector3<double> pointsPerMeterUser{1000.0, 1000.0, 1000.0};
    bool userDefined{false};

    void setLasFileScaling(const Vector3<double> &scaling);
    const Vector3<double> &pointsPerMeter() const;
    bool apply(const UnitsSettings &newSettings);
};

void fromJson(UnitsSettings &out, const Json &in);
void toJson(Json &out, const UnitsSettings &in);

std::string EXPORT_EDITOR toString(const UnitsSettings &in);

#include <WarningsEnable.hpp>

#endif /* UNITS_SETTINGS_HPP */
