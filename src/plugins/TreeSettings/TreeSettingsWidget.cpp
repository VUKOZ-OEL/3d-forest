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
#include <ColorSwitchWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <TreeSettingsWidget.hpp>

// Include Qt.
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "TreeSettingsWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/TreeSettingsResources/", name))

TreeSettingsWidget::TreeSettingsWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating tree settings widget.");

    // Tree attributes.
    useOnlyForSelectedTreesCheckBox_ = new QCheckBox;
    useOnlyForSelectedTreesCheckBox_->setChecked(
        settings_.useOnlyForSelectedTrees());
    useOnlyForSelectedTreesCheckBox_->setText(
        tr("Use only for selected trees"));
    connect(useOnlyForSelectedTreesCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetUseOnlyForSelectedTrees(int)));

    treeAttributesVisibleCheckBox_ = new QCheckBox;
    treeAttributesVisibleCheckBox_->setChecked(
        settings_.treeAttributesVisible());
    treeAttributesVisibleCheckBox_->setText(tr("Show tree attributes"));
    connect(treeAttributesVisibleCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetTreeAttributesVisible(int)));

    treePositionAtBottomCheckBox_ = new QCheckBox;
    treePositionAtBottomCheckBox_->setChecked(settings_.treePosition() ==
                                              TreeSettings::Position::BOTTOM);
    treePositionAtBottomCheckBox_->setText(tr("Show tree position at bottom"));
    connect(treePositionAtBottomCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetTreePositionAtBottom(int)));

    convexHullVisibleCheckBox_ = new QCheckBox;
    convexHullVisibleCheckBox_->setChecked(settings_.convexHullVisible());
    convexHullVisibleCheckBox_->setText(tr("Show convex hull"));
    connect(convexHullVisibleCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetConvexHullVisible(int)));

    convexHullProjectionVisibleCheckBox_ = new QCheckBox;
    convexHullProjectionVisibleCheckBox_->setChecked(
        settings_.convexHullVisible());
    convexHullProjectionVisibleCheckBox_->setText(
        tr("Show convex hull projection"));
    connect(convexHullProjectionVisibleCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetConvexHullProjectionVisible(int)));

    QVBoxLayout *optionsVBoxLayout = new QVBoxLayout;
    optionsVBoxLayout->addWidget(useOnlyForSelectedTreesCheckBox_);
    optionsVBoxLayout->addWidget(treeAttributesVisibleCheckBox_);
    optionsVBoxLayout->addWidget(treePositionAtBottomCheckBox_);
    optionsVBoxLayout->addWidget(convexHullVisibleCheckBox_);
    optionsVBoxLayout->addWidget(convexHullProjectionVisibleCheckBox_);

    QGroupBox *optionsGroupBox = new QGroupBox(tr("Options"));
    optionsGroupBox->setLayout(optionsVBoxLayout);

    // Layout.
    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(optionsGroupBox, 0, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(groupBoxLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());

    LOG_DEBUG(<< "Finished creating tree settings widget.");
}

void TreeSettingsWidget::slotUpdate(void *sender,
                                    const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input tree settings.");

        setTreeSettings(mainWindow_->editor().settings().treeSettings());
    }
}

void TreeSettingsWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output tree settings.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setTreeSettings(settings_);
    mainWindow_->update(this, {Editor::TYPE_SETTINGS});

    if (modifiers)
    {
        mainWindow_->updateModifiers();
    }
    else
    {
        mainWindow_->updateRender();
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

void TreeSettingsWidget::block()
{
    (void)blockSignals(true);
}

void TreeSettingsWidget::unblock()
{
    (void)blockSignals(false);
}
