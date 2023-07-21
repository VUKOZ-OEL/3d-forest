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

/** @file ElevationWidget.cpp */

#include <ElevationWidget.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ElevationWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/elevation/", name))

ElevationWidget::ElevationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      elevation_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets
    SliderWidget::create(voxelSizeSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Voxel radius"),
                         tr("Voxel radius"),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         100);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelSizeSlider_);
    settingsLayout->addStretch();

    // Buttons
    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel(tr("This tool requires pre-computed"
                                        " \nground classification.")));
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget
    setLayout(mainLayout);
}

void ElevationWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    elevation_.clear();
    QWidget::hideEvent(event);
}

void ElevationWidget::slotApply()
{
    LOG_DEBUG(<< "Compute elevation.");

    mainWindow_->suspendThreads();

    double voxelSize = static_cast<double>(voxelSizeSlider_->value());

    try
    {
        elevation_.start(voxelSize);
        ProgressDialog::run(mainWindow_, "Computing Elevation", &elevation_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_ELEVATION});
}
