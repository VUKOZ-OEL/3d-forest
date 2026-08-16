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

/** @file ExamplePlugin.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ExamplePlugin.hpp>
#include <ExampleWidget.hpp>
#include <Ui.hpp>

// Include local.
#define LOG_MODULE_NAME "ExamplePlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void ExamplePlugin::initialize(Application *app)
{
    LOG_DEBUG(<< "Start initializing example plugin.");
    app_ = app;
    widget_ = new ExampleWidget(app_);
    app_->addPanel({"Compute", "Example"}, widget_);

    openAction_ = new Action("A");
    app_->addMenuItem({"File", "Open"}, openAction_);
    closeAction_ = new Action("B");
    app_->addMenuItem({"File", "Close"}, closeAction_);
    LOG_DEBUG(<< "Finished initializing example plugin.");
}

void ExamplePlugin::release()
{
    if (widget_)
    {
        app_->removePanel(widget_);
        delete widget_;

        app_->removeMenuItem(openAction_);
        delete openAction_;

        app_->removeMenuItem(closeAction_);
        delete closeAction_;
    }

    delete this;
}
