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

/** @file DensityWindow.cpp */

#include <DensityAction.hpp>
#include <DensityWindow.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define MODULE_NAME "DensityWindow"
#define ICON(name) (ThemeIcon(":/density/", name))

DensityWindow::DensityWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG_GUI(MODULE_NAME, << "Window created.");

    // Widgets
    SliderWidget::create(radius_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Radius"),
                         tr("Radius"),
                         tr("pt"),
                         1,
                         10,
                         1000,
                         1000);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(radius_);
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

    // Dialog
    setLayout(mainLayout);
    setWindowTitle(tr("Density"));
    setWindowIcon(ICON("density"));
    setMaximumHeight(height());
    setModal(true);
}

void DensityWindow::slotApply()
{
    double radius = static_cast<double>(radius_->value());

    LOG_DEBUG_GUI(MODULE_NAME,
                  << "Compute density with radius<" << radius << ">");

    mainWindow_->suspendThreads();

    try
    {
        DensityAction computeDensity(&mainWindow_->editor());
        computeDensity.initialize(radius);
        ProgressDialog::run(mainWindow_, "Computing Density", &computeDensity);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_DENSITY});
}
