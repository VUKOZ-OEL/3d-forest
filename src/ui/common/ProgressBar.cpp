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

/** @file ProgressBar.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <ProgressBar.hpp>

// Include local.
#define LOG_MODULE_NAME "ProgressBar"
#include <Log.hpp>

ProgressBar::ProgressBar()
{
#if 0
    // Create modal progress dialog with custom progress bar.
    // Custom progress bar allows to display percentage with fractional part.
    QProgressDialog progressDialog(app);
    progressDialog.setWindowTitle(QObject::tr("Create Index"));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setMinimumDuration(0);

    QProgressBar *progressBar = new QProgressBar(&progressDialog);
    progressBar->setTextVisible(false);
    progressBar->setRange(0, 100);
    progressBar->setValue(progressBar->minimum());
    progressDialog.setBar(progressBar);
#endif
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::setRange(int min, int max)
{
    min_ = min;
    max_ = max;
}

void ProgressBar::setValue(int value)
{
}

void ProgressBar::setLabelText(const std::string &str)
{
}
