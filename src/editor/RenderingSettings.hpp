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

/** @file RenderingSettings.hpp */

#ifndef RENDERING_SETTINGS_HPP
#define RENDERING_SETTINGS_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Rendering Settings. */
class EXPORT_EDITOR RenderingSettings
{
public:
    RenderingSettings();

    size_t cacheSizeMaximum() const;

private:
    size_t cacheSizeMaximum_;

    friend void fromJson(RenderingSettings &out, const Json &in);
    friend void toJson(Json &out, const RenderingSettings &in);

    friend std::string toString(const RenderingSettings &in);
};

void fromJson(RenderingSettings &out, const Json &in);
void toJson(Json &out, const RenderingSettings &in);

std::string toString(const RenderingSettings &in);

#include <WarningsEnable.hpp>

#endif /* RENDERING_SETTINGS_HPP */
