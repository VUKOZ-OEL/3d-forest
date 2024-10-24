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

/** @file ComputeClassificationWidget.cpp */

// Include 3D Forest.
#include <ComputeClassificationWidget.hpp>
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ComputeClassificationWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeClassificationResources/", name))

ComputeClassificationWidget::ComputeClassificationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      infoDialog_(nullptr),
      classification_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    DoubleSliderWidget::create(voxelSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Voxel radius"),
                               tr("Voxel radius."),
                               tr("m"),
                               0.01,
                               0.01,
                               1.0,
                               parameters_.voxelRadius);

    DoubleSliderWidget::create(radiusSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Neighborhood search radius"),
                               tr("Neighborhood search radius."),
                               tr("m"),
                               0.01,
                               0.01,
                               2.0,
                               parameters_.searchRadius);

    DoubleSliderWidget::create(angleSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Maximum ground angle"),
                               tr("Maximum ground angle."),
                               tr("deg"),
                               1.0,
                               1.0,
                               89.0,
                               parameters_.angle);

    cleanGroundCheckBox_ = new QCheckBox;
    cleanGroundCheckBox_->setText(tr("Clean ground classifications at start"));
    cleanGroundCheckBox_->setChecked(parameters_.cleanGroundClassifications);

    cleanAllCheckBox_ = new QCheckBox;
    cleanAllCheckBox_->setText(tr("Clean all classifications at start"));
    cleanAllCheckBox_->setChecked(parameters_.cleanAllClassifications);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelSlider_);
    settingsLayout->addWidget(radiusSlider_);
    settingsLayout->addWidget(angleSlider_);
    settingsLayout->addWidget(cleanGroundCheckBox_);
    settingsLayout->addWidget(cleanAllCheckBox_);
    settingsLayout->addStretch();

    // Buttons.
    helpButton_ = new QPushButton(tr("Help"));
    helpButton_->setIcon(THEME_ICON("question"));
    connect(helpButton_, SIGNAL(clicked()), this, SLOT(slotHelp()));

    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(helpButton_);
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

void ComputeClassificationWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    classification_.clear();
    QWidget::hideEvent(event);
}

void ComputeClassificationWidget::slotApply()
{
    LOG_DEBUG(<< "Compute classification.");

    mainWindow_->suspendThreads();

    parameters_.voxelRadius = voxelSlider_->value();
    parameters_.searchRadius = static_cast<double>(radiusSlider_->value());
    parameters_.angle = static_cast<double>(angleSlider_->value());
    parameters_.cleanGroundClassifications = cleanGroundCheckBox_->isChecked();
    parameters_.cleanAllClassifications = cleanAllCheckBox_->isChecked();

    try
    {
        classification_.start(parameters_);
        ProgressDialog::run(mainWindow_,
                            "Compute Classification",
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

void ComputeClassificationWidget::slotHelp()
{
    QString t = "<h3>Compute Classification</h3>"
                "This tool calculates classification of ground points. "
                "It uses new algorithm which is specialized to classify "
                "LiDAR point clouds of complex natural forest environments. "
                "The algorithm is based on global minimum to deal with "
                "missing data in non scanned or obstructed parts. "
                "<br><br>"
                "<img "
                "src=':/ComputeClassificationResources/classification.png' "
                "width='362' height='388'/>"
                "<div>Example dataset with classified ground.</div>"
                ""
                "<h3>Algorithm</h3>"
                "<ol>"
                "<li>Voxelize the dataset.</li>"
                "<li>Find voxel with minimal z coordinate and append"
                " this voxel to working set W.</li>"
                "<li>While W is not processed, append other"
                " voxels in search radius from each new voxel in W, if"
                " selection cone given by maximal ground angle and"
                " their position does not contain any voxels, eg."
                " there is nothing below. Voxel is marked as processed"
                " when it searched for its neighbors.</li>"
                "<li>All voxels in W are classified as ground points.</li>"
                "<li>Voxel values are applied back to the dataset.</li>"
                "</ol>"
                "<br>"
                "<img "
                "src=':/ComputeClassificationResources/classification-alg.png' "
                "/>";

    if (!infoDialog_)
    {
        infoDialog_ = new InfoDialog(mainWindow_, 450, 450);
        infoDialog_->setWindowTitle(tr("Compute Classification Help"));
        infoDialog_->setText(t);
    }

    infoDialog_->show();
    infoDialog_->raise();
    infoDialog_->activateWindow();
}
