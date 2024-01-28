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

/** @file ColorSwitchWidget.hpp */

#ifndef COLOR_SWITCH_WIDGET_HPP
#define COLOR_SWITCH_WIDGET_HPP

// Include 3D Forest.
#include <Vector3.hpp>

// Include Qt.
#include <QWidget>

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Color Switch Widget. */
class EXPORT_GUI ColorSwitchWidget : public QWidget
{
    Q_OBJECT

public:
    ColorSwitchWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setForegroundColor(const QColor &color);
    void setBackgroundColor(const QColor &color);

    QColor foregroundColor() const;
    QColor backgroundColor() const;

signals:
    void colorChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int defaultWidth_;
    int defaultHeight_;
    QColor foregroundColorDefault_;
    QColor backgroundColorDefault_;
    QColor foregroundColor_;
    QColor backgroundColor_;

    bool colorDialog(QColor &color);
};

#include <WarningsEnable.hpp>

#endif /* COLOR_SWITCH_WIDGET_HPP */
