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
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

SettingsView::SettingsView()
    : pointSize_(1.0),
      pointColor_(1.0, 1.0, 1.0),
      backgroundColor_(0.2, 0.2, 0.2),
      colorSource_(SettingsView::ColorSource::COLOR),
      distanceBasedFadingVisible_(false),
      sceneBoundingBoxVisible_(true),
      treeAttributesVisible_(true)
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

void SettingsView::setPointColor(const Vector3<double> &rgb)
{
    pointColor_ = rgb;
}

void SettingsView::setBackgroundColor(const Vector3<double> &rgb)
{
    backgroundColor_ = rgb;
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

bool SettingsView::distanceBasedFadingVisible() const
{
    return distanceBasedFadingVisible_;
}

void SettingsView::setDistanceBasedFadingVisible(bool b)
{
    distanceBasedFadingVisible_ = b;
}

bool SettingsView::sceneBoundingBoxVisible() const
{
    return sceneBoundingBoxVisible_;
}

void SettingsView::setSceneBoundingBoxVisible(bool b)
{
    sceneBoundingBoxVisible_ = b;
}

bool SettingsView::treeAttributesVisible() const
{
    return treeAttributesVisible_;
}

void SettingsView::setTreeAttributesVisible(bool b)
{
    treeAttributesVisible_ = b;
}

void fromJson(SettingsView &out, const Json &in)
{
    fromJson(out.pointSize_, in["pointSize"]);
    fromJson(out.pointColor_, in["pointColor"]);
    fromJson(out.backgroundColor_, in["backgroundColor"]);
    fromJson(out.colorSource_, in["colorSource"]);

    fromJson(out.distanceBasedFadingVisible_, in["distanceBasedFadingVisible"]);
    fromJson(out.sceneBoundingBoxVisible_, in["sceneBoundingBoxVisible"]);
    fromJson(out.treeAttributesVisible_, in["treeAttributesVisible"]);
}

void toJson(Json &out, const SettingsView &in)
{
    toJson(out["pointSize"], in.pointSize_);
    toJson(out["pointColor"], in.pointColor_);
    toJson(out["backgroundColor"], in.backgroundColor_);
    toJson(out["colorSource"], in.colorSource_);

    toJson(out["distanceBasedFadingVisible"], in.distanceBasedFadingVisible_);
    toJson(out["sceneBoundingBoxVisible"], in.sceneBoundingBoxVisible_);
    toJson(out["treeAttributesVisible"], in.treeAttributesVisible_);
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
        LOG_ERROR(<< "Fix unknown color source <" << in << "> to <Color>.");
        out = SettingsView::ColorSource::COLOR;
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
