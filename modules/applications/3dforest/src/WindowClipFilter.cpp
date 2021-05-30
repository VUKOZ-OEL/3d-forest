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

/**
    @file WindowClipFilter.cpp
*/

#include <Editor.hpp>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <WindowClipFilter.hpp>

#define WINDOW_CLIP_FILTER_DECIMALS 6

WindowClipFilter::WindowClipFilter(QWidget *parent) : QWidget(parent)
{
    // Widgets
    xMinSpinBox_ = new QDoubleSpinBox;
    xMinSpinBox_->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
    connect(xMinSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    yMinSpinBox_ = new QDoubleSpinBox;
    yMinSpinBox_->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
    connect(yMinSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    zMinSpinBox_ = new QDoubleSpinBox;
    zMinSpinBox_->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
    connect(zMinSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));

    xMaxSpinBox_ = new QDoubleSpinBox;
    xMaxSpinBox_->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
    connect(xMaxSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    yMaxSpinBox_ = new QDoubleSpinBox;
    yMaxSpinBox_->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
    connect(yMaxSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    zMaxSpinBox_ = new QDoubleSpinBox;
    zMaxSpinBox_->setDecimals(WINDOW_CLIP_FILTER_DECIMALS);
    connect(zMaxSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));

    enabledCheckBox_ = new QCheckBox;
    connect(enabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(stateChanged(int)));

    resetButton_ = new QPushButton(tr("&Reset"), this);
    connect(resetButton_,
            &QAbstractButton::clicked,
            this,
            &WindowClipFilter::reset);

    // Layout
    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(new QLabel(tr("X min")), 0, 0);
    mainLayout->addWidget(xMinSpinBox_, 0, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("X max")), 1, 0);
    mainLayout->addWidget(xMaxSpinBox_, 1, 1, 1, 2);

    mainLayout->addWidget(new QLabel(tr("Y min")), 2, 0);
    mainLayout->addWidget(yMinSpinBox_, 2, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("Y max")), 3, 0);
    mainLayout->addWidget(yMaxSpinBox_, 3, 1, 1, 2);

    mainLayout->addWidget(new QLabel(tr("Z min")), 4, 0);
    mainLayout->addWidget(zMinSpinBox_, 4, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("Z max")), 5, 0);
    mainLayout->addWidget(zMaxSpinBox_, 5, 1, 1, 2);

    mainLayout->addWidget(new QLabel(tr("Enabled")), 6, 0);
    mainLayout->addWidget(enabledCheckBox_, 6, 1);
    mainLayout->addWidget(resetButton_, 6, 2, Qt::AlignRight);

    mainLayout->setVerticalSpacing(0);
    mainLayout->setColumnStretch(1, 1);
    setLayout(mainLayout);

    setFixedHeight(200);
}

WindowClipFilter::~WindowClipFilter()
{
}

void WindowClipFilter::valueChanged(double d)
{
    (void)d;
    changed();
}

void WindowClipFilter::stateChanged(int state)
{
    (void)state;
    changed();
}

void WindowClipFilter::changed()
{
    ClipFilter clipFilter;

    double x1 = xMinSpinBox_->value();
    double y1 = yMinSpinBox_->value();
    double z1 = zMinSpinBox_->value();
    double x2 = xMaxSpinBox_->value();
    double y2 = yMaxSpinBox_->value();
    double z2 = zMaxSpinBox_->value();
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

void WindowClipFilter::updateEditor(const Editor &editor)
{
    (void)blockSignals(true);

    const Aabb<double> &boundary = editor.boundary();
    xMinSpinBox_->setRange(boundary.min(0), boundary.max(0));
    yMinSpinBox_->setRange(boundary.min(1), boundary.max(1));
    zMinSpinBox_->setRange(boundary.min(2), boundary.max(2));
    xMaxSpinBox_->setRange(boundary.min(0), boundary.max(0));
    yMaxSpinBox_->setRange(boundary.min(1), boundary.max(1));
    zMaxSpinBox_->setRange(boundary.min(2), boundary.max(2));

    const ClipFilter &clipFilter = editor.clipFilter();
    xMinSpinBox_->setValue(clipFilter.box.min(0));
    yMinSpinBox_->setValue(clipFilter.box.min(1));
    zMinSpinBox_->setValue(clipFilter.box.min(2));
    xMaxSpinBox_->setValue(clipFilter.box.max(0));
    yMaxSpinBox_->setValue(clipFilter.box.max(1));
    zMaxSpinBox_->setValue(clipFilter.box.max(2));

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
