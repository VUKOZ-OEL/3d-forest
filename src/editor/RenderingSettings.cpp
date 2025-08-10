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

/** @file RenderingSettings.cpp */

// Include 3D Forest.
#include <RenderingSettings.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "RenderingSettings"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

RenderingSettings::RenderingSettings() : cacheSizeMaximum_(1024)
{
}

size_t RenderingSettings::cacheSizeMaximum() const
{
    return cacheSizeMaximum_;
}

void fromJson(RenderingSettings &out, const Json &in)
{
    if (in.contains("cacheSizeMaximum"))
    {
        fromJson(out.cacheSizeMaximum_, in["cacheSizeMaximum"]);
    }
    else
    {
        out.cacheSizeMaximum_ = 1024;
    }
}

void toJson(Json &out, const RenderingSettings &in)
{
    toJson(out["cacheSizeMaximum"], in.cacheSizeMaximum_);
}

std::string toString(const RenderingSettings &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}
