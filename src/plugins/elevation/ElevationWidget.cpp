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

#include <ElevationAction.hpp>
#include <ElevationWidget.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ElevationWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/elevation/", name))

ElevationWidget::ElevationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Widgets
    SliderWidget::create(nPointsSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Points per cell"),
                         tr("Points per cell"),
                         tr("pt"),
                         1,
                         1000,
                         1000000,
                         10000);

    SliderWidget::create(lengthSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Cell min length"),
                         tr("Cell min length"),
                         tr("%"),
                         1,
                         1,
                         100,
                         5);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(nPointsSlider_);
    settingsLayout->addWidget(lengthSlider_);
    settingsLayout->addStretch();

    // Buttons
    applyButton_ = new QPushButton(tr("Compute"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget
    setLayout(mainLayout);
}

void ElevationWidget::slotApply()
{
    LOG_DEBUG(<< "Compute elevation.");

    mainWindow_->suspendThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsSlider_->value());
    double cellLengthMinPercent = static_cast<double>(lengthSlider_->value());

    try
    {
        ElevationAction elevation(&mainWindow_->editor());
        elevation.initialize(pointsPerCell, cellLengthMinPercent);
        ProgressDialog::run(mainWindow_, "Computing Elevation", &elevation);
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
