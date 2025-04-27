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

/** @file ViewSettings.cpp */

// Include 3D Forest.
#include <ViewSettings.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewSettings"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ViewSettings::ViewSettings()
    : pointSize_(1.0),
      pointColor_(1.0, 1.0, 1.0),
      backgroundColor_(0.2, 0.2, 0.2),
      colorSource_(ViewSettings::ColorSource::COLOR),
      distanceBasedFadingVisible_(false),
      sceneBoundingBoxVisible_(true)
{
    colorSourceString_ = {"Color",
                          "Intensity",
                          "Return Number",
                          "Number of Returns",
                          "Classification",
                          "Tree",
                          "Species",
                          "Elevation",
                          "Descriptor"};
}

double ViewSettings::pointSize() const
{
    return pointSize_;
}

void ViewSettings::setPointSize(double size)
{
    pointSize_ = size;
}

void ViewSettings::setPointColor(const Vector3<double> &rgb)
{
    pointColor_ = rgb;
}

void ViewSettings::setBackgroundColor(const Vector3<double> &rgb)
{
    backgroundColor_ = rgb;
}

ViewSettings::ColorSource ViewSettings::colorSource() const
{
    return colorSource_;
}

void ViewSettings::setColorSource(ViewSettings::ColorSource colorSource)
{
    colorSource_ = colorSource;
}

size_t ViewSettings::colorSourceSize() const
{
    return colorSourceString_.size();
}

const char *ViewSettings::colorSourceString(size_t idx) const
{
    if (idx < colorSourceString_.size())
    {
        return colorSourceString_[idx].c_str();
    }

    return "Unknown";
}

bool ViewSettings::distanceBasedFadingVisible() const
{
    return distanceBasedFadingVisible_;
}

void ViewSettings::setDistanceBasedFadingVisible(bool b)
{
    distanceBasedFadingVisible_ = b;
}

bool ViewSettings::sceneBoundingBoxVisible() const
{
    return sceneBoundingBoxVisible_;
}

void ViewSettings::setSceneBoundingBoxVisible(bool b)
{
    sceneBoundingBoxVisible_ = b;
}

void fromJson(ViewSettings &out, const Json &in)
{
    fromJson(out.pointSize_, in["pointSize"]);
    fromJson(out.pointColor_, in["pointColor"]);
    fromJson(out.backgroundColor_, in["backgroundColor"]);
    fromJson(out.colorSource_, in["colorSource"]);

    fromJson(out.distanceBasedFadingVisible_, in["distanceBasedFadingVisible"]);
    fromJson(out.sceneBoundingBoxVisible_, in["sceneBoundingBoxVisible"]);
}

void toJson(Json &out, const ViewSettings &in)
{
    toJson(out["pointSize"], in.pointSize_);
    toJson(out["pointColor"], in.pointColor_);
    toJson(out["backgroundColor"], in.backgroundColor_);
    toJson(out["colorSource"], in.colorSource_);

    toJson(out["distanceBasedFadingVisible"], in.distanceBasedFadingVisible_);
    toJson(out["sceneBoundingBoxVisible"], in.sceneBoundingBoxVisible_);
}

std::string toString(const ViewSettings &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

void fromJson(ViewSettings::ColorSource &out, const Json &in)
{
    std::string colorSourceString;
    fromJson(colorSourceString, in);
    fromString(out, colorSourceString);
}

void toJson(Json &out, const ViewSettings::ColorSource &in)
{
    toJson(out, toString(in));
}

void fromString(ViewSettings::ColorSource &out, const std::string &in)
{
    if (in == "Color")
    {
        out = ViewSettings::ColorSource::COLOR;
    }
    else if (in == "Intensity")
    {
        out = ViewSettings::ColorSource::INTENSITY;
    }
    else if (in == "Return Number")
    {
        out = ViewSettings::ColorSource::RETURN_NUMBER;
    }
    else if (in == "Number of Returns")
    {
        out = ViewSettings::ColorSource::NUMBER_OF_RETURNS;
    }
    else if (in == "Classification")
    {
        out = ViewSettings::ColorSource::CLASSIFICATION;
    }
    else if (in == "Tree")
    {
        out = ViewSettings::ColorSource::SEGMENT;
    }
    else if (in == "Species")
    {
        out = ViewSettings::ColorSource::SPECIES;
    }
    else if (in == "Elevation")
    {
        out = ViewSettings::ColorSource::ELEVATION;
    }
    else if (in == "Descriptor")
    {
        out = ViewSettings::ColorSource::DESCRIPTOR;
    }
    else
    {
        LOG_ERROR(<< "Fix unknown color source <" << in << "> to <Color>.");
        out = ViewSettings::ColorSource::COLOR;
    }
}

std::string toString(const ViewSettings::ColorSource &in)
{
    switch (in)
    {
        case ViewSettings::ColorSource::COLOR:
            return "Color";
        case ViewSettings::ColorSource::INTENSITY:
            return "Intensity";
        case ViewSettings::ColorSource::RETURN_NUMBER:
            return "Return Number";
        case ViewSettings::ColorSource::NUMBER_OF_RETURNS:
            return "Number of Returns";
        case ViewSettings::ColorSource::CLASSIFICATION:
            return "Classification";
        case ViewSettings::ColorSource::SEGMENT:
            return "Tree";
        case ViewSettings::ColorSource::SPECIES:
            return "Species";
        case ViewSettings::ColorSource::ELEVATION:
            return "Elevation";
        case ViewSettings::ColorSource::DESCRIPTOR:
            return "Descriptor";
        case ViewSettings::ColorSource::UNKNOWN:
        default:
            return "Unknown";
    }
}
