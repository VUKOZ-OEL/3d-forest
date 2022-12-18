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

#include <SettingsView.hpp>

SettingsView::SettingsView()
    : pointSize_(1.0F),
      fogEnabled_(false),
      pointColor_(1.0F, 1.0F, 1.0F),
      background_(0.2F, 0.2F, 0.2F)
{
    colorSourceString_ = {"Color",
                          "Intensity",
                          "Return Number",
                          "Number of Returns",
                          "Classification",
                          "Layer",
                          "Elevation",
                          "Custom Color",
                          "Descriptor",
                          "Density",
                          "Normal"};

    colorSourceEnabled_.resize(colorSourceString_.size());
    for (auto &&it : colorSourceEnabled_)
    {
        it = false;
    }
    colorSourceEnabled_[0] = true;
}

float SettingsView::pointSize() const
{
    return pointSize_;
}

void SettingsView::setPointSize(float size)
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

void SettingsView::setPointColor(const Vector3<float> &rgb)
{
    pointColor_ = rgb;
}

void SettingsView::setBackgroundColor(const Vector3<float> &rgb)
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

bool SettingsView::isColorSourceEnabled(SettingsView::ColorSource id) const
{
    return colorSourceEnabled_[static_cast<size_t>(id)];
}

void SettingsView::setColorSourceEnabled(SettingsView::ColorSource id, bool v)
{
    colorSourceEnabled_[static_cast<size_t>(id)] = v;
}

void SettingsView::setColorSourceEnabledAll(bool v)
{
    for (auto &&it : colorSourceEnabled_)
    {
        it = v;
    }
}

void SettingsView::read(const Json &in)
{
    if (in.contains("pointSize"))
    {
        pointSize_ = static_cast<float>(in["pointSize"].number());
    }

    if (in.contains("fog") && in["fog"].contains("enabled"))
    {
        fogEnabled_ = in["fog"]["enabled"].isTrue();
    }
    else
    {
        fogEnabled_ = false;
    }

    if (in.contains("pointColor"))
    {
        pointColor_.read(in["pointColor"]);
    }

    if (in.contains("background"))
    {
        background_.read(in["background"]);
    }
}

Json &SettingsView::write(Json &out) const
{
    out["pointSize"] = pointSize_;
    out["fog"]["enabled"] = fogEnabled_;
    pointColor_.write(out["pointColor"]);
    background_.write(out["background"]);

    return out;
}
