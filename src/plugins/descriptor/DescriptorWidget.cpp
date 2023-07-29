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

#include <DescriptorWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "DescriptorWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/descriptor/", name))

DescriptorWidget::DescriptorWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      infoDialog_(nullptr),
      descriptor_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets
    SliderWidget::create(radiusSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Neighborhood Radius"),
                         tr("Neighborhood Radius"),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         100);

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

    // Method
    methodRadioButton_.push_back(new QRadioButton(tr("Density")));
    methodRadioButton_.push_back(new QRadioButton(tr("PCA intensity")));

    methodRadioButton_[0]->setChecked(true);

    QVBoxLayout *methodVBoxLayout = new QVBoxLayout;
    for (size_t i = 0; i < methodRadioButton_.size(); i++)
    {
        methodVBoxLayout->addWidget(methodRadioButton_[i]);
    }

    QGroupBox *methodGroupBox = new QGroupBox(tr("Method"));
    methodGroupBox->setLayout(methodVBoxLayout);

    // Options
    groundCheckBox_ = new QCheckBox;
    groundCheckBox_->setText(tr("Include ground points"));
    groundCheckBox_->setChecked(false);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(radiusSlider_);
    settingsLayout->addWidget(voxelSizeSlider_);
    settingsLayout->addWidget(methodGroupBox);
    settingsLayout->addWidget(groundCheckBox_);
    settingsLayout->addStretch();

    // Buttons
    helpButton_ = new QPushButton(tr("Help"));
    helpButton_->setIcon(THEME_ICON("question"));
    connect(helpButton_, SIGNAL(clicked()), this, SLOT(slotHelp()));

    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(helpButton_);
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

    double radius = static_cast<double>(radiusSlider_->value());
    double voxelSize = static_cast<double>(voxelSizeSlider_->value());

    DescriptorAction::Method method = DescriptorAction::METHOD_DENSITY;
    if (methodRadioButton_[DescriptorAction::METHOD_PCA_INTENSITY]->isChecked())
    {
        method = DescriptorAction::METHOD_PCA_INTENSITY;
    }

    bool includeGround = groundCheckBox_->isChecked();

    try
    {
        descriptor_.start(radius, voxelSize, method, includeGround);
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
