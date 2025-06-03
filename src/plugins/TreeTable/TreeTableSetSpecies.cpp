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

/** @file TreeTableSetSpecies.cpp */

// Include 3D Forest.
#include <InputComboBoxDialog.hpp>
#include <MainWindow.hpp>
#include <TreeTableSetSpecies.hpp>

// Include Qt.
#include <QAction>
#include <QCoreApplication>
#include <QMenu>
#include <QProgressDialog>

// Include local.
#define LOG_MODULE_NAME "TreeTableSetSpecies"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

TreeTableSetSpecies::TreeTableSetSpecies(MainWindow *mainWindow,
                                         QMenu *contextMenu)
    : mainWindow_(mainWindow),
      contextMenu_(contextMenu),
      menu_(nullptr)
{
    create();
}

void TreeTableSetSpecies::create()
{
    menu_ = new QMenu("Set Species", contextMenu_);

    // Set dialog items.
    Editor *editor = &mainWindow_->editor();
    const SpeciesList &speciesList = editor->speciesList();
    for (size_t i = 0; i < speciesList.size(); i++)
    {
        const Species &species = speciesList[i];
        QString text = QString::number(species.id) + " : " +
                       QString::fromStdString(species.latin);

        QAction *action = menu_->addAction(text);
        actions_[action] = species.id;
    }

    // Add to the parent menu.
    contextMenu_->addMenu(menu_);
}

void TreeTableSetSpecies::runAction(QAction *selectedAction,
                                    std::unordered_set<size_t> idList)
{
    auto it = actions_.find(selectedAction);
    if (it == actions_.end())
    {
        return;
    }

    LOG_DEBUG(<< "Start setting species values.");

    size_t newSpeciesId = it->second;
    Editor *editor = &mainWindow_->editor();
    Segments segments = editor->segments();
    for (const auto &id : idList)
    {
        size_t index = segments.index(id, false);

        if (index != SIZE_MAX)
        {
            segments[index].speciesId = newSpeciesId;
        }
    }

    editor->setSegments(segments);
    mainWindow_->update({Editor::TYPE_SEGMENT, Editor::TYPE_SPECIES});

    LOG_DEBUG(<< "Finished setting species values.");
}

void TreeTableSetSpecies::run(MainWindow *mainWindow,
                              std::unordered_set<size_t> idList)
{
    LOG_DEBUG(<< "Start setting species values.");

    size_t newSpeciesId = 0;

    // Editor.
    mainWindow->suspendThreads();

    Editor *editor = &mainWindow->editor();

    // Input.
    InputComboBoxDialog dialog;
    dialog.setWindowTitle("Select a species");

    // Set dialog items.
    const SpeciesList &speciesList = editor->speciesList();
    for (size_t i = 0; i < speciesList.size(); i++)
    {
        const Species &species = speciesList[i];
        QString text = QString::number(species.id) + " : " +
                       QString::fromStdString(species.latin);
        dialog.addItem(text);
    }

    // Open the dialog.
    if (dialog.exec() == QDialog::Accepted)
    {
        // Run.
        size_t index = static_cast<size_t>(dialog.currentIndex());
        newSpeciesId = speciesList[index].id;
        LOG_DEBUG(<< "User entered index <" << dialog.currentIndex() << "> id <"
                  << newSpeciesId << ">.");
    }
    else
    {
        // Cancel.
        LOG_DEBUG(<< "Canceled setting species values.");
        return;
    }

    // Edit segments.
    Segments segments = editor->segments();
    for (const auto &id : idList)
    {
        size_t index = segments.index(id, false);

        if (index != SIZE_MAX)
        {
            segments[index].speciesId = newSpeciesId;
        }
    }

    editor->setSegments(segments);
    mainWindow->update({Editor::TYPE_SEGMENT, Editor::TYPE_SPECIES});

    LOG_DEBUG(<< "Finished setting species values.");
}
