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
      showSceneBoundingBoxEnabled_(true),
      showAttributesEnabled_(true),
      pointColor_(1.0, 1.0, 1.0),
      background_(0.2, 0.2, 0.2),
      colorSource_(SettingsView::ColorSource::COLOR)
{
    colorSourceString_ = {"Color",
                          "Intensity",
                          "Return Number",
                          "Number of Returns",
                          "Classification",
                          "Tree",
                          "Elevation",
                          "Descriptor"};
}

double SettingsView::pointSize() const
{
    return pointSize_;
}

void SettingsView::setPointSize(double size)
{
    pointSize_ = size;
}

bool SettingsView::fogEnabled() const
{
    return fogEnabled_;
}

void SettingsView::setFogEnabled(bool b)
{
    fogEnabled_ = b;
}

bool SettingsView::showSceneBoundingBoxEnabled() const
{
    return showSceneBoundingBoxEnabled_;
}

void SettingsView::setShowSceneBoundingBoxEnabled(bool b)
{
    showSceneBoundingBoxEnabled_ = b;
}

bool SettingsView::showAttributesEnabled() const
{
    return showAttributesEnabled_;
}

void SettingsView::setShowAttributesEnabled(bool b)
{
    showAttributesEnabled_ = b;
}

void SettingsView::setPointColor(const Vector3<double> &rgb)
{
    pointColor_ = rgb;
}

void SettingsView::setBackgroundColor(const Vector3<double> &rgb)
{
    background_ = rgb;
}

SettingsView::ColorSource SettingsView::colorSource() const
{
    return colorSource_;
}

void SettingsView::setColorSource(SettingsView::ColorSource colorSource)
{
    colorSource_ = colorSource;
}

size_t SettingsView::colorSourceSize() const
{
    return colorSourceString_.size();
}

const char *SettingsView::colorSourceString(size_t idx) const
{
    if (idx < colorSourceString_.size())
    {
        return colorSourceString_[idx].c_str();
    }

    return "Unknown";
}

void fromJson(SettingsView &out, const Json &in)
{
    fromJson(out.pointSize_, in["pointSize"]);
    fromJson(out.fogEnabled_, in["fogEnabled"]);
    fromJson(out.showSceneBoundingBoxEnabled_,
             in["showSceneBoundingBoxEnabled"]);
    fromJson(out.showAttributesEnabled_, in["showAttributesEnabled"]);
    fromJson(out.pointColor_, in["pointColor"]);
    fromJson(out.background_, in["background"]);
    fromJson(out.colorSource_, in["colorSource"]);
}

void toJson(Json &out, const SettingsView &in)
{
    toJson(out["pointSize"], in.pointSize_);
    toJson(out["fogEnabled"], in.fogEnabled_);
    toJson(out["showSceneBoundingBoxEnabled"], in.showSceneBoundingBoxEnabled_);
    toJson(out["showAttributesEnabled"], in.showAttributesEnabled_);
    toJson(out["pointColor"], in.pointColor_);
    toJson(out["background"], in.background_);
    toJson(out["colorSource"], in.colorSource_);
}

std::string toString(const SettingsView &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

void fromJson(SettingsView::ColorSource &out, const Json &in)
{
    std::string colorSourceString;
    fromJson(colorSourceString, in);
    fromString(out, colorSourceString);
}

void toJson(Json &out, const SettingsView::ColorSource &in)
{
    toJson(out, toString(in));
}

void fromString(SettingsView::ColorSource &out, const std::string &in)
{
    if (in == "Color")
    {
        out = SettingsView::ColorSource::COLOR;
    }
    else if (in == "Intensity")
    {
        out = SettingsView::ColorSource::INTENSITY;
    }
    else if (in == "Return Number")
    {
        out = SettingsView::ColorSource::RETURN_NUMBER;
    }
    else if (in == "Number of Returns")
    {
        out = SettingsView::ColorSource::NUMBER_OF_RETURNS;
    }
    else if (in == "Classification")
    {
        out = SettingsView::ColorSource::CLASSIFICATION;
    }
    else if (in == "Tree")
    {
        out = SettingsView::ColorSource::SEGMENT;
    }
    else if (in == "Elevation")
    {
        out = SettingsView::ColorSource::ELEVATION;
    }
    else if (in == "Descriptor")
    {
        out = SettingsView::ColorSource::DESCRIPTOR;
    }
    else
    {
        out = SettingsView::ColorSource::UNKNOWN;
    }
}

std::string toString(const SettingsView::ColorSource &in)
{
    switch (in)
    {
        case SettingsView::ColorSource::COLOR:
            return "Color";
        case SettingsView::ColorSource::INTENSITY:
            return "Intensity";
        case SettingsView::ColorSource::RETURN_NUMBER:
            return "Return Number";
        case SettingsView::ColorSource::NUMBER_OF_RETURNS:
            return "Number of Returns";
        case SettingsView::ColorSource::CLASSIFICATION:
            return "Classification";
        case SettingsView::ColorSource::SEGMENT:
            return "Tree";
        case SettingsView::ColorSource::ELEVATION:
            return "Elevation";
        case SettingsView::ColorSource::DESCRIPTOR:
            return "Descriptor";
        case SettingsView::ColorSource::UNKNOWN:
        default:
            return "Unknown";
    }
}
