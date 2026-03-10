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
#include <GuiUtil.hpp>
#include <ProjectFileAction.hpp>
#include <MainWindow.hpp>
#include <Editor.hpp>

// Include Qt.
#include <QFileDialog>
#include <QMessageBox>

// Include local.
#define LOG_MODULE_NAME "ProjectFileAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define PROJECT_FILE_PLUGIN_FILTER_PRJ "3DForest Project (*.json)"

bool ProjectFileAction::newProject(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start creating a new project.");

    // Close the current project.
    if (!closeProject(mainWindow))
    {
        LOG_DEBUG(<< "Cancelled, the current project can not be closed.");
        return false;
    }

    // Update.
    mainWindow->updateNewProject();
    mainWindow->slotRenderViewports();

    LOG_DEBUG(<< "Finished creating new project.");
    return true;
}

bool ProjectFileAction::openProject(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start opening a project.");

    QString fileName;

    fileName = QFileDialog::getOpenFileName(mainWindow,
                                            QObject::tr("Open Project"),
                                            "",
                                            QObject::tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        LOG_DEBUG(<< "Cancelled, the filename is empty.");
        return false;
    }

    bool result = openProject(mainWindow, fileName);

    LOG_DEBUG(<< "Finished opening project <" << fileName << ">.");
    return result;
}

bool ProjectFileAction::saveProject(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start saving the project.");

    bool result = saveProject(mainWindow, "");

    LOG_DEBUG(<< "Finished saving the project.");
    return result;
}

bool ProjectFileAction::saveAsProject(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start saving the project as.");

    QString fileName;

    fileName = QFileDialog::getSaveFileName(mainWindow,
                                            QObject::tr("Save Project As"),
                                            "",
                                            QObject::tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        LOG_DEBUG(<< "Cancelled, the filename is empty.");
        return false;
    }

    bool result = saveProject(mainWindow, fileName);

    LOG_DEBUG(<< "Finished saving the project as <" << fileName << ">.");
    return result;
}

bool ProjectFileAction::reloadProject(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start reloading the project.");

    mainWindow->suspendThreads();

    if (mainWindow->editor().unsavedChanges())
    {
        LOG_DEBUG(<< "Project has unsaved changes.");

        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Please save the changes first.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        (void)msgBox.exec();
        return false;
    }

    try
    {
        mainWindow->editor().reload();
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow->showError(e.what());
        return false;
    }

    // Update.
    mainWindow->updateNewProject();
    mainWindow->slotRenderViewports();

    LOG_DEBUG(<< "Finished reloading the project.");
    return true;
}

bool ProjectFileAction::openProject(MainWindow *mainWindow, const QString &path)
{
    LOG_DEBUG(<< "Start opening new project <" << path << ">.");

    // Close the current project.
    if (!closeProject(mainWindow))
    {
        LOG_DEBUG(<< "Cancelled, the current project can not be closed.");
        return false;
    }

    // Open new project.
    try
    {
        mainWindow->editor().open(path.toStdString());
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow->showError(e.what());
        return false;
    }

    // Update.
    mainWindow->updateNewProject();
    mainWindow->slotRenderViewports();

    LOG_DEBUG(<< "Finished opening new project.");

    return true; // Opened
}

bool ProjectFileAction::saveProject(MainWindow *mainWindow, const QString &path)
{
    LOG_DEBUG(<< "Start saving the project to path <" << path << ">.");

    std::string writePath;

    mainWindow->suspendThreads();

    if (path.isEmpty())
    {
        // Save.
        if (mainWindow->editor().projectPath().empty())
        {
            // First time save.
            QString fileName;

            fileName = QFileDialog::getSaveFileName(
                mainWindow,
                QObject::tr("Save As"),
                "",
                QObject::tr(PROJECT_FILE_PLUGIN_FILTER_PRJ));

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
            writePath = mainWindow->editor().projectPath();
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
        mainWindow->editor().saveProject(writePath);
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow->showError(e.what());
        return false;
    }

    LOG_DEBUG(<< "Finished saving the project to <" << writePath << ">.");

    return true; // Saved.
}

bool ProjectFileAction::closeProject(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start closing the project.");

    mainWindow->suspendThreads();

    // Save changes.
    if (mainWindow->editor().unsavedChanges())
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
                canClose = saveProject(mainWindow, "");
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
        mainWindow->editor().close();
    }
    catch (std::exception &e)
    {
        LOG_DEBUG(<< "Cancelled, show error <" << e.what() << ">.");
        mainWindow->showError(e.what());
    }

    LOG_DEBUG(<< "Finished closing the project.");

    return true; // Closed.
}
