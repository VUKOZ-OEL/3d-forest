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

/** @file TreeTablePlugin.cpp */

// Include 3D Forest.
#include <Action.hpp>
#include <Application.hpp>
#include <TreeTablePlugin.hpp>
#include <TreeTableWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeTablePlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(app_, ":/TreeTableResources/", name))

void TreeTablePlugin::initialize(Application *app)
{
    app_ = app;

    treeTableWidget_ = new TreeTableWidget(app_);

    auto *treeTableAction = new Action(tr("Tree Table"));
    treeTableAction->setToolTip(tr("Show tree table"));
    treeTableAction->setIcon(ICON("tree-table"));
    treeTableAction->setCheckable(true);
    treeTableAction->setChecked(true);
    treeTableAction->triggered.connect(
        [this]() { app_->toggleBottomWidget(treeTableWidget_); });
    app_->addNavigationItem(this,
                            {{"Tool", MAIN_WINDOW_MENU_TOOL_PRIORITY}},
                            treeTableAction);
    app_->showBottomWidget(treeTableWidget_);
}

void TreeTablePlugin::release()
{
    if (treeTableWidget_)
    {
        app_->removeBottomWidget(treeTableWidget_);
        delete treeTableWidget_;
        treeTableWidget_ = nullptr;
    }

    delete this;
}
