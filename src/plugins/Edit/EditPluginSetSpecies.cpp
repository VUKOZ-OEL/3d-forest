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

/** @file EditPluginSetSpecies.cpp */

// Include 3D Forest.
#include <EditPluginSetSpecies.hpp>
#include <InputComboBoxDialog.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QProgressDialog>

// Include local.
#define LOG_MODULE_NAME "EditPluginSetSpecies"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void EditPluginSetSpecies::run(MainWindow *mainWindow)
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
                       QString::fromStdString(species.label);
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

    // Run.
    QProgressDialog progress("Processing...", "Cancel", 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    Query query(editor);
    query.setWhere(editor->viewports().where());
    query.exec();

    int bulk = 1000;
    int counter = 0;
    while (query.next())
    {
        size_t segmentId = query.segment();
        size_t segmentIndex = segments.index(segmentId, false);

        if (segmentIndex != SIZE_MAX)
        {
            segments[segmentIndex].speciesId = newSpeciesId;
        }

        counter++;
        if (counter == bulk)
        {
            counter = 0;

            QCoreApplication::processEvents();

            if (progress.wasCanceled())
            {
                break;
            }
        }
    }

    progress.close();

    editor->setSegments(segments);
    mainWindow->update({Editor::TYPE_SEGMENT, Editor::TYPE_SPECIES});

    LOG_DEBUG(<< "Finished setting species values.");
}
