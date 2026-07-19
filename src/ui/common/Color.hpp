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

// Include 3D Forest.
#include <Ui.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Color. */
class EXPORT_UI_COMMON Color
{
public:
    enum Spec
    {
        Invalid,
        Rgb,
        Hsv,
        Cmyk,
        Hsl,
        ExtendedRgb
    };
    enum NameFormat
    {
        HexRgb,
        HexArgb
    };

    Color();
    Color(int r, int g, int b, int a = 255);
    Color(Ui::GlobalColor gc);

    Spec spec() const { return spec_; }
    bool isValid() const { return spec_ != Invalid; }

    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);
    void setAlpha(int a);

    int red() const { return r_; }
    int green() const { return g_; }
    int blue() const { return b_; }
    int alpha() const { return a_; }

    void setRedF(float r);
    void setGreenF(float g);
    void setBlueF(float b);
    void setAlphaF(float a);

    float redF() const { return static_cast<float>(r_) / 255.0F; }
    float greenF() const { return static_cast<float>(g_) / 255.0F; }
    float blueF() const { return static_cast<float>(b_) / 255.0F; }
    float alphaF() const { return static_cast<float>(a_) / 255.0F; }

    void setRgb(int r, int g, int b, int a = 255);
    void setRgbF(float r, float g, float b, float a = 1.0F);

private:
    Spec spec_{Invalid};
    int r_{255};
    int g_{255};
    int b_{255};
    int a_{255};
};

#include <WarningsEnable.hpp>

#endif /* COLOR_HPP */
