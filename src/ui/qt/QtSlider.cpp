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

/** @file QtSlider.cpp */

// Include std.
#include <algorithm>
#include <cmath>

// Include 3D Forest.
#include <QtSlider.hpp>

// Include Qt.
#include <QPainter>
#include <QPointer>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyleOptionSlider>

// Include local.
#define LOG_MODULE_NAME "QtSlider"
#include <Log.hpp>

QtSlider::QtSlider(Slider *slider, QWidget *parent)
    : QSlider(Qt::Horizontal, parent),
      slider_(slider)
{
    setProperty("singleValueSlider", true);

    updateSettings();

    const QPointer<QtSlider> guard(this);

    connect(this,
            &QSlider::valueChanged,
            this,
            [this](int value) { slider_->setValue(value, true); });

    connect(this,
            &QSlider::sliderReleased,
            this,
            [this]
            {
                if (!slider_->signalsBlocked())
                    slider_->sliderReleased();
            });

    slider_->valueUpdated.connect(
        [guard](int value)
        {
            if (guard)
            {
                const QSignalBlocker blocker(guard.data());
                guard->setValue(value);
            }
        });

    slider_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });
}

QtSlider::~QtSlider()
{
}

void QtSlider::updateSettings()
{
    const QSignalBlocker blocker(this);

    setOrientation(static_cast<Qt::Orientation>(slider_->orientation()));
    if (orientation() == Qt::Horizontal)
        setMinimumHeight(22);
    setRange(slider_->minimum(), slider_->maximum());
    setSingleStep(slider_->singleStep());
    setTickInterval(slider_->tickInterval());
    setTickPosition(
        static_cast<QSlider::TickPosition>(slider_->tickPosition()));

    setValue(slider_->value());
}

void QtSlider::paintEvent(QPaintEvent *event)
{
    const QColor accent = property("sidebarSliderAccent").value<QColor>();
    if (orientation() != Qt::Horizontal || !accent.isValid())
    {
        QSlider::paintEvent(event);
        return;
    }
    QStyleOptionSlider option;
    initStyleOption(&option);
    const auto center = [&](int position)
    {
        option.sliderPosition = position;
        option.sliderValue = position;
        return style()
            ->subControlRect(QStyle::CC_Slider,
                             &option,
                             QStyle::SC_SliderHandle,
                             this)
            .center();
    };
    const QPoint start = center(minimum());
    const QPoint end = center(maximum());
    const QPoint handle = center(sliderPosition());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled())
        painter.setOpacity(.45);
    painter.setPen(Qt::NoPen);
    painter.setBrush(property("sidebarSliderGroove").value<QColor>());
    painter.drawRoundedRect(QRectF(std::min(start.x(), end.x()),
                                   handle.y() - 3,
                                   std::abs(end.x() - start.x()),
                                   6),
                            3,
                            3);
    painter.setBrush(accent);
    painter.drawRoundedRect(QRectF(std::min(start.x(), handle.x()),
                                   handle.y() - 3,
                                   std::abs(handle.x() - start.x()),
                                   6),
                            3,
                            3);
    if (tickPosition() != QSlider::NoTicks)
    {
        const int interval =
            std::max(1, tickInterval() > 0 ? tickInterval() : singleStep());
        const bool above = tickPosition() == QSlider::TicksAbove ||
                           tickPosition() == QSlider::TicksBothSides;
        const bool below = tickPosition() == QSlider::TicksBelow ||
                           tickPosition() == QSlider::TicksBothSides;
        painter.setPen(property("sidebarSliderGroove").value<QColor>());
        for (qint64 value = minimum(); value <= maximum(); value += interval)
        {
            const int x = center(int(value)).x();
            if (above)
                painter.drawLine(x, 1, x, handle.y() - 5);
            if (below)
                painter.drawLine(x, handle.y() + 5, x, height() - 2);
            if (maximum() - value < interval)
                break;
        }
    }
    painter.setPen(QPen(property("sidebarSliderBorder").value<QColor>(), 2));
    painter.setBrush(property("sidebarSliderHandle").value<QColor>());
    // Equal dimensions keep the handle circular regardless of the row height.
    painter.drawEllipse(QPointF(handle), 8.0, 8.0);
}
