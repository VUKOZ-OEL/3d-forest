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

#include <Log.hpp>

#include <IconTheme.hpp>
#include <MainWindow.hpp>
#include <ProjectFilePlugin.hpp>

#include <QFileDialog>
#include <QMessageBox>

#define PROJECT_FILE_PLUGIN_FILTER_PRJ "3DForest Project (*.json)"
#define ICON(name) (IconTheme(":/projectfile/", name))

ProjectFilePlugin::ProjectFilePlugin(MainWindow *mainWindow)
    : QObject(mainWindow),
      mainWindow_(mainWindow)
{
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
    if (projectClose())
    {
        mainWindow_->updateEverything();
    }
}

void ProjectFilePlugin::slotProjectOpen()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(mainWindow_,
                                            tr("Open Project"),
                                            "",
                                            tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectOpen(fileName);
}

void ProjectFilePlugin::slotProjectSave()
{
    (void)projectSave();
}

void ProjectFilePlugin::slotProjectSaveAs()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(mainWindow_,
                                            tr("Save Project As"),
                                            "",
                                            tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectSave(fileName);
}

bool ProjectFilePlugin::projectOpen(const QString &path)
{
    // Close the current project
    if (!projectClose())
    {
        return false;
    }

    // Open new project
    try
    {
        mainWindow_->editor().open(path.toStdString());
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
        return false;
    }

    mainWindow_->updateEverything();

    return true; // Opened
}

bool ProjectFilePlugin::projectClose()
{
    mainWindow_->suspendThreads();

    // Save changes
    if (mainWindow_->editor().hasUnsavedChanges())
    {
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
                // Save was clicked
                canClose = projectSave();
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                canClose = false;
                break;
            default:
                // should never be reached
                Q_UNREACHABLE();
                break;
        }

        if (canClose == false)
        {
            return false;
        }
    }

    // Close
    try
    {
        mainWindow_->editor().close();
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }

    return true; // Closed
}

bool ProjectFilePlugin::projectSave(const QString &path)
{
    std::string writePath;

    mainWindow_->suspendThreads();

    if (path.isEmpty())
    {
        // Save
        writePath = mainWindow_->editor().projectPath();
        if (writePath.empty())
        {
            // First time save
            QString fileName;

            fileName = QFileDialog::getSaveFileName(
                mainWindow_,
                tr("Save As"),
                "",
                tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

            if (fileName.isEmpty())
            {
                return false;
            }

            writePath = fileName.toStdString();
        }
    }
    else
    {
        // Save As
        writePath = path.toStdString();
    }

    // Write
    try
    {
        mainWindow_->editor().save(writePath);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
        return false;
    }

    return true; // Saved
}
