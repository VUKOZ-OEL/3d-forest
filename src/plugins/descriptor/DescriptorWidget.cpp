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

/** @file DescriptorWidget.cpp */

// Include 3D Forest.
#include <DescriptorWidget.hpp>
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "DescriptorWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/descriptor/", name))

DescriptorWidget::DescriptorWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      infoDialog_(nullptr),
      descriptor_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Method.
    methodRadioButton_.push_back(new QRadioButton(tr("Density")));
    methodRadioButton_.push_back(new QRadioButton(tr("PCA intensity")));

    if (parameters_.method == DescriptorParameters::METHOD_DENSITY)
    {
        methodRadioButton_[0]->setChecked(true);
    }
    else if (parameters_.method == DescriptorParameters::METHOD_PCA_INTENSITY)
    {
        methodRadioButton_[1]->setChecked(true);
    }
    else
    {
        THROW("DescriptorParameters method not implemented.");
    }

    QVBoxLayout *methodVBoxLayout = new QVBoxLayout;
    for (size_t i = 0; i < methodRadioButton_.size(); i++)
    {
        methodVBoxLayout->addWidget(methodRadioButton_[i]);
    }

    QGroupBox *methodGroupBox = new QGroupBox(tr("Method"));
    methodGroupBox->setLayout(methodVBoxLayout);

    // Widgets.
    DoubleSliderWidget::create(voxelRadiusSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Voxel radius"),
                               tr("Voxel radius"),
                               tr("m"),
                               0.01,
                               0.01,
                               1.0,
                               parameters_.voxelRadius);

    DoubleSliderWidget::create(searchRadiusSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Neighborhood search radius"),
                               tr("Neighborhood search radius"),
                               tr("m"),
                               0.01,
                               0.01,
                               2.0,
                               parameters_.searchRadius);

    // Options.
    includeGroundPointsCheckBox_ = new QCheckBox;
    includeGroundPointsCheckBox_->setText(tr("Include ground points"));
    includeGroundPointsCheckBox_->setChecked(parameters_.includeGroundPoints);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(methodGroupBox);
    settingsLayout->addWidget(voxelRadiusSlider_);
    settingsLayout->addWidget(searchRadiusSlider_);
    settingsLayout->addWidget(includeGroundPointsCheckBox_);
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

void DescriptorWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    descriptor_.clear();
    QWidget::hideEvent(event);
}

void DescriptorWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    parameters_.method = DescriptorParameters::METHOD_DENSITY;
    if (methodRadioButton_[DescriptorParameters::METHOD_PCA_INTENSITY]
            ->isChecked())
    {
        parameters_.method = DescriptorParameters::METHOD_PCA_INTENSITY;
    }

    parameters_.voxelRadius = voxelRadiusSlider_->value();
    parameters_.searchRadius = searchRadiusSlider_->value();

    parameters_.includeGroundPoints = includeGroundPointsCheckBox_->isChecked();

    try
    {
        descriptor_.start(parameters_);
        ProgressDialog::run(mainWindow_, "Computing Descriptors", &descriptor_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    descriptor_.clear();

    mainWindow_->update({Editor::TYPE_DESCRIPTOR});
}

void DescriptorWidget::slotHelp()
{
    QString t;
    t = "<h3>Descriptor Tool</h3>"
        "This tool calculates point descriptor values. "
        "Descriptors are calculated from <i>Neighborhood Radius</i> "
        "of each point. The point cloud can be voxelized to speed up "
        "the calculation. There are several methods to choose from. "
        "Basic <i>Density Method</i> calculates global density of each "
        "voxel. Calculated descriptors are normalized to range from "
        "zero (global minimum) to one (global maximum). "
        "Descriptor is additional point attribute added by 3D Forest. "
        "<br><br>"
        "<img src=':/descriptor/descriptor_density.png'/>"
        "<div>Example dataset (Before) with calculated density"
        " descriptors (After).</div>"
        ""
        "<h3>Algorithm for Density</h3>"
        "Count the number of points in <i>Neighborhood Radius</i>."
        ""
        "<h3>Algorithm for PCA Intensity</h3>"
        "Calculate PCA of points in <i>Neighborhood Radius</i>. "
        "Descriptor value is ratio between length of longest projected "
        "eigen vector to sum of lengths of all projected eigen vectors.";

    if (!infoDialog_)
    {
        infoDialog_ = new InfoDialog(mainWindow_, 450, 450);
        infoDialog_->setWindowTitle(tr("Descriptor Help"));
        infoDialog_->setText(t);
    }

    infoDialog_->show();
    infoDialog_->raise();
    infoDialog_->activateWindow();
}
