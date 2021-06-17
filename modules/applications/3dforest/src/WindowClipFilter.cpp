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

/** @file WindowClipFilter.cpp */

#include <Editor.hpp>
#include <QCheckBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <WindowClipFilter.hpp>
#include <ctkrangeslider.h>

// 6 is the number of zero after decimal point in epsilon for float
#define WINDOW_CLIP_FILTER_DECIMALS 6
#define WINDOW_CLIP_FILTER_MAX 99

WindowClipFilter::WindowClipFilter(QWidget *parent) : QWidget(parent)
{
    // Widgets
    for (size_t i = 0; i < 3; i++)
    {
        rangeSlider_[i] = new ctkRangeSlider;
        rangeSlider_[i]->setOrientation(Qt::Horizontal);
        rangeSlider_[i]->setMaximumHeight(12);
        connect(rangeSlider_[i],
                SIGNAL(minimumPositionChanged(int)),
                this,
                SLOT(setRangeMin(int)));
        connect(rangeSlider_[i],
                SIGNAL(maximumPositionChanged(int)),
                this,
                SLOT(setRangeMax(int)));

        minSpinBox_[i] = new QDoubleSpinBox;
        minSpinBox_[i]->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
        connect(minSpinBox_[i],
                SIGNAL(valueChanged(double)),
                this,
                SLOT(setValueMin(double)));

        maxSpinBox_[i] = new QDoubleSpinBox;
        maxSpinBox_[i]->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
        connect(maxSpinBox_[i],
                SIGNAL(valueChanged(double)),
                this,
                SLOT(setValueMax(double)));
    }

    enabledCheckBox_ = new QCheckBox;
    connect(enabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(setEnabled(int)));

    resetButton_ = new QPushButton(tr("&Reset"), this);
    connect(resetButton_,
            &QAbstractButton::clicked,
            this,
            &WindowClipFilter::reset);

    // Layout
    QGridLayout *mainLayout = new QGridLayout();
    int row = 0;

    mainLayout->addWidget(new QLabel(tr("X")), row, 0);
    mainLayout->addWidget(rangeSlider_[0], row, 1, 1, 2);
    row++;
    mainLayout->addWidget(new QLabel(tr("Y")), row, 0);
    mainLayout->addWidget(rangeSlider_[1], row, 1, 1, 2);
    row++;
    mainLayout->addWidget(new QLabel(tr("Z")), row, 0);
    mainLayout->addWidget(rangeSlider_[2], row, 1, 1, 2);
    row++;

    mainLayout->addWidget(new QLabel(tr("X min")), row, 0);
    mainLayout->addWidget(minSpinBox_[0], row, 1, 1, 2);
    row++;
    mainLayout->addWidget(new QLabel(tr("X max")), row, 0);
    mainLayout->addWidget(maxSpinBox_[0], row, 1, 1, 2);
    row++;

    mainLayout->addWidget(new QLabel(tr("Y min")), row, 0);
    mainLayout->addWidget(minSpinBox_[1], row, 1, 1, 2);
    row++;
    mainLayout->addWidget(new QLabel(tr("Y max")), row, 0);
    mainLayout->addWidget(maxSpinBox_[1], row, 1, 1, 2);
    row++;

    mainLayout->addWidget(new QLabel(tr("Z min")), row, 0);
    mainLayout->addWidget(minSpinBox_[2], row, 1, 1, 2);
    row++;
    mainLayout->addWidget(new QLabel(tr("Z max")), row, 0);
    mainLayout->addWidget(maxSpinBox_[2], row, 1, 1, 2);
    row++;

    mainLayout->addWidget(new QLabel(tr("Enabled")), row, 0);
    mainLayout->addWidget(enabledCheckBox_, row, 1);
    mainLayout->addWidget(resetButton_, row, 2, Qt::AlignRight);
    row++;

    mainLayout->setVerticalSpacing(0);
    mainLayout->setColumnStretch(1, 1);
    setLayout(mainLayout);

    // Window
    setFixedHeight(280);
}

WindowClipFilter::~WindowClipFilter()
{
}

void WindowClipFilter::setRangeMin(int v)
{
    QObject *obj = sender();

    for (int i = 0; i < 3; i++)
    {
        if (obj == rangeSlider_[i])
        {
            double u = static_cast<double>(v) / WINDOW_CLIP_FILTER_MAX;
            double n = minSpinBox_[i]->maximum() - minSpinBox_[i]->minimum();
            minSpinBox_[i]->setValue(minSpinBox_[i]->minimum() + (u * n));

            break;
        }
    }

    filterUpdate();
}

void WindowClipFilter::setRangeMax(int v)
{
    QObject *obj = sender();

    for (int i = 0; i < 3; i++)
    {
        if (obj == rangeSlider_[i])
        {
            double u = static_cast<double>(v) / WINDOW_CLIP_FILTER_MAX;
            double n = maxSpinBox_[i]->maximum() - maxSpinBox_[i]->minimum();
            maxSpinBox_[i]->setValue(maxSpinBox_[i]->minimum() + (u * n));

            break;
        }
    }

    filterUpdate();
}

void WindowClipFilter::setValueMin(double d)
{
    QObject *obj = sender();

    for (int i = 0; i < 3; i++)
    {
        if (obj == minSpinBox_[i])
        {
            double len = minSpinBox_[i]->maximum() - minSpinBox_[i]->minimum();
            if (len > std::numeric_limits<double>::epsilon())
            {
                d -= minSpinBox_[i]->minimum();
                int min = static_cast<int>((d / len) * WINDOW_CLIP_FILTER_MAX);
                rangeSlider_[i]->setMinimumPosition(min);
            }
            else
            {
                rangeSlider_[i]->setMinimumPosition(0);
            }

            break;
        }
    }

    filterUpdate();
}

void WindowClipFilter::setValueMax(double d)
{
    QObject *obj = sender();

    for (int i = 0; i < 3; i++)
    {
        if (obj == maxSpinBox_[i])
        {
            double len = maxSpinBox_[i]->maximum() - maxSpinBox_[i]->minimum();
            if (len > std::numeric_limits<double>::epsilon())
            {
                d -= maxSpinBox_[i]->minimum();
                int max = static_cast<int>((d / len) * WINDOW_CLIP_FILTER_MAX);
                rangeSlider_[i]->setMaximumPosition(max);
            }
            else
            {
                rangeSlider_[i]->setMaximumPosition(WINDOW_CLIP_FILTER_MAX);
            }

            break;
        }
    }

    filterUpdate();
}

void WindowClipFilter::setEnabled(int state)
{
    (void)state;
    filterUpdate();
}

void WindowClipFilter::filterUpdate()
{
    ClipFilter clipFilter;

    double x1 = minSpinBox_[0]->value();
    double y1 = minSpinBox_[1]->value();
    double z1 = minSpinBox_[2]->value();
    double x2 = maxSpinBox_[0]->value();
    double y2 = maxSpinBox_[1]->value();
    double z2 = maxSpinBox_[2]->value();
    bool checked = (enabledCheckBox_->checkState() == Qt::Checked);

    clipFilter.box.set(x1, y1, z1, x2, y2, z2);
    if (checked)
    {
        clipFilter.enabled = ClipFilter::TYPE_BOX;
    }
    else
    {
        clipFilter.enabled = ClipFilter::TYPE_NONE;
    }

    emit filterChanged(clipFilter);
}

void WindowClipFilter::reset()
{
    emit filterReset();
}

void WindowClipFilter::setClipFilter(const Editor &editor)
{
    (void)blockSignals(true);

    const Aabb<double> &boundary = editor.boundary();
    const ClipFilter &clipFilter = editor.clipFilter();

    for (size_t i = 0; i < 3; i++)
    {
        minSpinBox_[i]->setRange(boundary.min(i), boundary.max(i));
        minSpinBox_[i]->setValue(boundary.min(i));

        maxSpinBox_[i]->setRange(boundary.min(i), boundary.max(i));
        maxSpinBox_[i]->setValue(boundary.max(i));
    }

    if (clipFilter.enabled == ClipFilter::TYPE_BOX)
    {
        enabledCheckBox_->setCheckState(Qt::Checked);
    }
    else
    {
        enabledCheckBox_->setCheckState(Qt::Unchecked);
    }

    (void)blockSignals(false);
}
