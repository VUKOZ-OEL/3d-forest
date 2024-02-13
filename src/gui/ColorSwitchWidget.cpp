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

/** @file ColorSwitchWidget.cpp */

// Include 3D Forest.
#include <ColorSwitchWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

// Include local.
#define LOG_MODULE_NAME "ColorSwitchWidget"
#include <Log.hpp>

ColorSwitchWidget::ColorSwitchWidget(QWidget *parent)
    : QWidget(parent),
      defaultWidth_(32),
      defaultHeight_(32),
      foregroundColorDefault_(255, 255, 255),
      backgroundColorDefault_(51, 51, 51),
      foregroundColor_(255, 255, 255),
      backgroundColor_(51, 51, 51)
{
}

QSize ColorSwitchWidget::sizeHint() const
{
    return QSize(defaultWidth_, defaultHeight_);
}

QSize ColorSwitchWidget::minimumSizeHint() const
{
    return QSize(defaultWidth_, defaultHeight_);
}

void ColorSwitchWidget::setForegroundColor(const QColor &color)
{
    foregroundColor_ = color;
}

void ColorSwitchWidget::setBackgroundColor(const QColor &color)
{
    backgroundColor_ = color;
}

QColor ColorSwitchWidget::foregroundColor() const
{
    return foregroundColor_;
}

QColor ColorSwitchWidget::backgroundColor() const
{
    return backgroundColor_;
}

void ColorSwitchWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setPen(QColor(0, 0, 0));

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

void ColorSwitchWidget::mousePressEvent(QMouseEvent *event)
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
            emit colorChanged();
        }
    }
    else if (x >= x2 && y >= y2)
    {
        // Background.
        if (colorDialog(backgroundColor_))
        {
            emit colorChanged();
        }
    }
    else if (x < x2 && y >= y2)
    {
        // Default.
        foregroundColor_ = foregroundColorDefault_;
        backgroundColor_ = backgroundColorDefault_;
        repaint();
        emit colorChanged();
    }
    else
    {
        // Swap.
        QColor tmp = foregroundColor_;
        foregroundColor_ = backgroundColor_;
        backgroundColor_ = tmp;
        repaint();
        emit colorChanged();
    }
}

bool ColorSwitchWidget::colorDialog(QColor &color)
{
    QColorDialog dialog(color, this);

    if (dialog.exec() == QDialog::Rejected)
    {
        return false;
    }

    color = dialog.selectedColor();

    return true;
}
