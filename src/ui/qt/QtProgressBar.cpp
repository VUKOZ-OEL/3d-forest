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

/** @file QtProgressBar.cpp */

// Include 3D Forest.
#include <QtProgressBar.hpp>

// Include Qt.
#include <QPointer>

// Include local.
#define LOG_MODULE_NAME "QtProgressBar"
#include <Log.hpp>

QtProgressBar::QtProgressBar(ProgressBar *progressBar, QWidget *parent)
    : QProgressBar(parent),
      progressBar_(progressBar)
{
    setRange(progressBar_->minimum(), progressBar_->maximum());
    setTextVisible(progressBar_->isTextVisible());

    if (progressBar_->value() < progressBar_->minimum())
    {
        reset();
    }
    else
    {
        setValue(progressBar_->value());
    }

    const QPointer<QtProgressBar> guard(this);

    progressBar_->rangeChanged.connect(
        [guard](int minimum, int maximum)
        {
            if (guard)
            {
                guard->setRange(minimum, maximum);
            }
        });

    progressBar_->valueChanged.connect(
        [guard](int value)
        {
            if (guard)
            {
                guard->setValue(value);
            }
        });

    progressBar_->resetRequested.connect(
        [guard]
        {
            if (guard)
            {
                guard->reset();
            }
        });

    progressBar_->textVisibleChanged.connect(
        [guard](bool visible)
        {
            if (guard)
            {
                guard->setTextVisible(visible);
            }
        });
}

QtProgressBar::~QtProgressBar()
{
}
