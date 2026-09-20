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

/** @file QtProgressDialog.cpp */

// Include 3D Forest.
#include <QtProgressDialog.hpp>

// Include Qt.
#include <QPointer>

// Include local.
#define LOG_MODULE_NAME "QtProgressDialog"
#include <Log.hpp>

QtProgressDialog::QtProgressDialog(ProgressDialog *dialog, QWidget *parent)
    : QProgressDialog(parent),
      dialog_(dialog)
{
    setAutoClose(false);
    setAutoReset(false);
    setMinimumDuration(0);

    updateProperties();

    setRange(dialog_->minimum(), dialog_->maximum());
    setLabelText(QString::fromStdString(dialog_->labelText()));
    setCancelButtonText(QString::fromStdString(dialog_->cancelButtonText()));

    const QPointer<QtProgressDialog> guard(this);

    dialog_->propertiesChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateProperties();
            }
        });

    dialog_->rangeChanged.connect(
        [guard](int minimum, int maximum)
        {
            if (guard)
            {
                guard->setRange(minimum, maximum);
            }
        });

    dialog_->valueChanged.connect(
        [guard](int value)
        {
            if (guard)
            {
                guard->setValue(value);
            }
        });

    dialog_->labelTextChanged.connect(
        [guard](const std::string &text)
        {
            if (guard)
            {
                guard->setLabelText(QString::fromStdString(text));
            }
        });

    dialog_->cancelButtonTextChanged.connect(
        [guard](const std::string &text)
        {
            if (guard)
            {
                guard->setCancelButtonText(QString::fromStdString(text));
            }
        });

    dialog_->resetRequested.connect(
        [guard]
        {
            if (guard)
            {
                guard->reset();
            }
        });

    // Programmatic common cancellation updates Qt's cancellation state.
    dialog_->canceled.connect(
        [guard]
        {
            if (guard)
            {
                guard->cancel();
            }
        });

    // Common close()/accept()/reject()/done() closes the Qt dialog.
    dialog_->finished.connect(
        [guard](int result)
        {
            if (guard)
            {
                guard->QDialog::done(result);
            }
        });

    // The Qt Cancel button or window close requests common cancellation.
    QObject::connect(this,
                     &QProgressDialog::canceled,
                     this,
                     [this] { dialog_->cancel(); });

    // Set the initial value after cancellation connections are installed.
    if (dialog_->value() >= dialog_->minimum())
    {
        setValue(dialog_->value());
    }
}

QtProgressDialog::~QtProgressDialog()
{
}

void QtProgressDialog::updateProperties()
{
    setWindowTitle(QString::fromStdString(dialog_->windowTitle()));

    switch (dialog_->windowModality())
    {
        case Dialog::WindowModal:
            setWindowModality(Qt::WindowModal);
            break;

        case Dialog::ApplicationModal:
            setWindowModality(Qt::ApplicationModal);
            break;

        default:
            setWindowModality(Qt::NonModal);
            break;
    }
}
