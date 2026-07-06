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

/** @file Color.hpp */

#ifndef COLOR_HPP
#define COLOR_HPP

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Color. */
class EXPORT_UI_COMMON Color
{
public:
    Color();
    Color(int r, int g, int b, int a = 255);

    int red() const { return r_; }
    int green() const { return g_; }
    int blue() const { return b_; }
    int alpha() const { return a_; }

    float redF() const { return static_cast<float>(r_) / 255.0; }
    float greenF() const { return static_cast<float>(g_) / 255.0; }
    float blueF() const { return static_cast<float>(b_) / 255.0; }
    float alphaF() const { return static_cast<float>(a_) / 255.0; }

    void setRgb(int r, int g, int b, int a = 255);
    void setRgbF(float r, float g, float b, float a = 1.0F);

private:
    int r_{255};
    int g_{255};
    int b_{255};
    int a_{255};
};

#include <WarningsEnable.hpp>

#endif /* COLOR_HPP */
