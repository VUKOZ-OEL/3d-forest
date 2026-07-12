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

/** @file Widget.cpp */

// Include std.

// Include 3D Forest.
#include <Widget.hpp>
#include <Application.hpp>

// Include local.
#define LOG_MODULE_NAME "Widget"
#include <Log.hpp>

Widget::Widget()
{
}

Widget::Widget(Application *app)
{
}

Widget::~Widget()
{
}

void Widget::setText(const std::string &str)
{
    text_ = str;
}

void Widget::setToolTip(const std::string &str)
{
    toolTip_ = str;
}

void Widget::setFocusPolicy(int focusPolicy)
{
}

void Widget::setVisible(bool b)
{
}

void Widget::setEnabled(bool b)
{
}

void Widget::setDisabled(bool b)
{
}

void Widget::setLayout(Layout *layout)
{
}

bool Widget::blockSignals(bool b)
{
    bool wasBlocked = signalsBlocked();
    signalsBlocked_ = b;
    return wasBlocked;
}

bool Widget::signalsBlocked() const
{
    return signalsBlocked_;
}

void Widget::repaint()
{
}

Size Widget::sizeHint() const
{
    return Size();
}

Size Widget::minimumSizeHint() const
{
    return Size();
}

void Widget::paintEvent(PaintEvent *event)
{
}

void Widget::mousePressEvent(MouseEvent *event)
{
}

void Widget::hideEvent(HideEvent *event)
{
}

void Widget::showEvent(ShowEvent *event)
{
}

void Widget::closeEvent(CloseEvent *event)
{
}
