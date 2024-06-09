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
    enum ColorSource
    {
        COLOR_SOURCE_COLOR,
        COLOR_SOURCE_INTENSITY,
        COLOR_SOURCE_RETURN_NUMBER,
        COLOR_SOURCE_NUMBER_OF_RETURNS,
        COLOR_SOURCE_CLASSIFICATION,
        COLOR_SOURCE_SEGMENT,
        COLOR_SOURCE_ELEVATION,
        COLOR_SOURCE_DESCRIPTOR,
        COLOR_SOURCE_LAST
    };

    SettingsView();

    double pointSize() const;
    void setPointSize(double size);

    bool isFogEnabled() const;
    void setFogEnabled(bool b);

    void setPointColor(const Vector3<double> &rgb);
    const Vector3<double> &pointColor() const { return pointColor_; }

    void setBackgroundColor(const Vector3<double> &rgb);
    const Vector3<double> &backgroundColor() const { return background_; }

    size_t colorSourceSize() const;
    const char *colorSourceString(SettingsView::ColorSource id) const;
    const char *colorSourceString(size_t id) const;
    bool isColorSourceEnabled(SettingsView::ColorSource id) const;
    bool isColorSourceEnabled(size_t id) const;
    void setColorSourceEnabled(SettingsView::ColorSource id, bool v);
    void setColorSourceEnabled(size_t id, bool v);
    void setColorSourceEnabledAll(bool v);

protected:
    double pointSize_;
    bool fogEnabled_;
    Vector3<double> pointColor_;
    Vector3<double> background_;
    std::vector<std::string> colorSourceString_;
    std::vector<bool> colorSourceEnabled_;

    friend void fromJson(SettingsView &out, const Json &in);
    friend void toJson(Json &out, const SettingsView &in);
};

void fromJson(SettingsView &out, const Json &in);
void toJson(Json &out, const SettingsView &in);

#include <WarningsEnable.hpp>

#endif /* SETTINGS_VIEW_HPP */
