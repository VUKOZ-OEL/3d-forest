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

/** @file TreeAttributesWidget.cpp */

// Include 3D Forest.
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>
#include <TreeAttributesWidget.hpp>

// Include Qt.
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "TreeAttributesWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/treeattributes/", name))

TreeAttributesWidget::TreeAttributesWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      treeAttributesAction_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    DoubleSliderWidget::create(treePositionHeightRangeSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Tree position height range"),
                               tr("Tree position height range to include more "
                                  "neighboring points above the lowest tree "
                                  "point"),
                               tr("m"),
                               0.01,
                               0.01,
                               1.0,
                               parameters_.treePositionHeightRange);

    DoubleSliderWidget::create(dbhElevationSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Calculate DBH at given elevation"),
                               tr("Calculate DBH at given elevation"),
                               tr("m"),
                               0.01,
                               0.5,
                               1.5,
                               parameters_.dbhElevation);

    DoubleSliderWidget::create(dbhElevationRangeSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("DBH elevation range"),
                               tr("DBH elevation range to include more "
                                  "neighboring points above and below "
                                  "the DBH elevation value"),
                               tr("m"),
                               0.01,
                               0.01,
                               0.5,
                               parameters_.dbhElevationRange);

    DoubleSliderWidget::create(maximumValidCalculatedDbhSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Maximum valid calculated DBH"),
                               tr("Maximum valid calculated DBH"),
                               tr("m"),
                               0.01,
                               0.01,
                               10.0,
                               parameters_.maximumValidCalculatedDbh);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(treePositionHeightRangeSlider_);
    settingsLayout->addWidget(dbhElevationSlider_);
    settingsLayout->addWidget(dbhElevationRangeSlider_);
    settingsLayout->addWidget(maximumValidCalculatedDbhSlider_);
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

void TreeAttributesWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    treeAttributesAction_.clear();
    QWidget::hideEvent(event);
}

void TreeAttributesWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    parameters_.treePositionHeightRange =
        treePositionHeightRangeSlider_->value();
    parameters_.dbhElevation = dbhElevationSlider_->value();
    parameters_.dbhElevationRange = dbhElevationRangeSlider_->value();
    parameters_.maximumValidCalculatedDbh =
        maximumValidCalculatedDbhSlider_->value();

    try
    {
        treeAttributesAction_.start(parameters_);

        ProgressDialog::run(mainWindow_,
                            "Computing Tree Attributes",
                            &treeAttributesAction_);
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
