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

/** @file QtMessageBox.cpp */

// Include 3D Forest.
#include <MessageBox.hpp>
#include <QtMessageBox.hpp>

// Include Qt.
#include <QPointer>

// Include local.
#define LOG_MODULE_NAME "QtMessageBox"
#include <Log.hpp>

namespace
{
struct ButtonMapping
{
    MessageBox::StandardButton common;
    QMessageBox::StandardButton qt;
};

constexpr ButtonMapping buttonMappings[] = {
    {MessageBox::Ok, QMessageBox::Ok},
    {MessageBox::Save, QMessageBox::Save},
    {MessageBox::SaveAll, QMessageBox::SaveAll},
    {MessageBox::Open, QMessageBox::Open},
    {MessageBox::Yes, QMessageBox::Yes},
    {MessageBox::YesToAll, QMessageBox::YesToAll},
    {MessageBox::No, QMessageBox::No},
    {MessageBox::NoToAll, QMessageBox::NoToAll},
    {MessageBox::Abort, QMessageBox::Abort},
    {MessageBox::Retry, QMessageBox::Retry},
    {MessageBox::Ignore, QMessageBox::Ignore},
    {MessageBox::Close, QMessageBox::Close},
    {MessageBox::Cancel, QMessageBox::Cancel},
    {MessageBox::Discard, QMessageBox::Discard},
    {MessageBox::Help, QMessageBox::Help},
    {MessageBox::Apply, QMessageBox::Apply},
    {MessageBox::Reset, QMessageBox::Reset},
    {MessageBox::RestoreDefaults, QMessageBox::RestoreDefaults}};

QMessageBox::StandardButton toQtButton(int button)
{
    for (const auto &mapping : buttonMappings)
    {
        if (mapping.common == button)
        {
            return mapping.qt;
        }
    }

    return QMessageBox::NoButton;
}

MessageBox::StandardButton toCommonButton(int button)
{
    for (const auto &mapping : buttonMappings)
    {
        if (mapping.qt == button)
        {
            return mapping.common;
        }
    }

    return MessageBox::NoButton;
}
} // namespace

QtMessageBox::QtMessageBox(MessageBox *messageBox, QWidget *parent)
    : QMessageBox(parent),
      messageBox_(messageBox)
{
    updateProperties();

    const QPointer<QtMessageBox> guard(this);

    messageBox_->propertiesChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateProperties();
            }
        });

    // Common done(result) closes the Qt representation.
    messageBox_->finished.connect(
        [guard](int result)
        {
            if (!guard || guard->finishing_)
            {
                return;
            }

            guard->finishing_ = true;
            guard->done(toQtButton(result));

            if (guard)
            {
                guard->finishing_ = false;
            }
        });

    // Native completion updates and notifies the common object.
    QObject::connect(this,
                     &QDialog::finished,
                     this,
                     [this, guard](int qtResult)
                     {
                         if (finishing_)
                         {
                             return;
                         }

                         finishing_ = true;

                         const int result = toCommonButton(qtResult);
                         messageBox_->setResult(result);
                         messageBox_->finished(result);

                         if (guard)
                         {
                             guard->finishing_ = false;
                         }
                     });
}

void QtMessageBox::updateProperties()
{
    setWindowTitle(QString::fromStdString(messageBox_->windowTitle()));

    setText(QString::fromStdString(messageBox_->text()));

    setInformativeText(QString::fromStdString(messageBox_->informativeText()));

    QMessageBox::StandardButtons qtButtons = QMessageBox::NoButton;

    for (const auto &mapping : buttonMappings)
    {
        if (messageBox_->standardButtons() & mapping.common)
        {
            qtButtons |= mapping.qt;
        }
    }

    setStandardButtons(qtButtons);
    setDefaultButton(toQtButton(messageBox_->defaultButton()));
}

int QtMessageBox::exec()
{
    messageBox_->setResult(MessageBox::NoButton);

    QMessageBox::exec();

    return messageBox_->result();
}
