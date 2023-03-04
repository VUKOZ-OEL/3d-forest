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

/** @file ProjectNavigatorItemClipping.cpp */

#include <MainWindow.hpp>
#include <ProjectNavigatorItemClipping.hpp>
#include <ProjectNavigatorItemClippingBox.hpp>
#include <ProjectNavigatorItemClippingCylinder.hpp>
#include <ThemeIcon.hpp>
#include <ToolTabWidget.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ProjectNavigatorItemClipping"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorItemClipping::ProjectNavigatorItemClipping(
    MainWindow *mainWindow,
    const QIcon &icon,
    const QString &text)
    : ProjectNavigatorItem(mainWindow, icon, text)
{
    LOG_DEBUG(<< "Create.");

    // Tabs
    boxWidget_ = new ProjectNavigatorItemClippingBox(mainWindow_);
    cylinderWidget_ = new ProjectNavigatorItemClippingCylinder(mainWindow_);

    // Tab
    tabWidget_ = new ToolTabWidget;
    tabWidget_->addTab(boxWidget_, ICON("clip_filter"), tr("Box"));
    tabWidget_->addTab(cylinderWidget_, ICON("cylinder"), tr("Cylinder"));

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(tabWidget_);
    mainLayout->addStretch();

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data
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

void ProjectNavigatorItemClipping::slotUpdate(void *sender,
                                              const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (!target.empty() && !target.contains(Editor::TYPE_CLIP_FILTER))
    {
        return;
    }

    region_ = mainWindow_->editor().clipFilter();
    LOG_DEBUG(<< "Update region <" << region_ << ">.");

    boxWidget_->setRegion(region_);
    cylinderWidget_->setRegion(region_);
}

void ProjectNavigatorItemClipping::slotRegionChanged(const Region &region)
{
    LOG_DEBUG(<< "Parameter region <" << region << ">.");

    if (region.enabled == Region::TYPE_BOX)
    {
        region_.box = region.box;
        region_.enabled = region.enabled;
    }
    else if (region.enabled == Region::TYPE_CYLINDER)
    {
        region_.cylinder = region.cylinder;
        region_.enabled = region.enabled;
    }

    filterChanged();
}

void ProjectNavigatorItemClipping::filterChanged()
{
    LOG_DEBUG(<< "Called.");

    Region filter = region_;

    if (!isFilterEnabled())
    {
        filter.enabled = Region::TYPE_NONE;
    }

    mainWindow_->suspendThreads();
    mainWindow_->editor().setClipFilter(filter);
    mainWindow_->updateFilter();
}

void ProjectNavigatorItemClipping::setFilterEnabled(bool b)
{
    ProjectNavigatorItem::setFilterEnabled(b);
    filterChanged();
}
