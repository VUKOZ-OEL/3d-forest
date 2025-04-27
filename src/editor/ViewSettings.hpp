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

/** @file ViewSettings.hpp */

#ifndef VIEW_SETTINGS_HPP
#define VIEW_SETTINGS_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** View Settings. */
class EXPORT_EDITOR ViewSettings
{
public:
    enum class ColorSource
    {
        COLOR,
        INTENSITY,
        RETURN_NUMBER,
        NUMBER_OF_RETURNS,
        CLASSIFICATION,
        SEGMENT,
        SPECIES,
        MANAGEMENT_STATUS,
        ELEVATION,
        DESCRIPTOR,
        UNKNOWN
    };

    ViewSettings();

    double pointSize() const;
    void setPointSize(double size);

    const Vector3<double> &pointColor() const { return pointColor_; }
    void setPointColor(const Vector3<double> &rgb);

    const Vector3<double> &backgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Vector3<double> &rgb);

    ViewSettings::ColorSource colorSource() const;
    void setColorSource(ViewSettings::ColorSource colorSource);

    size_t colorSourceSize() const;
    const char *colorSourceString(size_t idx) const;

    bool distanceBasedFadingVisible() const;
    void setDistanceBasedFadingVisible(bool b);

    bool sceneBoundingBoxVisible() const;
    void setSceneBoundingBoxVisible(bool b);

protected:
    double pointSize_;
    Vector3<double> pointColor_;
    Vector3<double> backgroundColor_;
    ViewSettings::ColorSource colorSource_;

    bool distanceBasedFadingVisible_;
    bool sceneBoundingBoxVisible_;

    std::vector<std::string> colorSourceString_;

    friend void fromJson(ViewSettings &out, const Json &in);
    friend void toJson(Json &out, const ViewSettings &in);
};

void fromJson(ViewSettings &out, const Json &in);
void toJson(Json &out, const ViewSettings &in);

std::string toString(const ViewSettings &in);

void fromJson(ViewSettings::ColorSource &out, const Json &in);
void toJson(Json &out, const ViewSettings::ColorSource &in);

void EXPORT_EDITOR fromString(ViewSettings::ColorSource &out,
                              const std::string &in);
std::string EXPORT_EDITOR toString(const ViewSettings::ColorSource &in);

#include <WarningsEnable.hpp>

#endif /* VIEW_SETTINGS_HPP */
