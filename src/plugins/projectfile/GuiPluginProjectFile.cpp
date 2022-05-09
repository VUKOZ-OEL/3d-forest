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

/** @file GuiPluginProjectFile.cpp */

#include <Log.hpp>

#include <GuiPluginProjectFile.hpp>
#include <GuiWindowMain.hpp>

#include <QFileDialog>
#include <QMessageBox>

#define GUI_PLUGIN_PROJECT_FILTER_PRJ "3DForest Project (*.json)"
#define ICON(name) (QIcon(":/projectfile/" name GUI_ICON_THEME ".png"))

GuiPluginProjectFile::GuiPluginProjectFile(GuiWindowMain *window)
    : QObject(window),
      window_(window)
{
    window_->createAction(&actionProjectNew_,
                          "File",
                          tr("&New Project"),
                          tr("Create new project"),
                          ICON("create"),
                          this,
                          SLOT(slotProjectNew()),
                          true);

    window_->createAction(&actionProjectOpen_,
                          "File",
                          tr("&Open Project..."),
                          tr("Open new project"),
                          ICON("opened-folder"),
                          this,
                          SLOT(slotProjectOpen()),
                          true);

    window_->createAction(&actionProjectSave_,
                          "File",
                          tr("&Save Project"),
                          tr("Save project"),
                          ICON("save"),
                          this,
                          SLOT(slotProjectSave()),
                          true);

    window_->createAction(&actionProjectSaveAs_,
                          "File",
                          tr("Save Project &As..."),
                          tr("Save project as"),
                          ICON("save-as"),
                          this,
                          SLOT(slotProjectSaveAs()),
                          true);
}

void GuiPluginProjectFile::slotProjectNew()
{
    if (projectClose())
    {
        window_->updateEverything();
    }
}

void GuiPluginProjectFile::slotProjectOpen()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(window_,
                                            tr("Open Project"),
                                            "",
                                            tr(GUI_PLUGIN_PROJECT_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectOpen(fileName);
}

void GuiPluginProjectFile::slotProjectSave()
{
    (void)projectSave();
}

void GuiPluginProjectFile::slotProjectSaveAs()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(window_,
                                            tr("Save Project As"),
                                            "",
                                            tr(GUI_PLUGIN_PROJECT_FILTER_PRJ));

    if (fileName.isEmpty())
    {
        return;
    }

    (void)projectSave(fileName);
}

bool GuiPluginProjectFile::projectOpen(const QString &path)
{
    // Close the current project
    if (!projectClose())
    {
        return false;
    }

    // Open new project
    try
    {
        window_->editor().open(path.toStdString());
    }
    catch (std::exception &e)
    {
        window_->showError(e.what());
        return false;
    }

    window_->updateEverything();

    return true; // Opened
}

bool GuiPluginProjectFile::projectClose()
{
    window_->cancelThreads();

    // Save changes
    if (window_->editor().hasUnsavedChanges())
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
        window_->editor().close();
    }
    catch (std::exception &e)
    {
        window_->showError(e.what());
    }

    return true; // Closed
}

bool GuiPluginProjectFile::projectSave(const QString &path)
{
    std::string writePath;

    window_->cancelThreads();

    if (path.isEmpty())
    {
        // Save
        writePath = window_->editor().projectPath();
        if (writePath.empty())
        {
            // First time save
            QString fileName;

            fileName =
                QFileDialog::getSaveFileName(window_,
                                             tr("Save As"),
                                             "",
                                             tr(GUI_PLUGIN_PROJECT_FILTER_PRJ));

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
        window_->editor().save(writePath);
    }
    catch (std::exception &e)
    {
        window_->showError(e.what());
        return false;
    }

    return true; // Saved
}
