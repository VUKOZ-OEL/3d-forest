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

/** @file SettingsView.cpp */

// Include 3D Forest.
#include <SettingsView.hpp>

// Include local.
#define LOG_MODULE_NAME "SettingsView"
#include <Log.hpp>

SettingsView::SettingsView()
    : pointSize_(1.0),
      fogEnabled_(false),
      pointColor_(1.0, 1.0, 1.0),
      background_(0.2, 0.2, 0.2)
{
    colorSourceString_ = {"Color",
                          "Intensity",
                          "Return Number",
                          "Number of Returns",
                          "Classification",
                          "Segment",
                          "Elevation",
                          "Descriptor"};

    colorSourceEnabled_.resize(colorSourceString_.size());
    for (auto &&it : colorSourceEnabled_)
    {
        it = false;
    }
    colorSourceEnabled_[0] = true;
}

double SettingsView::pointSize() const
{
    return pointSize_;
}

void SettingsView::setPointSize(double size)
{
    pointSize_ = size;
}

bool SettingsView::isFogEnabled() const
{
    return fogEnabled_;
}

void SettingsView::setFogEnabled(bool b)
{
    fogEnabled_ = b;
}

void SettingsView::setPointColor(const Vector3<double> &rgb)
{
    pointColor_ = rgb;
}

void SettingsView::setBackgroundColor(const Vector3<double> &rgb)
{
    background_ = rgb;
}

size_t SettingsView::colorSourceSize() const
{
    return colorSourceString_.size();
}

const char *SettingsView::colorSourceString(SettingsView::ColorSource id) const
{
    return colorSourceString_[static_cast<size_t>(id)].c_str();
}

const char *SettingsView::colorSourceString(size_t id) const
{
    if (id < static_cast<size_t>(SettingsView::COLOR_SOURCE_LAST))
    {
        return colorSourceString_[id].c_str();
    }

    return "";
}

bool SettingsView::isColorSourceEnabled(SettingsView::ColorSource id) const
{
    return colorSourceEnabled_[static_cast<size_t>(id)];
}

bool SettingsView::isColorSourceEnabled(size_t id) const
{
    if (id < static_cast<size_t>(SettingsView::COLOR_SOURCE_LAST))
    {
        return colorSourceEnabled_[id];
    }

    return false;
}

void SettingsView::setColorSourceEnabled(SettingsView::ColorSource id, bool v)
{
    colorSourceEnabled_[static_cast<size_t>(id)] = v;
}

void SettingsView::setColorSourceEnabled(size_t id, bool v)
{
    if (id < static_cast<size_t>(SettingsView::COLOR_SOURCE_LAST))
    {
        colorSourceEnabled_[id] = v;
    }
}

void SettingsView::setColorSourceEnabledAll(bool v)
{
    for (auto &&it : colorSourceEnabled_)
    {
        it = v;
    }
}

void fromJson(SettingsView &out, const Json &in)
{
    fromJson(out.pointSize_, in["pointSize"]);
    fromJson(out.fogEnabled_, in["fogEnabled"]);
    fromJson(out.pointColor_, in["pointColor"]);
    fromJson(out.background_, in["background"]);
}

void toJson(Json &out, const SettingsView &in)
{
    toJson(out["pointSize"], in.pointSize_);
    toJson(out["fogEnabled"], in.fogEnabled_);
    toJson(out["pointColor"], in.pointColor_);
    toJson(out["background"], in.background_);
}

std::string toString(const SettingsView &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}
