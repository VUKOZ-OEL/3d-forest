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

/** @file HeightMapWindow.cpp */

#include <HeightMapModifier.hpp>
#include <HeightMapWindow.hpp>
#include <MainWindow.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#define PLUGIN_HEIGHT_MAP_NAME "Height Map"
#define PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET "Matlab Jet"
#define PLUGIN_HEIGHT_MAP_COLORMAP_VTK "VTK"
#define PLUGIN_HEIGHT_MAP_COLORMAP_GRAY "Gray"
#define PLUGIN_HEIGHT_MAP_COLORMAP_WIN_XP "Windows XP"
#define PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET
#define PLUGIN_HEIGHT_MAP_COLORS_MAX 65536
#define PLUGIN_HEIGHT_MAP_COLORS_DEFAULT 256
#define PLUGIN_HEIGHT_MAP_SOURCE_Z_POSITION "Z position"
#define PLUGIN_HEIGHT_MAP_SOURCE_ELEVATION "Elevation"
#define PLUGIN_HEIGHT_MAP_SOURCE_DEFAULT PLUGIN_HEIGHT_MAP_SOURCE_Z_POSITION

HeightMapWindow::HeightMapWindow(MainWindow *mainWindow,
                                 HeightMapModifier *modifier)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow),
      modifier_(modifier)
{
    // Widgets colormap
    colorCountSpinBox_ = new QSpinBox;
    colorCountSpinBox_->setRange(1, PLUGIN_HEIGHT_MAP_COLORS_MAX);
    colorCountSpinBox_->setValue(PLUGIN_HEIGHT_MAP_COLORS_DEFAULT);
    colorCountSpinBox_->setSingleStep(1);

    connect(colorCountSpinBox_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(colorCountChanged(int)));

    colormapComboBox_ = new QComboBox;
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_VTK);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_GRAY);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_WIN_XP);
    colormapComboBox_->setCurrentText(PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT);

    connect(colormapComboBox_,
            SIGNAL(activated(int)),
            this,
            SLOT(colorCountChanged(int)));

    sourceComboBox_ = new QComboBox;
    sourceComboBox_->addItem(PLUGIN_HEIGHT_MAP_SOURCE_Z_POSITION);
    sourceComboBox_->addItem(PLUGIN_HEIGHT_MAP_SOURCE_ELEVATION);
    sourceComboBox_->setCurrentText(PLUGIN_HEIGHT_MAP_SOURCE_DEFAULT);

    connect(sourceComboBox_,
            SIGNAL(activated(int)),
            this,
            SLOT(sourceChanged(int)));

    // Widgets apply
    previewCheckBox_ = new QCheckBox;
    connect(previewCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(previewChanged(int)));

    applyButton_ = new QPushButton(tr("Apply and save"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    applyButton_->setDisabled(true);
    connect(applyButton_,
            SIGNAL(clicked()),
            this,
            SLOT(apply()));

    // Layout
    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(new QLabel(tr("N colors")), 0, 0);
    groupBoxLayout->addWidget(colorCountSpinBox_, 0, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Colormap")), 1, 0);
    groupBoxLayout->addWidget(colormapComboBox_, 1, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Source")), 2, 0);
    groupBoxLayout->addWidget(sourceComboBox_, 2, 1);
    groupBoxLayout->setColumnStretch(1, 1);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(previewCheckBox_);
    hbox->addWidget(new QLabel(tr("Preview")));
    hbox->addStretch();
    hbox->addWidget(applyButton_, 0, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(groupBoxLayout);
    vbox->addSpacing(10);
    vbox->addLayout(hbox);
    vbox->addStretch();

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);

    setWidget(widget_);
    setWindowTitle(QObject::tr(PLUGIN_HEIGHT_MAP_NAME));
    setFloating(true);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}

void HeightMapWindow::colorCountChanged(int i)
{
    (void)i;
    modifier_->setColormap(colormapComboBox_->currentText(),
                           colorCountSpinBox_->value());
}

void HeightMapWindow::colormapChanged(int index)
{
    (void)index;
    modifier_->setColormap(colormapComboBox_->currentText(),
                           colorCountSpinBox_->value());
}

void HeightMapWindow::sourceChanged(int index)
{
    (void)index;

    HeightMapModifier::Source source;
    if (sourceComboBox_->currentText() == PLUGIN_HEIGHT_MAP_SOURCE_Z_POSITION)
    {
        source = HeightMapModifier::SOURCE_Z_POSITION;
    }
    else
    {
        source = HeightMapModifier::SOURCE_ELEVATION;
    }

    modifier_->setSource(source);
}

void HeightMapWindow::previewChanged(int index)
{
    (void)index;
    modifier_->setPreviewEnabled(previewCheckBox_->isChecked());
}

void HeightMapWindow::apply()
{
    // Filter is active during proccesing
    modifier_->setPreviewEnabled(true, false);
    modifier_->apply(mainWindow_);
    modifier_->setPreviewEnabled(previewCheckBox_->isChecked(), true, true);
}

void HeightMapWindow::closeEvent(QCloseEvent *event)
{
    if (previewCheckBox_->isChecked())
    {
        previewCheckBox_->setChecked(false);
        modifier_->setPreviewEnabled(false);
    }

    event->accept();
}
