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
    @file Forest3dClipFilter.cpp
*/

#include <Forest3dClipFilter.hpp>
#include <Project.hpp>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>

Forest3dClipFilter::Forest3dClipFilter(QWidget *parent) : QWidget(parent)
{
    // Widgets
    xMinSpinBox_ = new QDoubleSpinBox;
    connect(xMinSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    yMinSpinBox_ = new QDoubleSpinBox;
    connect(yMinSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    zMinSpinBox_ = new QDoubleSpinBox;
    connect(zMinSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    xMaxSpinBox_ = new QDoubleSpinBox;
    connect(xMaxSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    yMaxSpinBox_ = new QDoubleSpinBox;
    connect(yMaxSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    zMaxSpinBox_ = new QDoubleSpinBox;
    connect(zMaxSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(valueChanged(double)));
    enabledCheckBox_ = new QCheckBox;
    connect(enabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(stateChanged(int)));

    // Layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(new QLabel(tr("Min X")), 0, 0);
    mainLayout->addWidget(xMinSpinBox_, 0, 1);
    mainLayout->addWidget(new QLabel(tr("Min Y")), 1, 0);
    mainLayout->addWidget(yMinSpinBox_, 1, 1);
    mainLayout->addWidget(new QLabel(tr("Min Z")), 2, 0);
    mainLayout->addWidget(zMinSpinBox_, 2, 1);
    mainLayout->addWidget(new QLabel(tr("Max X")), 3, 0);
    mainLayout->addWidget(xMaxSpinBox_, 3, 1);
    mainLayout->addWidget(new QLabel(tr("Max Y")), 4, 0);
    mainLayout->addWidget(yMaxSpinBox_, 4, 1);
    mainLayout->addWidget(new QLabel(tr("Max Z")), 5, 0);
    mainLayout->addWidget(zMaxSpinBox_, 5, 1);
    mainLayout->addWidget(new QLabel(tr("Enabled")), 6, 0);
    mainLayout->addWidget(enabledCheckBox_, 6, 1);
    mainLayout->setVerticalSpacing(0);
    mainLayout->setColumnStretch(1, 1);
    setLayout(mainLayout);
}

Forest3dClipFilter::~Forest3dClipFilter()
{
}

void Forest3dClipFilter::valueChanged(double d)
{
    (void)d;
    changed();
}

void Forest3dClipFilter::stateChanged(int state)
{
    (void)state;
    changed();
}

void Forest3dClipFilter::changed()
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

void Forest3dClipFilter::updateEditor(const Editor &editor)
{
    (void)blockSignals(true);

    const ClipFilter &clipFilter = editor.project().clipFilter();
    const Aabb<double> &boundary = editor.boundary();

    xMinSpinBox_->setValue(clipFilter.box.min(0));
    yMinSpinBox_->setValue(clipFilter.box.min(1));
    zMinSpinBox_->setValue(clipFilter.box.min(2));
    xMaxSpinBox_->setValue(clipFilter.box.max(0));
    yMaxSpinBox_->setValue(clipFilter.box.max(1));
    zMaxSpinBox_->setValue(clipFilter.box.max(2));

    xMinSpinBox_->setRange(boundary.min(0), boundary.max(0));
    yMinSpinBox_->setRange(boundary.min(1), boundary.max(1));
    zMinSpinBox_->setRange(boundary.min(2), boundary.max(2));
    xMaxSpinBox_->setRange(boundary.min(0), boundary.max(0));
    yMaxSpinBox_->setRange(boundary.min(1), boundary.max(1));
    zMaxSpinBox_->setRange(boundary.min(2), boundary.max(2));

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
