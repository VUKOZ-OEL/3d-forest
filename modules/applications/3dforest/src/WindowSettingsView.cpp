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

#include <QComboBox>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QVBoxLayout>
#include <WindowSettingsView.hpp>

WindowSettingsView::WindowSettingsView(QWidget *parent) : QWidget(parent)
{
    int row;

    // Tab Visualization
    colorSourceComboBox_ = new QComboBox;
    colorSourceComboBox_->addItem("RGB");
    connect(colorSourceComboBox_,
            SIGNAL(activated(int)),
            this,
            SLOT(setColorIndex(int)));

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

    QGridLayout *visualizationLayout1 = new QGridLayout;
    row = 0;
    visualizationLayout1->addWidget(new QLabel(tr("Color Source")), row, 0);
    visualizationLayout1->addWidget(colorSourceComboBox_, row, 1);
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
    setFixedHeight(140);
}

WindowSettingsView::~WindowSettingsView()
{
}

void WindowSettingsView::setColorIndex(int v)
{
    (void)v;
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
