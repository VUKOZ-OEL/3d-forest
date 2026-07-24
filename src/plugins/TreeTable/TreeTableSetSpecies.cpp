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
#include <Action.hpp>
#include <Application.hpp>
#include <InputComboBoxDialog.hpp>
#include <Menu.hpp>
#include <ProgressDialog.hpp>
#include <TreeTableSetSpecies.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeTableSetSpecies"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

TreeTableSetSpecies::TreeTableSetSpecies(Application *app, Menu *contextMenu)
    : app_(app),
      contextMenu_(contextMenu),
      menu_(nullptr)
{
    create();
}

void TreeTableSetSpecies::create()
{
    menu_ = new Menu("Set Species", contextMenu_);

    // Set dialog items.
    Editor *editor = &app_->editor();
    const SpeciesList &speciesList = editor->speciesList();
    for (size_t i = 0; i < speciesList.size(); i++)
    {
        const Species &species = speciesList[i];
        std::string text = toString(species.id) + " : " + species.latin;

        Action *action = menu_->addAction(text);
        actions_[action] = species.id;
    }

    // Add to the parent menu.
    contextMenu_->addMenu(menu_);
}

void TreeTableSetSpecies::runAction(Action *selectedAction,
                                    std::unordered_set<size_t> idList)
{
    auto it = actions_.find(selectedAction);
    if (it == actions_.end())
    {
        return;
    }

    LOG_DEBUG(<< "Start setting species values.");

    size_t newSpeciesId = it->second;
    Editor *editor = &app_->editor();
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
    app_->update(this, Message::TYPE_SEGMENT | Message::TYPE_SPECIES);

    LOG_DEBUG(<< "Finished setting species values.");
}

void TreeTableSetSpecies::run(Application *app,
                              std::unordered_set<size_t> idList)
{
    LOG_DEBUG(<< "Start setting species values.");

    size_t newSpeciesId = 0;

    // Editor.
    app->suspendThreads();

    Editor *editor = &app->editor();

    // Input.
    InputComboBoxDialog dialog(app);
    dialog.setWindowTitle("Select a species");

    // Set dialog items.
    const SpeciesList &speciesList = editor->speciesList();
    for (size_t i = 0; i < speciesList.size(); i++)
    {
        const Species &species = speciesList[i];
        std::string text = toString(species.id) + " : " + species.latin;
        dialog.addItem(text);
    }

    // Open the dialog.
    if (dialog.exec() == Dialog::Accepted)
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
    app->update(nullptr, Message::TYPE_SEGMENT | Message::TYPE_SPECIES);

    LOG_DEBUG(<< "Finished setting species values.");
}
