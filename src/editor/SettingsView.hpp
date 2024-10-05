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

/** @file SettingsView.hpp */

#ifndef SETTINGS_VIEW_HPP
#define SETTINGS_VIEW_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Settings View. */
class EXPORT_EDITOR SettingsView
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
        ELEVATION,
        DESCRIPTOR,
        UNKNOWN
    };

    SettingsView();

    double pointSize() const;
    void setPointSize(double size);

    bool fogEnabled() const;
    void setFogEnabled(bool b);

    bool showSceneBoundingBoxEnabled() const;
    void setShowSceneBoundingBoxEnabled(bool b);

    bool showAttributesEnabled() const;
    void setShowAttributesEnabled(bool b);

    const Vector3<double> &pointColor() const { return pointColor_; }
    void setPointColor(const Vector3<double> &rgb);

    const Vector3<double> &backgroundColor() const { return background_; }
    void setBackgroundColor(const Vector3<double> &rgb);

    SettingsView::ColorSource colorSource() const;
    void setColorSource(SettingsView::ColorSource colorSource);

    size_t colorSourceSize() const;
    const char *colorSourceString(size_t idx) const;

protected:
    double pointSize_;
    bool fogEnabled_;
    bool showSceneBoundingBoxEnabled_;
    bool showAttributesEnabled_;
    Vector3<double> pointColor_;
    Vector3<double> background_;
    SettingsView::ColorSource colorSource_;

    std::vector<std::string> colorSourceString_;

    friend void fromJson(SettingsView &out, const Json &in);
    friend void toJson(Json &out, const SettingsView &in);
};

void fromJson(SettingsView &out, const Json &in);
void toJson(Json &out, const SettingsView &in);

std::string toString(const SettingsView &in);

void fromJson(SettingsView::ColorSource &out, const Json &in);
void toJson(Json &out, const SettingsView::ColorSource &in);

void fromString(SettingsView::ColorSource &out, const std::string &in);
std::string toString(const SettingsView::ColorSource &in);

#include <WarningsEnable.hpp>

#endif /* SETTINGS_VIEW_HPP */
