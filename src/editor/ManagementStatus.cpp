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

/** @file ManagementStatus.cpp */

// Include 3D Forest.
#include <Error.hpp>
#include <ManagementStatus.hpp>
#include <Core.hpp>

// Include local.
#define LOG_MODULE_NAME "ManagementStatus"
#include <Log.hpp>

ManagementStatus::ManagementStatus()
{
}

ManagementStatus::ManagementStatus(size_t id,
                                   const std::string &label,
                                   const Vector3<double> &color)
    : id(id),
      label(label),
      color(color)
{
}

void fromJson(ManagementStatus &out, const Json &in)
{
    fromJson(out.id, in["id"]);
    fromJson(out.color, in["color"]);

    if (in.contains("label"))
    {
        fromJson(out.label, in["label"]);
    }

    if (in.contains("en"))
    {
        fromJson(out.label, in["en"]);
    }

    if (in.contains("cs"))
    {
        std::string str;
        fromJson(str, in["cs"]);
        core().insertTranslation(out.label, str, "cs");
    }
}

void toJson(Json &out, const ManagementStatus &in)
{
    toJson(out["id"], in.id);
    toJson(out["en"], in.label);
    toJson(out["color"], in.color);

    std::string cs = core().translate(in.label, "cs");
    if (cs != in.label)
    {
         toJson(out["cs"], cs);
    }
}

std::ostream &operator<<(std::ostream &out, const ManagementStatus &in)
{
    Json json;
    toJson(json, in);
    return out << json.serialize();
}
