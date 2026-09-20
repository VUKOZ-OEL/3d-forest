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

/** @file QtTextEdit.cpp */

// Include 3D Forest.
#include <QtTextEdit.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtTextEdit"
#include <Log.hpp>

QtTextEdit::QtTextEdit(TextEdit *textEdit, QWidget *parent)
    : QTextEdit(parent),
      textEdit_(textEdit)
{
    // setAcceptRichText(false);
    // setPlainText(QString::fromStdString(textEdit_->text()));

    setAcceptRichText(true);
    setHtml(QString::fromStdString(textEdit_->text()));
    setReadOnly(textEdit_->isReadOnly());

    const QPointer<QtTextEdit> guard(this);

    // Common text -> Qt document.
    textEdit_->textUpdated.connect(
        [guard](const std::string &text)
        {
            if (!guard || guard->updatingCommon_)
            {
                return;
            }

            const QSignalBlocker blocker(guard.data());
            guard->setHtml(QString::fromStdString(text));
        });

    textEdit_->readOnlyChanged.connect(
        [guard](bool readOnly)
        {
            if (guard)
            {
                guard->setReadOnly(readOnly);
            }
        });

    // Qt document -> common text.
    QObject::connect(this,
                     &QTextEdit::textChanged,
                     this,
                     [this, guard]
                     {
                         updatingCommon_ = true;

                         try
                         {
                             textEdit_->setText(toHtml().toStdString(), true);
                         }
                         catch (...)
                         {
                             if (guard)
                             {
                                 guard->updatingCommon_ = false;
                             }

                             throw;
                         }

                         if (guard)
                         {
                             guard->updatingCommon_ = false;
                         }
                     });
}

QtTextEdit::~QtTextEdit()
{
}
