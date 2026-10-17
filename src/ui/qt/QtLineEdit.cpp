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

/** @file QtLineEdit.cpp */

// Include 3D Forest.
#include <QtLineEdit.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtLineEdit"
#include <Log.hpp>

QtLineEdit::QtLineEdit(LineEdit *lineEdit, QWidget *parent)
    : QLineEdit(parent),
      lineEdit_(lineEdit)
{
    setText(QString::fromStdString(lineEdit_->text()));
    setPlaceholderText(QString::fromStdString(lineEdit_->placeholderText()));
    setReadOnly(lineEdit_->isReadOnly());

    const QPointer<QtLineEdit> guard(this);

    // Common -> Qt.
    lineEdit_->textUpdated.connect(
        [guard](const std::string &text)
        {
            if (!guard)
            {
                return;
            }

            const QString qtText = QString::fromStdString(text);

            // Preserve cursor position and undo history during user edits.
            if (guard->text() == qtText)
            {
                return;
            }

            const QSignalBlocker blocker(guard.data());
            guard->setText(qtText);
        });

    lineEdit_->placeholderTextChanged.connect(
        [guard](const std::string &text)
        {
            if (guard)
            {
                guard->setPlaceholderText(QString::fromStdString(text));
            }
        });

    lineEdit_->readOnlyChanged.connect(
        [guard](bool readOnly)
        {
            if (guard)
            {
                guard->setReadOnly(readOnly);
            }
        });

    // Qt user edits -> common.
    QObject::connect(this,
                     &QLineEdit::textEdited,
                     this,
                     [this](const QString &text)
                     { lineEdit_->setText(text.toStdString(), true); });

    QObject::connect(this,
                     &QLineEdit::returnPressed,
                     this,
                     [this]
                     {
                         if (!lineEdit_->signalsBlocked())
                         {
                             lineEdit_->returnPressed();
                         }
                     });

    QObject::connect(this,
                     &QLineEdit::editingFinished,
                     this,
                     [this]
                     {
                         if (!lineEdit_->signalsBlocked())
                         {
                             lineEdit_->editingFinished();
                         }
                     });
}

QtLineEdit::~QtLineEdit()
{
}
