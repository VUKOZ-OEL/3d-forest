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
#include <GuiUtil.hpp>
#include <MainWindow.hpp>
#include <ProjectFilePlugin.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QFileDialog>
#include <QMessageBox>

// Include local.
#define LOG_MODULE_NAME "ProjectFilePlugin"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define PROJECT_FILE_PLUGIN_FILTER_PRJ "3DForest Project (*.json)"
#define ICON(name) (ThemeIcon(":/projectfile/", name))

ProjectFilePlugin::ProjectFilePlugin() : mainWindow_(nullptr)
{
}

void ProjectFilePlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&actionProjectNew_,
                              "File",
                              "File Project",
                              tr("&New Project"),
                              tr("Create new project"),
                              ICON("create"),
                              this,
                              SLOT(slotProjectNew()));

    mainWindow_->createAction(&actionProjectOpen_,
                              "File",
                              "File Project",
                              tr("&Open Project..."),
                              tr("Open new project"),
                              ICON("opened_folder"),
                              this,
                              SLOT(slotProjectOpen()));

    mainWindow_->createAction(&actionProjectSave_,
                              "File",
                              "File Project",
                              tr("&Save Project"),
                              tr("Save project"),
                              ICON("save"),
                              this,
                              SLOT(slotProjectSave()));

    mainWindow_->createAction(&actionProjectSaveAs_,
                              "File",
                              "File Project",
                              tr("Save Project &As..."),
                              tr("Save project as"),
                              ICON("save_as"),
                              this,
                              SLOT(slotProjectSaveAs()));

    mainWindow_->hideToolBar("File Project");
}

void ProjectFilePlugin::slotProjectNew()
{
    LOG_DEBUG(<< "Start creating a new project.");

    // Close the current project.
    if (!projectClose())
    {
        LOG_DEBUG(<< "Cancelled, the current project can not be closed.");
        return;
    }

    // Update.
    mainWindow_->updateNewProject();

    LOG_DEBUG(<< "Finished creating new project.");
}

void ProjectFilePlugin::slotProjectOpen()
{
    LOG_DEBUG(<< "Start opening a new project.");

    QString fileName;

    fileName = QFileDialog::getOpenFileName(mainWindow_,
                                            tr("Open Project"),
                                            "",
                                            tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        LOG_DEBUG(<< "Cancelled, the filename is empty.");
        return;
    }

    (void)projectOpen(fileName);

    LOG_DEBUG(<< "Finished opening new project <" << fileName << ">.");
}

void ProjectFilePlugin::slotProjectSave()
{
    LOG_DEBUG(<< "Start saving the project.");

    (void)projectSave();

    LOG_DEBUG(<< "Finished saving the project.");
}

void ProjectFilePlugin::slotProjectSaveAs()
{
    LOG_DEBUG(<< "Start saving the project as.");

    QString fileName;

    fileName = QFileDialog::getSaveFileName(mainWindow_,
                                            tr("Save Project As"),
                                            "",
                                            tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        LOG_DEBUG(<< "Cancelled, the filename is empty.");
        return;
    }

    (void)projectSave(fileName);

    LOG_DEBUG(<< "Finished saving the project as <" << fileName << ">.");
}

bool ProjectFilePlugin::projectOpen(const QString &path)
{
    LOG_DEBUG(<< "Start opening new project <" << path << ">.");

    // Close the current project.
    if (!projectClose())
    {
        LOG_DEBUG(<< "Cancelled, the current project can not be closed.");
        return false;
    }

    // Open new project.
    try
    {
        mainWindow_->editor().open(path.toStdString());
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow_->showError(e.what());
        return false;
    }

    // Update.
    mainWindow_->updateNewProject();
    mainWindow_->slotRenderViewports();

    LOG_DEBUG(<< "Finished opening new project.");

    return true; // Opened
}

bool ProjectFilePlugin::projectClose()
{
    LOG_DEBUG(<< "Start closing the project.");

    mainWindow_->suspendThreads();

    // Save changes.
    if (mainWindow_->editor().hasUnsavedChanges())
    {
        LOG_DEBUG(<< "Project has unsaved changes.");

        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                                  QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        bool canClose = true;

        switch (ret)
        {
            case QMessageBox::Save:
                // Save was clicked.
                canClose = projectSave();
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked.
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked.
                canClose = false;
                break;
            default:
                // Should never be reached.
                Q_UNREACHABLE();
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
        mainWindow_->editor().close();
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow_->showError(e.what());
    }

    LOG_DEBUG(<< "Finished closing the project.");

    return true; // Closed.
}

bool ProjectFilePlugin::projectSave(const QString &path)
{
    LOG_DEBUG(<< "Start saving the project to path <" << path << ">.");

    std::string writePath;

    mainWindow_->suspendThreads();

    if (path.isEmpty())
    {
        // Save.
        if (mainWindow_->editor().projectPath().empty())
        {
            // First time save.
            QString fileName;

            fileName = QFileDialog::getSaveFileName(
                mainWindow_,
                tr("Save As"),
                "",
                tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

            if (fileName.isEmpty())
            {
                LOG_DEBUG(<< "Cancelled, the filename is empty.");
                return false;
            }

            writePath = fileName.toStdString();
            LOG_DEBUG(<< "Set project path to <" << writePath << ">.");
        }
        else
        {
            writePath = mainWindow_->editor().projectPath();
            LOG_DEBUG(<< "Set project path to <" << writePath << ">.");
        }
    }
    else
    {
        // Save As.
        writePath = path.toStdString();
        LOG_DEBUG(<< "Set project path to <" << writePath << ">.");
    }

    // Write.
    try
    {
        mainWindow_->editor().saveProject(writePath);
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow_->showError(e.what());
        return false;
    }

    LOG_DEBUG(<< "Finished saving the project to <" << writePath << ">.");

    return true; // Saved.
}
