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

/** @file TreeSettingsWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <CheckBox.hpp>
#include <Color.hpp>
#include <ComboBox.hpp>
#include <GridLayout.hpp>
#include <GroupBox.hpp>
#include <Label.hpp>
#include <Slider.hpp>
#include <ThemeIcon.hpp>
#include <TreeSettingsWidget.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeSettingsWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/TreeSettingsResources/", name))

TreeSettingsWidget::TreeSettingsWidget(Application *app)
    : Widget(app),
      app_(app)
{
    LOG_DEBUG(<< "Start creating tree settings widget.");

    // Tree attributes.
    useOnlyForSelectedTreesCheckBox_ = new CheckBox;
    useOnlyForSelectedTreesCheckBox_->setChecked(
        settings_.useOnlyForSelectedTrees());
    useOnlyForSelectedTreesCheckBox_->setText(
        tr("Use only for selected trees"));
    useOnlyForSelectedTreesCheckBox_->stateChanged.connect(
        [this](int val) { slotSetUseOnlyForSelectedTrees(val); });

    treeAttributesVisibleCheckBox_ = new CheckBox;
    treeAttributesVisibleCheckBox_->setChecked(
        settings_.treeAttributesVisible());
    treeAttributesVisibleCheckBox_->setText(tr("Show tree attributes"));
    treeAttributesVisibleCheckBox_->stateChanged.connect(
        [this](int val) { slotSetTreeAttributesVisible(val); });

    treePositionAtBottomCheckBox_ = new CheckBox;
    treePositionAtBottomCheckBox_->setChecked(settings_.treePosition() ==
                                              TreeSettings::Position::BOTTOM);
    treePositionAtBottomCheckBox_->setText(tr("Show tree position at bottom"));
    treePositionAtBottomCheckBox_->stateChanged.connect(
        [this](int val) { slotSetTreePositionAtBottom(val); });

    // Convex hull.
    convexHullVisibleCheckBox_ = new CheckBox;
    convexHullVisibleCheckBox_->setChecked(settings_.convexHullVisible());
    convexHullVisibleCheckBox_->setText(tr("Show convex hull"));
    convexHullVisibleCheckBox_->stateChanged.connect(
        [this](int val) { slotSetConvexHullVisible(val); });

    convexHullProjectionVisibleCheckBox_ = new CheckBox;
    convexHullProjectionVisibleCheckBox_->setChecked(
        settings_.convexHullVisible());
    convexHullProjectionVisibleCheckBox_->setText(
        tr("Show convex hull projection"));
    convexHullProjectionVisibleCheckBox_->stateChanged.connect(
        [this](int val) { slotSetConvexHullProjectionVisible(val); });

    // Concave hull.
    concaveHullVisibleCheckBox_ = new CheckBox;
    concaveHullVisibleCheckBox_->setChecked(settings_.concaveHullVisible());
    concaveHullVisibleCheckBox_->setText(tr("Show concave hull"));
    concaveHullVisibleCheckBox_->stateChanged.connect(
        [this](int val) { slotSetConcaveHullVisible(val); });

    concaveHullProjectionVisibleCheckBox_ = new CheckBox;
    concaveHullProjectionVisibleCheckBox_->setChecked(
        settings_.concaveHullVisible());
    concaveHullProjectionVisibleCheckBox_->setText(
        tr("Show concave hull projection"));
    concaveHullProjectionVisibleCheckBox_->stateChanged.connect(
        [this](int val) { slotSetConcaveHullProjectionVisible(val); });

    // DBH scale.
    dbhScaleSlider_ = new Slider;
    dbhScaleSlider_->setMinimum(1);
    dbhScaleSlider_->setMaximum(10);
    dbhScaleSlider_->setSingleStep(1);
    dbhScaleSlider_->setTickInterval(1);
    dbhScaleSlider_->setTickPosition(Slider::TicksAbove);
    dbhScaleSlider_->setOrientation(Ui::Horizontal);
    dbhScaleSlider_->valueChanged.connect([this](int val)
                                          { slotSetDbhScale(val); });

    // Options.
    VBoxLayout *optionsVBoxLayout = new VBoxLayout;
    optionsVBoxLayout->addWidget(useOnlyForSelectedTreesCheckBox_);
    optionsVBoxLayout->addWidget(treeAttributesVisibleCheckBox_);
    optionsVBoxLayout->addWidget(treePositionAtBottomCheckBox_);
    optionsVBoxLayout->addWidget(convexHullVisibleCheckBox_);
    optionsVBoxLayout->addWidget(convexHullProjectionVisibleCheckBox_);
    optionsVBoxLayout->addWidget(concaveHullVisibleCheckBox_);
    optionsVBoxLayout->addWidget(concaveHullProjectionVisibleCheckBox_);

    GroupBox *optionsGroupBox = new GroupBox(tr("Options"));
    optionsGroupBox->setLayout(optionsVBoxLayout);

    // Layout.
    GridLayout *groupBoxLayout = new GridLayout;
    groupBoxLayout->addWidget(optionsGroupBox, 0, 0, 1, 2);
    groupBoxLayout->addWidget(new Label(tr("DBH scale:")), 1, 0);
    groupBoxLayout->addWidget(dbhScaleSlider_, 1, 1);

    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->addLayout(groupBoxLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect(
        [this](void *sender, const std::set<Editor::Type> &target)
        { slotUpdate(sender, target); });

    slotUpdate(nullptr, std::set<Editor::Type>());

    LOG_DEBUG(<< "Finished creating tree settings widget.");
}

void TreeSettingsWidget::slotUpdate(void *sender,
                                    const std::set<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input tree settings.");

        setTreeSettings(app_->editor().settings().treeSettings());
    }
}

void TreeSettingsWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output tree settings.");

    app_->suspendThreads();
    app_->editor().setTreeSettings(settings_);
    app_->emitUpdate(this, {Editor::TYPE_SETTINGS});

    if (modifiers)
    {
        app_->updateModifiers();
    }
    else
    {
        app_->updateRender();
    }
}

void TreeSettingsWidget::setTreeSettings(const TreeSettings &settings)
{
    LOG_DEBUG(<< "Set tree settings <" << toString(settings) << ">.");

    block();

    settings_ = settings;

    // Use only for selected/all trees.
    useOnlyForSelectedTreesCheckBox_->setChecked(
        settings_.useOnlyForSelectedTrees());

    // Tree attributes.
    treeAttributesVisibleCheckBox_->setChecked(
        settings_.treeAttributesVisible());

    // Tree position.
    treePositionAtBottomCheckBox_->setChecked(settings_.treePosition() ==
                                              TreeSettings::Position::BOTTOM);

    // Convex hull.
    convexHullVisibleCheckBox_->setChecked(settings_.convexHullVisible());

    // Convex hull projection.
    convexHullProjectionVisibleCheckBox_->setChecked(
        settings_.convexHullProjectionVisible());

    // Concave hull.
    concaveHullVisibleCheckBox_->setChecked(settings_.concaveHullVisible());

    // Concave hull projection.
    concaveHullProjectionVisibleCheckBox_->setChecked(
        settings_.concaveHullProjectionVisible());

    // DBH scale.
    dbhScaleSlider_->setValue(static_cast<int>(settings_.dbhScale()));

    unblock();
}

void TreeSettingsWidget::slotSetUseOnlyForSelectedTrees(int v)
{
    (void)v;
    settings_.setUseOnlyForSelectedTrees(
        useOnlyForSelectedTreesCheckBox_->isChecked());
    dataChanged();
}

void TreeSettingsWidget::slotSetTreeAttributesVisible(int v)
{
    (void)v;
    settings_.setTreeAttributesVisible(
        treeAttributesVisibleCheckBox_->isChecked());
    dataChanged();
}

void TreeSettingsWidget::slotSetTreePositionAtBottom(int v)
{
    (void)v;

    if (treePositionAtBottomCheckBox_->isChecked())
    {
        settings_.setTreePosition(TreeSettings::Position::BOTTOM);
    }
    else
    {
        settings_.setTreePosition(TreeSettings::Position::TOP);
    }

    dataChanged();
}

void TreeSettingsWidget::slotSetConvexHullVisible(int v)
{
    (void)v;
    settings_.setConvexHullVisible(convexHullVisibleCheckBox_->isChecked());
    dataChanged();
}

void TreeSettingsWidget::slotSetConvexHullProjectionVisible(int v)
{
    (void)v;
    settings_.setConvexHullProjectionVisible(
        convexHullProjectionVisibleCheckBox_->isChecked());
    dataChanged();
}

void TreeSettingsWidget::slotSetConcaveHullVisible(int v)
{
    (void)v;
    settings_.setConcaveHullVisible(concaveHullVisibleCheckBox_->isChecked());
    dataChanged();
}

void TreeSettingsWidget::slotSetConcaveHullProjectionVisible(int v)
{
    (void)v;
    settings_.setConcaveHullProjectionVisible(
        concaveHullProjectionVisibleCheckBox_->isChecked());
    dataChanged();
}

void TreeSettingsWidget::slotSetDbhScale(int v)
{
    settings_.setDbhScale(static_cast<double>(v));
    dataChanged();
}

void TreeSettingsWidget::block()
{
    (void)blockSignals(true);
}

void TreeSettingsWidget::unblock()
{
    (void)blockSignals(false);
}
