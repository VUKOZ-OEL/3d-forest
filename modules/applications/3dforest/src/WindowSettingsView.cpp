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
    @file WindowSettingsView.cpp
*/

#include <QCheckBox>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QVBoxLayout>
#include <WindowSettingsView.hpp>

WindowSettingsView::WindowSettingsView(QWidget *parent) : QWidget(parent)
{
    int row;

    // Tab Visualization color source
    QGroupBox *groupBox = new QGroupBox(tr("Color Source"));
    QVBoxLayout *vbox = new QVBoxLayout;

    colorSourceCheckBox_.resize(settings_.colorSourceSize());
    for (size_t i = 0; i < colorSourceCheckBox_.size(); i++)
    {
        colorSourceCheckBox_[i] =
            new QCheckBox(tr(settings_.colorSourceString(i)));

        vbox->addWidget(colorSourceCheckBox_[i]);

        if (settings_.isColorSourceEnabled(i))
        {
            colorSourceCheckBox_[i]->setChecked(true);
        }

        connect(colorSourceCheckBox_[i],
                SIGNAL(stateChanged(int)),
                this,
                SLOT(setColorSourceEnabled(int)));
    }

    vbox->addStretch(1);
    groupBox->setLayout(vbox);

    // Tab Visualization point size
    pointSizeSlider_ = new QSlider;
    pointSizeSlider_->setMinimum(1);
    pointSizeSlider_->setMaximum(5);
    pointSizeSlider_->setSingleStep(1);
    pointSizeSlider_->setTickInterval(1);
    pointSizeSlider_->setTickPosition(QSlider::TicksAbove);
    pointSizeSlider_->setOrientation(Qt::Horizontal);
    connect(pointSizeSlider_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(setPointSize(int)));

    // Tab Visualization
    QGridLayout *visualizationLayout1 = new QGridLayout;
    row = 0;
    visualizationLayout1->addWidget(groupBox, row, 0, 1, 2);
    row++;
    visualizationLayout1->addWidget(new QLabel(tr("Point Size")), row, 0);
    visualizationLayout1->addWidget(pointSizeSlider_, row, 1);

    QWidget *visualization = new QWidget;
    QVBoxLayout *visualizationLayout = new QVBoxLayout;
    visualizationLayout->addLayout(visualizationLayout1);
    visualizationLayout->addStretch(1);
    visualization->setLayout(visualizationLayout);

    // Tab Main
    tabWidget_ = new QTabWidget;
    tabWidget_->addTab(visualization, tr("Visualization"));

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget_);
    setLayout(mainLayout);

    // Window
    setFixedHeight(240);
}

WindowSettingsView::~WindowSettingsView()
{
}

void WindowSettingsView::setColorSourceEnabled(int v)
{
    (void)v;
    for (size_t i = 0; i < colorSourceCheckBox_.size(); i++)
    {
        if (colorSourceCheckBox_[i]->isChecked())
        {
            settings_.setColorSourceEnabled(i, true);
        }
        else
        {
            settings_.setColorSourceEnabled(i, false);
        }
    }
    emit settingsColorChanged();
}

void WindowSettingsView::setPointSize(int v)
{
    settings_.setPointSize(static_cast<float>(v));
    emit settingsChanged();
}

void WindowSettingsView::setSettings(const EditorSettings::View &settings)
{
    (void)blockSignals(true);

    settings_ = settings;

    pointSizeSlider_->setValue(static_cast<int>(settings_.pointSize()));

    (void)blockSignals(false);
}
