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

/** @file SegmentationWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SegmentationWidget.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "SegmentationWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/segmentation/", name))

SegmentationWidget::SegmentationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      infoDialog_(nullptr),
      segmentation_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Voxel radius.
    DoubleSliderWidget::create(
        voxelRadiusSlider_,
        this,
        nullptr,
        nullptr,
        tr("Voxel radius"),
        tr("Higher voxel radius values affect the quality of the results but "
           "speed up computation and reduce disk space usage."),
        tr("m"),
        0.01,
        0.01,
        1.0,
        parameters_.voxelRadius);

    // Descriptor.
    leafToWoodChannelRadioButton_.push_back(new QRadioButton(tr("descriptor")));
    leafToWoodChannelRadioButton_.push_back(new QRadioButton(tr("intensity")));

    if (parameters_.leafToWoodChannel ==
        SegmentationParameters::CHANNEL_DESCRIPTOR)
    {
        leafToWoodChannelRadioButton_[0]->setChecked(true);
    }
    else if (parameters_.leafToWoodChannel ==
             SegmentationParameters::CHANNEL_INTENSITY)
    {
        leafToWoodChannelRadioButton_[1]->setChecked(true);
    }
    else
    {
        THROW("SegmentationParameters leafToWoodChannel not implemented.");
    }

    QVBoxLayout *trunkDescriptorChannelVBoxLayout = new QVBoxLayout;
    for (size_t i = 0; i < leafToWoodChannelRadioButton_.size(); i++)
    {
        trunkDescriptorChannelVBoxLayout->addWidget(
            leafToWoodChannelRadioButton_[i]);
    }

    QGroupBox *trunkDescriptorChannelGroupBox =
        new QGroupBox(tr("Leaf-to-wood gradient channel"));
    trunkDescriptorChannelGroupBox->setLayout(trunkDescriptorChannelVBoxLayout);

    DoubleSliderWidget::create(woodThresholdMinMinSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Minimal leaf-to-wood gradient threshold"
                                  " value"),
                               tr("Minimal leaf-to-wood gradient threshold"
                                  " value"),
                               tr("%"),
                               1.0,
                               0,
                               100.0,
                               parameters_.woodThresholdMin);

    // Search radius.
    DoubleSliderWidget::create(searchRadiusForTrunkPointsSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Maximal distance to connect trunk points"),
                               tr("Neighborhood radius to search for"
                                  " voxels which belong to the same tree."),
                               tr("m"),
                               0.01,
                               0.01,
                               1.0,
                               parameters_.searchRadiusTrunkPoints);

    DoubleSliderWidget::create(searchRadiusForLeafPointsSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Maximal distance to connect leaf points"),
                               tr("Neighborhood radius to search for"
                                  " voxels which belong to the same tree."),
                               tr("m"),
                               0.01,
                               0.01,
                               1.0,
                               parameters_.searchRadiusLeafPoints);

    // Tree.
    DoubleRangeSliderWidget::create(
        treeBaseElevationSlider_,
        this,
        nullptr,
        nullptr,
        tr("Look for tree base in elevation range"),
        tr("The values allow to cut off flying trees and prevent connecting "
           "trees by ground wood."),
        tr("m"),
        0.01,
        0.0,
        10.0,
        parameters_.treeBaseElevationMin,
        parameters_.treeBaseElevationMax);

    DoubleSliderWidget::create(treeHeightSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Minimal height of tree"),
                               tr("Minimal height of detected voxel group to"
                                  " \ndetect it as a new tree."),
                               tr("m"),
                               0.01,
                               0,
                               10.0,
                               parameters_.treeHeightMin);

    // Options.
    zCoordinatesAsElevationCheckBox_ = new QCheckBox;
    zCoordinatesAsElevationCheckBox_->setText(tr("Use z-coordinates instead of"
                                                 " ground elevation"));
    zCoordinatesAsElevationCheckBox_->setChecked(
        parameters_.zCoordinatesAsElevation);

    segmentOnlyTrunksCheckBox_ = new QCheckBox;
    segmentOnlyTrunksCheckBox_->setText(tr("Segment only trunks"
                                           " (fast preview)"));
    segmentOnlyTrunksCheckBox_->setChecked(parameters_.segmentOnlyTrunks);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelRadiusSlider_);
    settingsLayout->addWidget(trunkDescriptorChannelGroupBox);
    settingsLayout->addWidget(woodThresholdMinMinSlider_);
    settingsLayout->addWidget(searchRadiusForTrunkPointsSlider_);
    settingsLayout->addWidget(searchRadiusForLeafPointsSlider_);
    settingsLayout->addWidget(treeBaseElevationSlider_);
    settingsLayout->addWidget(treeHeightSlider_);
    settingsLayout->addWidget(zCoordinatesAsElevationCheckBox_);
    settingsLayout->addWidget(segmentOnlyTrunksCheckBox_);
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

void SegmentationWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    segmentation_.clear();
    QWidget::hideEvent(event);
}

void SegmentationWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    parameters_.leafToWoodChannel = SegmentationParameters::CHANNEL_DESCRIPTOR;
    if (leafToWoodChannelRadioButton_[SegmentationParameters::CHANNEL_INTENSITY]
            ->isChecked())
    {
        parameters_.leafToWoodChannel =
            SegmentationParameters::CHANNEL_INTENSITY;
    }

    parameters_.voxelRadius = voxelRadiusSlider_->value();
    parameters_.woodThresholdMin = woodThresholdMinMinSlider_->value();
    parameters_.searchRadiusTrunkPoints =
        searchRadiusForTrunkPointsSlider_->value();
    parameters_.searchRadiusLeafPoints =
        searchRadiusForLeafPointsSlider_->value();
    parameters_.treeBaseElevationMin = treeBaseElevationSlider_->minimumValue();
    parameters_.treeBaseElevationMax = treeBaseElevationSlider_->maximumValue();
    parameters_.treeHeightMin = treeHeightSlider_->value();

    parameters_.zCoordinatesAsElevation =
        zCoordinatesAsElevationCheckBox_->isChecked();
    parameters_.segmentOnlyTrunks = segmentOnlyTrunksCheckBox_->isChecked();

    try
    {
        segmentation_.start(parameters_);

        ProgressDialog::run(mainWindow_,
                            "Computing Segmentation",
                            &segmentation_);
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

void SegmentationWidget::slotHelp()
{
    QString t;
    t = "<h3>Automatic Segmentation Tool</h3>"
        "This tool identifies trees in point cloud. "
        "The basic idea of used segmentation algorithm is the same as in "
        "the original 3D Forest paper. "
        "The algorithm is based on point descriptors and connections "
        "between nearest neighbors. "
        "Both algorithms are specialized to classify LiDAR point clouds "
        "of complex natural forest environments."
        "<br>"
        "This tool requires either pre-computed "
        "ground classification and point elevation values "
        "or to use enabled option "
        "<i>'Use z-coordinate instead of ground elevation'</i>. "
        "Pre-computed descriptor values are always required."
        "<br><br>"
        "<img src=':/segmentation/segmentation.png'/>"
        "<div>Example dataset with calculated segmentation.</div>"
        ""
        "<h3>Segmentation Steps</h3>"
        "Segmentation steps are described on the image below."
        "<br>"
        "<img src=':/segmentation/segmentation_steps.png'/>"
        "<div>On the image:"
        " a) Original unsegmented dataset."
        " b) Shows pre-calculated descriptors from black (low)"
        " to white (high). Descriptors with high value should"
        " describe trunks."
        " c) Shows the effect of option <i>'Find only trunks'</i>."
        " 3 trunks are identified."
        " d) Shows the final result of segmented dataset."
        " Unsegmented (disconnected and ground) points are hidden."
        " These points are assigned to main segment."
        "</div>"
        ""
        "<h3>Algorithm</h3>"
        "<ol>"
        "<li>Voxelize the dataset.</li>"
        "<li>Detect individual trunks by using search radius"
        " to connect voxels which have descriptor values above"
        " user provided threshold. Assign a unique segment value"
        " to each detected trunk.</li>"
        "<li>Repeat the following for all remaining voxels:"
        "<ol>"
        "<li>Start at the next unprocessed voxel. The position"
        " of this voxel is random because the voxels are ordered"
        " by multi-segment octal-tree. This voxel creates new"
        " voxel group.</li>"
        "<li>Find minimum spanning tree from this voxel until a voxel with"
        " existing segment value is reached."
        " The spanning tree is calculated by iteratively appending"
        " the next nearest neighbor to the current voxel group.</li>"
        "<li>Set segment value of all voxels in this voxel group"
        " to segment value from terminating voxel. This connects"
        " spanning trees to trunks. Connected voxels are marked"
        " as processed.</li>"
        "</ol>"
        "</li>"
        "<li>Segment values from voxels are applied back to the"
        " dataset.</li>"
        "</ol>"
        "<br>"
        "<img src=':/segmentation/segmentation_alg.png' />"
        "<div>Top: The first step is segmentation of trunks."
        " <i>Descriptor threshold</i> is set to 0.5 (50 %)."
        " Bottom: The second step is segmentation of leaves and"
        " small branches which are connected to some trunk.</div>";

    if (!infoDialog_)
    {
        infoDialog_ = new InfoDialog(mainWindow_, 550, 450);
        infoDialog_->setWindowTitle(tr("Segmentation Help"));
        infoDialog_->setText(t);
    }

    infoDialog_->show();
    infoDialog_->raise();
    infoDialog_->activateWindow();
}
