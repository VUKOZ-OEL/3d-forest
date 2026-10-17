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

/** @file QtDoubleRangeSlider.hpp */

#ifndef QT_DOUBLE_RANGE_SLIDER_HPP
#define QT_DOUBLE_RANGE_SLIDER_HPP

// Include 3D Forest.
#include <DoubleRangeSlider.hpp>

// Include 3rd party.
#include <ctkDoubleRangeSlider.h>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtDoubleRangeSlider. */
class EXPORT_UI_QT QtDoubleRangeSlider : public ctkDoubleRangeSlider
{
    Q_OBJECT

    Q_PROPERTY(QColor grooveColor READ grooveColor WRITE setGrooveColor)
    Q_PROPERTY(
        QColor highlightColor READ highlightColor WRITE setHighlightColor)
    Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)

public:
    explicit QtDoubleRangeSlider(DoubleRangeSlider *slider,
                                 QWidget *parent = nullptr);
    virtual ~QtDoubleRangeSlider();

    QColor grooveColor() const { return grooveColor_; }
    void setGrooveColor(const QColor &color);

    QColor highlightColor() const { return highlightColor_; }
    void setHighlightColor(const QColor &color);

    QColor handleColor() const { return handleColor_; }
    void setHandleColor(const QColor &color);

    QColor borderColor() const { return borderColor_; }
    void setBorderColor(const QColor &color);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    DoubleRangeSlider *slider_;

    QColor grooveColor_{"#484848"};
    QColor highlightColor_{"#d0d0d0"};
    QColor handleColor_{"#f0f0f0"};
    QColor borderColor_{"#909090"};

    void updateSettings();

    void setColor(QColor &color, const QColor &newColor);
};

#include <WarningsEnable.hpp>

#endif /* QT_DOUBLE_RANGE_SLIDER_HPP */
