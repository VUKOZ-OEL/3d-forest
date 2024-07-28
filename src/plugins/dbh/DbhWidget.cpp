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

/** @file DbhWidget.cpp */

// Include 3D Forest.
#include <DbhWidget.hpp>
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "DbhWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/dbh/", name))

DbhWidget::DbhWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      dbh_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    DoubleSliderWidget::create(elevationSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Look for DBH in elevation"),
                               tr("Look for DBH in elevation"),
                               tr("m"),
                               0.01,
                               0.5,
                               1.5,
                               parameters_.elevation);

    DoubleSliderWidget::create(elevationToleranceSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("DBH elevation tolerance"),
                               tr("DBH elevation tolerance"),
                               tr("m"),
                               0.01,
                               0.01,
                               0.5,
                               parameters_.elevationTolerance);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(elevationSlider_);
    settingsLayout->addWidget(elevationToleranceSlider_);
    settingsLayout->addStretch();

    // Buttons.
    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget.
    setLayout(mainLayout);
}

void DbhWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    dbh_.clear();
    QWidget::hideEvent(event);
}

void DbhWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    parameters_.elevation = elevationSlider_->value();
    parameters_.elevationTolerance = elevationToleranceSlider_->value();

    try
    {
        dbh_.start(parameters_);

        ProgressDialog::run(mainWindow_, "Computing Dbh", &dbh_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_SEGMENT});
}
