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

/** @file ColorSwitch.cpp */

// Include std.

// Include 3D Forest.
#include <ColorSwitch.hpp>
#include <Application.hpp>
#include <ColorDialog.hpp>
#include <Painter.hpp>

// Include local.
#define LOG_MODULE_NAME "ColorSwitch"
#include <Log.hpp>

ColorSwitch::ColorSwitch()
    : defaultWidth_(32),
      defaultHeight_(32),
      foregroundColorDefault_(255, 255, 255),
      backgroundColorDefault_(51, 51, 51),
      foregroundColor_(255, 255, 255),
      backgroundColor_(51, 51, 51)
{
}

ColorSwitch::~ColorSwitch()
{
}

void ColorSwitch::setForegroundColor(const Color &color)
{
    foregroundColor_ = color;
}

void ColorSwitch::setBackgroundColor(const Color &color)
{
    backgroundColor_ = color;
}

Color ColorSwitch::foregroundColor() const
{
    return foregroundColor_;
}

Color ColorSwitch::backgroundColor() const
{
    return backgroundColor_;
}

Size ColorSwitch::sizeHint() const
{
    return Size(defaultWidth_, defaultHeight_);
}

Size ColorSwitch::minimumSizeHint() const
{
    return Size(defaultWidth_, defaultHeight_);
}

void ColorSwitch::paintEvent(PaintEvent *)
{
    Painter painter(this);

    painter.setPen(Color(0, 0, 0));

    // Foreground/Background color.
    int w = (defaultWidth_ / 2) + 2;
    int h = (defaultHeight_ / 2) + 2;
    int x1 = 1;
    int y1 = 1;
    int x2 = (defaultWidth_ / 2) - 3;
    int y2 = (defaultHeight_ / 2) - 3;

    painter.fillRect(x2, y2, w, h, backgroundColor_);
    painter.drawRect(x2, y2, w, h);

    painter.fillRect(x1, y1, w, h, foregroundColor_);
    painter.drawRect(x1, y1, w, h);

    // Default colors.
    w = 5;
    h = 5;
    x1 = 1;
    y1 = (defaultHeight_ / 2) + (w + 1);
    x2 = x1 + w - 1;
    y2 = y1 + h - 1;

    painter.fillRect(x2, y2, w, h, backgroundColorDefault_);
    painter.drawRect(x2, y2, w, h);

    painter.fillRect(x1, y1, w, h, foregroundColorDefault_);
    painter.drawRect(x1, y1, w, h);

    // Swap.
    x1 = defaultWidth_ - (w + 1);
    y1 = 1;

    painter.drawLine(x1, y1, x1 + 2, y1);
    painter.drawLine(x1, y1, x1, y1 + 2);
    painter.drawLine(x1, y1, x1 + w, y1 + h);
    painter.drawLine(x1 + w - 2, y1 + h, x1 + w, y1 + h);
    painter.drawLine(x1 + w, y1 + h - 2, x1 + w, y1 + h);
}

void ColorSwitch::mousePressEvent(MouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();

    int w = (defaultWidth_ / 2) + 2;
    int h = (defaultHeight_ / 2) + 2;
    int x2 = (defaultWidth_ / 2) - 3;
    int y2 = (defaultHeight_ / 2) - 3;

    if (x < w && y < h)
    {
        // Foreground.
        if (colorDialog(foregroundColor_))
        {
            colorChanged();
        }
    }
    else if (x >= x2 && y >= y2)
    {
        // Background.
        if (colorDialog(backgroundColor_))
        {
            colorChanged();
        }
    }
    else if (x < x2 && y >= y2)
    {
        // Default.
        foregroundColor_ = foregroundColorDefault_;
        backgroundColor_ = backgroundColorDefault_;
        repaint();
        colorChanged();
    }
    else
    {
        // Swap.
        Color tmp = foregroundColor_;
        foregroundColor_ = backgroundColor_;
        backgroundColor_ = tmp;
        repaint();
        colorChanged();
    }
}

bool ColorSwitch::colorDialog(Color &color)
{
    ColorDialog dialog(color, this);

    if (dialog.exec() == Dialog::Rejected)
    {
        return false;
    }

    color = dialog.selectedColor();

    return true;
}
