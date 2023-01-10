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

/** @file ClassificationWindow.cpp */

#include <ClassificationWindow.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define MODULE_NAME "ClassificationWindow"
#define ICON(name) (ThemeIcon(":/classification/", name))

ClassificationWindow::ClassificationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      classification_(&mainWindow->editor())
{
    LOG_DEBUG_GUI(MODULE_NAME, << "Window created.");

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

    SliderWidget::create(rangeSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Ground level"),
                         tr("Ground level maximum"),
                         tr("%"),
                         1,
                         1,
                         100,
                         15);

    SliderWidget::create(angleSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Ground angle"),
                         tr("Ground angle"),
                         tr("deg"),
                         1,
                         1,
                         89,
                         60);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(nPointsSlider_);
    settingsLayout->addWidget(lengthSlider_);
    settingsLayout->addWidget(rangeSlider_);
    settingsLayout->addWidget(angleSlider_);
    settingsLayout->addStretch();

    // Buttons
    applyButton_ = new QPushButton(tr("Classify"));
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
    setWindowTitle(tr("Classification"));
    setWindowIcon(ICON("soil"));
    setMaximumHeight(height());
    setModal(true);
}

void ClassificationWindow::slotApply()
{
    LOG_DEBUG_GUI(MODULE_NAME, << "Compute classification");

    mainWindow_->suspendThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsSlider_->value());
    double cellLengthMinPercent = static_cast<double>(lengthSlider_->value());
    double groundErrorPercent = static_cast<double>(rangeSlider_->value());
    double angleDeg = static_cast<double>(angleSlider_->value());

    try
    {
        classification_.initialize(pointsPerCell,
                                   cellLengthMinPercent,
                                   groundErrorPercent,
                                   angleDeg);
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

    classification_.clear();

    mainWindow_->editor().viewports().setState(Page::STATE_READ);

    mainWindow_->resumeThreads();
}
