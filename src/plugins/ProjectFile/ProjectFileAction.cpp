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

/** @file ProjectFileAction.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <Editor.hpp>
#include <FileDialog.hpp>
#include <MessageBox.hpp>
#include <ProjectFileAction.hpp>

// Include local.
#define LOG_MODULE_NAME "ProjectFileAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define PROJECT_FILE_PLUGIN_FILTER_PRJ "3DForest Project (*.json)"

bool ProjectFileAction::newProject(Application *app)
{
    LOG_DEBUG(<< "Start creating a new project.");

    // Close the current project.
    if (!closeProject(app))
    {
        LOG_DEBUG(<< "Cancelled, the current project can not be closed.");
        return false;
    }

    // Update.
    app->updateNewProject();
    app->slotRenderViewports();

    LOG_DEBUG(<< "Finished creating new project.");
    return true;
}

bool ProjectFileAction::openProject(Application *app)
{
    LOG_DEBUG(<< "Start opening a project.");

    std::string fileName;

    fileName =
        app->getOpenFileName("Open Project", PROJECT_FILE_PLUGIN_FILTER_PRJ);

    if (fileName.empty())
    {
        LOG_DEBUG(<< "Cancelled, the filename is empty.");
        return false;
    }

    bool result = openProject(app, fileName);

    LOG_DEBUG(<< "Finished opening project <" << fileName << ">.");
    return result;
}

bool ProjectFileAction::saveProject(Application *app)
{
    LOG_DEBUG(<< "Start saving the project.");

    bool result = saveProject(app, "");

    LOG_DEBUG(<< "Finished saving the project.");
    return result;
}

bool ProjectFileAction::saveAsProject(Application *app)
{
    LOG_DEBUG(<< "Start saving the project as.");

    std::string fileName;

    fileName = FileDialog::getSaveFileName(app,
                                           "Save Project As",
                                           PROJECT_FILE_PLUGIN_FILTER_PRJ);

    if (fileName.empty())
    {
        LOG_DEBUG(<< "Cancelled, the filename is empty.");
        return false;
    }

    bool result = saveProject(app, fileName);

    LOG_DEBUG(<< "Finished saving the project as <" << fileName << ">.");
    return result;
}

bool ProjectFileAction::reloadProject(Application *app)
{
    LOG_DEBUG(<< "Start reloading the project.");

    app->suspendThreads();

    if (app->editor().unsavedChanges())
    {
        LOG_DEBUG(<< "Project has unsaved changes.");

        MessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Please save the changes first.");
        msgBox.setStandardButtons(MessageBox::Ok);
        msgBox.setDefaultButton(MessageBox::Ok);
        (void)msgBox.exec();

        return false;
    }

    try
    {
        app->editor().reload();
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        app->showError(e.what());
        return false;
    }

    // Update.
    app->updateNewProject();
    app->slotRenderViewports();

    LOG_DEBUG(<< "Finished reloading the project.");
    return true;
}

bool ProjectFileAction::openProject(Application *app, const std::string &path)
{
    LOG_DEBUG(<< "Start opening new project <" << path << ">.");

    // Close the current project.
    if (!closeProject(app))
    {
        LOG_DEBUG(<< "Cancelled, the current project can not be closed.");
        return false;
    }

    // Open new project.
    try
    {
        app->editor().open(path);
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        app->showError(e.what());
        return false;
    }

    // Update.
    app->updateNewProject();
    app->slotRenderViewports();

    LOG_DEBUG(<< "Finished opening new project.");

    return true; // Opened
}

bool ProjectFileAction::saveProject(Application *app, const std::string &path)
{
    LOG_DEBUG(<< "Start saving the project to path <" << path << ">.");

    std::string writePath;

    app->suspendThreads();

    if (path.empty())
    {
        // Save.
        if (app->editor().projectPath().empty())
        {
            // First time save.
            std::string fileName;

            fileName =
                FileDialog::getSaveFileName(app,
                                            "Save As",
                                            PROJECT_FILE_PLUGIN_FILTER_PRJ);

            if (fileName.empty())
            {
                LOG_DEBUG(<< "Cancelled, the filename is empty.");
                return false;
            }

            writePath = fileName;
            LOG_DEBUG(<< "Set project path to <" << writePath << ">.");
        }
        else
        {
            writePath = app->editor().projectPath();
            LOG_DEBUG(<< "Set project path to <" << writePath << ">.");
        }
    }
    else
    {
        // Save As.
        writePath = path;
        LOG_DEBUG(<< "Set project path to <" << writePath << ">.");
    }

    // Write.
    try
    {
        app->editor().saveProject(writePath);
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        app->showError(e.what());
        return false;
    }

    LOG_DEBUG(<< "Finished saving the project to <" << writePath << ">.");

    return true; // Saved.
}

bool ProjectFileAction::closeProject(Application *app)
{
    LOG_DEBUG(<< "Start closing the project.");

    app->suspendThreads();

    // Save changes.
    if (app->editor().unsavedChanges())
    {
        LOG_DEBUG(<< "Project has unsaved changes.");

        MessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(MessageBox::Save | MessageBox::Discard |
                                  MessageBox::Cancel);
        msgBox.setDefaultButton(MessageBox::Save);
        int ret = msgBox.exec();
        bool canClose = true;

        switch (ret)
        {
            case MessageBox::Save:
                // Save was clicked.
                canClose = saveProject(app, "");
                break;
            case MessageBox::Discard:
                // Don't Save was clicked.
                break;
            case MessageBox::Cancel:
                // Cancel was clicked.
                canClose = false;
                break;
            default:
                // Should never be reached.
                break;
        }

        if (canClose == false)
        {
            LOG_DEBUG(<< "Cancelled, the project should not be closed yet.");
            return false;
        }
    }

    // Close.
    try
    {
        app->editor().close();
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        app->showError(e.what());
    }

    LOG_DEBUG(<< "Finished closing the project.");

    return true; // Closed.
}
