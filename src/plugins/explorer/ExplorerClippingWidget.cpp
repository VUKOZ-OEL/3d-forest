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

/** @file ExplorerClippingWidget.cpp */

// Include 3D Forest.
#include <ExplorerClippingBoxWidget.hpp>
#include <ExplorerClippingCylinderWidget.hpp>
#include <ExplorerClippingWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <ToolTabWidget.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerClippingWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/explorer/", name))

ExplorerClippingWidget::ExplorerClippingWidget(MainWindow *mainWindow,
                                               const QIcon &icon,
                                               const QString &text)
    : ExplorerWidgetInterface(mainWindow, icon, text)
{
    LOG_DEBUG(<< "Create.");

    // Tabs.
    boxWidget_ = new ExplorerClippingBoxWidget(mainWindow_);
    cylinderWidget_ = new ExplorerClippingCylinderWidget(mainWindow_);

    // Tab.
    tabWidget_ = new ToolTabWidget;
    tabWidget_->addTab(boxWidget_,
                       ICON("selection_box"),
                       tr("Box"),
                       tr("Box clip filter"));
    tabWidget_->addTab(cylinderWidget_,
                       ICON("selection_cylinder"),
                       tr("Cylinder"),
                       tr("Cylinder clip filter"));

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(tabWidget_);
    mainLayout->addStretch();

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    connect(boxWidget_,
            SIGNAL(signalRegionChanged(const Region &)),
            this,
            SLOT(slotRegionChanged(const Region &)));

    connect(cylinderWidget_,
            SIGNAL(signalRegionChanged(const Region &)),
            this,
            SLOT(slotRegionChanged(const Region &)));
}

void ExplorerClippingWidget::slotUpdate(void *sender,
                                        const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_CLIP_FILTER) ||
        target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_TRACE_UPDATE(<< "Input clip filter.");

        region_ = mainWindow_->editor().clipFilter();
        LOG_DEBUG(<< "Set region <" << region_ << ">.");

        boxWidget_->setRegion(region_);
        cylinderWidget_->setRegion(region_);
    }
}

void ExplorerClippingWidget::filterChanged()
{
    LOG_TRACE_UPDATE(<< "Output clip filter.");

    Region filter = region_;

    if (!isFilterEnabled())
    {
        filter.shape = Region::SHAPE_NONE;
    }

    mainWindow_->suspendThreads();
    mainWindow_->editor().setClipFilter(filter);
    mainWindow_->updateFilter();
}

void ExplorerClippingWidget::slotRegionChanged(const Region &region)
{
    LOG_DEBUG(<< "Set clip region <" << region << ">.");

    if (region.shape == Region::SHAPE_BOX)
    {
        region_.box = region.box;
        region_.shape = region.shape;
    }
    else if (region.shape == Region::SHAPE_CYLINDER)
    {
        region_.cylinder = region.cylinder;
        region_.shape = region.shape;
    }

    filterChanged();
}

void ExplorerClippingWidget::setFilterEnabled(bool b)
{
    ExplorerWidgetInterface::setFilterEnabled(b);
    filterChanged();
}
