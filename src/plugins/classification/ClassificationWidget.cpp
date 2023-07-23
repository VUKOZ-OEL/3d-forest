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

/** @file ClassificationWidget.cpp */

#include <ClassificationWidget.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ClassificationWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/classification/", name))

ClassificationWidget::ClassificationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      classification_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets
    SliderWidget::create(voxelSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Voxel radius"),
                         tr("Voxel radius."),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         100);

    SliderWidget::create(radiusSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Search radius"),
                         tr("Search radius."),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         400);

    SliderWidget::create(angleSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Maximum ground angle"),
                         tr("Maximum ground angle"),
                         tr("deg"),
                         1,
                         1,
                         89,
                         60);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelSlider_);
    settingsLayout->addWidget(radiusSlider_);
    settingsLayout->addWidget(angleSlider_);
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
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget
    setLayout(mainLayout);
}

void ClassificationWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    classification_.clear();
    QWidget::hideEvent(event);
}

void ClassificationWidget::slotApply()
{
    LOG_DEBUG(<< "Compute classification.");

    mainWindow_->suspendThreads();

    double voxel = static_cast<double>(voxelSlider_->value());
    double radius = static_cast<double>(radiusSlider_->value());
    double angle = static_cast<double>(angleSlider_->value());

    try
    {
        classification_.start(voxel, radius, angle);
        ProgressDialog::run(mainWindow_,
                            "Computing Classification",
                            &classification_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_CLASSIFICATION, Editor::TYPE_ELEVATION});
}
