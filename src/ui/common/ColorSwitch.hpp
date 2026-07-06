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

/** @file ColorSwitch.hpp */

#ifndef COLOR_SWITCH_HPP
#define COLOR_SWITCH_HPP

// Include 3D Forest.
#include <Widget.hpp>
#include <Color.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** ColorSwitch. */
class EXPORT_UI_COMMON ColorSwitch : public Widget
{
public:
    ColorSwitch();
    virtual ~ColorSwitch();

    void setForegroundColor(const Color &color);
    void setBackgroundColor(const Color &color);

    Color foregroundColor() const;
    Color backgroundColor() const;

    Signal<> colorChanged;

    Size sizeHint() const override;
    Size minimumSizeHint() const override;

    void paintEvent(PaintEvent *event) override;
    void mousePressEvent(MouseEvent *event) override;

private:
    int defaultWidth_;
    int defaultHeight_;
    Color foregroundColorDefault_;
    Color backgroundColorDefault_;
    Color foregroundColor_;
    Color backgroundColor_;

    bool colorDialog(Color &color);
};

#include <WarningsEnable.hpp>

#endif /* COLOR_SWITCH_HPP */
