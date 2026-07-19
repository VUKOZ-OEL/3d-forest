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

/** @file ProjectFilePlugin.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ProjectFileAction.hpp>
#include <ProjectFilePlugin.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "ProjectFilePlugin"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ProjectFileResources/", name))

ProjectFilePlugin::ProjectFilePlugin()
{
}

void ProjectFilePlugin::initialize(Application *app)
{
    LOG_DEBUG(<< "Initialize.");
    app_ = app;

    app_->createAction(
        &newProjectAction_,
        "File",
        "File Project",
        tr("New Project"),
        tr("Create new project"),
        ICON("create"),
        [this]() { slotNewProject(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        10);

    app_->createAction(
        &openProjectAction_,
        "File",
        "File Project",
        tr("Open Project..."),
        tr("Open project"),
        ICON("opened-folder"),
        [this]() { slotOpenProject(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        20);

    app_->createAction(
        &saveProjectAction_,
        "File",
        "File Project",
        tr("Save Project"),
        tr("Save project"),
        ICON("save"),
        [this]() { slotSaveProject(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        30);

    app_->createAction(
        &saveAsProjectAction_,
        "File",
        "File Project",
        tr("Save Project &As..."),
        tr("Save project as"),
        ICON("save-as"),
        [this]() { slotSaveAsProject(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        40);

    app_->createAction(
        &reloadProjectAction_,
        "File",
        "File Project",
        tr("Reload Project"),
        tr("Reload Project"),
        ICON("reload"),
        [this]() { slotReloadProject(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        45);

    app_->hideToolBar("File Project");
}

void ProjectFilePlugin::slotNewProject()
{
    (void)ProjectFileAction::newProject(app_);
}

void ProjectFilePlugin::slotOpenProject()
{
    (void)ProjectFileAction::openProject(app_);
}

void ProjectFilePlugin::slotSaveProject()
{
    (void)ProjectFileAction::saveProject(app_);
}

void ProjectFilePlugin::slotSaveAsProject()
{
    (void)ProjectFileAction::saveAsProject(app_);
}

void ProjectFilePlugin::slotReloadProject()
{
    (void)ProjectFileAction::reloadProject(app_);
}

bool ProjectFilePlugin::closeProject()
{
    return ProjectFileAction::closeProject(app_);
}
