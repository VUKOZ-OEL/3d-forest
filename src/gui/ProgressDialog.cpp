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

/** @file ProgressDialog.cpp */

#include <MainWindow.hpp>
#include <ProgressActionInterface.hpp>
#include <ProgressDialog.hpp>

#include <QCoreApplication>
#include <QProgressBar>

#define MODULE_NAME "ProgressDialog"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MESSAGE(LOG_DEBUG, MODULE_NAME, msg)

bool ProgressDialog::run(MainWindow *mainWindow,
                         const char *title,
                         ProgressActionInterface *progressAction)
{
    LOG_DEBUG_LOCAL(<< "");
    ProgressDialog *progressDialog = new ProgressDialog(mainWindow, title);
    return progressDialog->run(progressAction);
}

ProgressDialog::ProgressDialog(MainWindow *mainWindow, const char *title)
    : QProgressDialog(mainWindow)
{
    LOG_DEBUG_LOCAL(<< "");

    // Create modal progress dialog with custom progress bar.
    // Custom progress bar allows to display percentage with fractional part.
    setWindowTitle(QObject::tr(title));
    setWindowModality(Qt::WindowModal);
    setCancelButtonText(QObject::tr("&Cancel"));
    setMinimumDuration(0);

    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setTextVisible(false);
    progressBar->setRange(0, 100);
    progressBar->setValue(progressBar->minimum());

    setBar(progressBar);
}

bool ProgressDialog::run(ProgressActionInterface *progressAction)
{
    LOG_DEBUG_LOCAL(<< "");

    char buffer[64];

    // Show the progress dialog.
    show();

    // Process all iteration steps.
    while (!progressAction->end())
    {
        // Update current progress.
        double processedPercent = progressAction->percent();

        std::snprintf(buffer,
                      sizeof(buffer),
                      "Overall progress: %6.2f %% complete",
                      processedPercent);

        setValue(static_cast<int>(processedPercent));
        setLabelText(buffer);

        // Keep processing qt events.
        QCoreApplication::processEvents();

        // Canceled?
        if (wasCanceled())
        {
            return false;
        }

        // Process the action for few milliseconds.
        progressAction->step();
    }

    // Finish the progress dialog.
    setValue(maximum());

    return true;
}
