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

/** @file QtDoubleRangeSlider.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <QtDoubleRangeSlider.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>

// Include 3rd party.
#include <ctkRangeSlider.h>

// Include local.
#define LOG_MODULE_NAME "QtDoubleRangeSlider"
#include <Log.hpp>

QtDoubleRangeSlider::QtDoubleRangeSlider(DoubleRangeSlider *slider,
                                         QWidget *parent)
    : ctkDoubleRangeSlider(Qt::Horizontal, parent),
      slider_(slider)
{
    updateSettings();

    const QPointer<QtDoubleRangeSlider> guard(this);

    // Qt user changes -> common state and plugin callbacks.
    QObject::connect(this,
                     &ctkDoubleRangeSlider::valuesChanged,
                     this,
                     [this](double minimumValue, double maximumValue)
                     { slider_->setValues(minimumValue, maximumValue, true); });

    QObject::connect(this,
                     &ctkDoubleRangeSlider::sliderReleased,
                     this,
                     [this]
                     {
                         if (!slider_->signalsBlocked())
                         {
                             slider_->sliderReleased();
                         }
                     });

    // Common state -> Qt.
    slider_->valuesUpdated.connect(
        [guard](double minimumValue, double maximumValue)
        {
            if (!guard)
            {
                return;
            }

            if (guard->minimumValue() == minimumValue &&
                guard->maximumValue() == maximumValue)
            {
                return;
            }

            const QSignalBlocker blocker(guard.data());
            guard->setValues(minimumValue, maximumValue);
        });

    slider_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    // Paint
    if (auto *rangeSlider = findChild<ctkRangeSlider *>())
    {
        rangeSlider->installEventFilter(this);
    }
}

QtDoubleRangeSlider::~QtDoubleRangeSlider()
{
}

void QtDoubleRangeSlider::updateSettings()
{
    const QSignalBlocker blocker(this);

    setOrientation(slider_->orientation() == DoubleRangeSlider::Vertical
                       ? Qt::Vertical
                       : Qt::Horizontal);

    setSingleStep(slider_->singleStep());
    setRange(slider_->minimum(), slider_->maximum());
    setValues(slider_->minimumValue(), slider_->maximumValue());
}

void QtDoubleRangeSlider::setGrooveColor(const QColor &color)
{
    setColor(grooveColor_, color);
}

void QtDoubleRangeSlider::setHighlightColor(const QColor &color)
{
    setColor(highlightColor_, color);
}

void QtDoubleRangeSlider::setHandleColor(const QColor &color)
{
    setColor(handleColor_, color);
}

void QtDoubleRangeSlider::setBorderColor(const QColor &color)
{
    setColor(borderColor_, color);
}

void QtDoubleRangeSlider::setColor(QColor &color, const QColor &newColor)
{
    if (color == newColor)
    {
        return;
    }

    color = newColor;

    // Painting happens on CTK's internal slider.
    if (auto *slider = findChild<ctkRangeSlider *>())
    {
        slider->update();
    }
}

bool QtDoubleRangeSlider::eventFilter(QObject *object, QEvent *event)
{
    auto *rangeSlider = qobject_cast<ctkRangeSlider *>(object);

    if (!rangeSlider || event->type() != QEvent::Paint ||
        rangeSlider->orientation() != Qt::Horizontal)
    {
        return ctkDoubleRangeSlider::eventFilter(object, event);
    }

    QStyleOptionSlider option;
    option.initFrom(rangeSlider);

    option.orientation = Qt::Horizontal;
    option.state |= QStyle::State_Horizontal;
    option.minimum = rangeSlider->minimum();
    option.maximum = rangeSlider->maximum();
    option.singleStep = rangeSlider->singleStep();
    option.pageStep = rangeSlider->pageStep();
    option.tickPosition = rangeSlider->tickPosition();
    option.tickInterval = rangeSlider->tickInterval();

    option.upsideDown = rangeSlider->invertedAppearance() !=
                        (rangeSlider->layoutDirection() == Qt::RightToLeft);

    // Use native handle geometry to keep drawing aligned with hit testing.
    const auto handleCenter = [&](int position)
    {
        option.sliderPosition = position;
        option.sliderValue = position;

        return rangeSlider->style()
            ->subControlRect(QStyle::CC_Slider,
                             &option,
                             QStyle::SC_SliderHandle,
                             rangeSlider)
            .center();
    };

    const QPoint start = handleCenter(rangeSlider->minimum());
    const QPoint end = handleCenter(rangeSlider->maximum());

    const QPoint lower = handleCenter(rangeSlider->minimumPosition());
    const QPoint upper = handleCenter(rangeSlider->maximumPosition());

    const qreal left = std::min(start.x(), end.x());
    const qreal right = std::max(start.x(), end.x());
    const qreal centerY = start.y();

    QPainter painter(rangeSlider);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!rangeSlider->isEnabled())
    {
        painter.setOpacity(0.45);
    }

    // Full groove.
    painter.setPen(Qt::NoPen);
    painter.setBrush(grooveColor_);

    painter.drawRoundedRect(QRectF(left, centerY - 2.0, right - left, 4.0),
                            2.0,
                            2.0);

    // Selected interval between the handles.
    const qreal selectedLeft = std::min(lower.x(), upper.x());
    const qreal selectedRight = std::max(lower.x(), upper.x());

    painter.setBrush(highlightColor_);

    painter.drawRoundedRect(
        QRectF(selectedLeft, centerY - 2.0, selectedRight - selectedLeft, 4.0),
        2.0,
        2.0);

    // Paint both handles last, above the groove.
    const auto drawHandle = [&](const QPoint &center)
    {
        painter.setPen(QPen(borderColor_, 1.0));
        painter.setBrush(handleColor_);

        painter.drawEllipse(
            QRectF(center.x() - 7.0, center.y() - 7.0, 14.0, 14.0));
    };

    // Draw the active handle last when the handles overlap.
    if (rangeSlider->isMinimumSliderDown())
    {
        drawHandle(upper);
        drawHandle(lower);
    }
    else
    {
        drawHandle(lower);
        drawHandle(upper);
    }

    return true; // Skip CTK's original painting.
}
