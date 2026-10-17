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

/** @file Painter.hpp */

#ifndef PAINTER_HPP
#define PAINTER_HPP

// Include 3D Forest.
#include <Color.hpp>
class Widget;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Painter. */
class EXPORT_UI_COMMON Painter
{
public:
    Painter();
    Painter(Widget *widget);
    Painter(int w, int h);

    int width() const { return w_; }
    int height() const { return h_; }

    void setWidth(int w);
    void setHeight(int h);

    void setPen(const Color &color);

    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int w, int h);
    void fillRect(int x, int y, int w, int h, const Color &color);

private:
    int w_{0};
    int h_{0};
};

#include <WarningsEnable.hpp>

#endif /* PAINTER_HPP */
