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

/** @file QtDialog.cpp */

// Include 3D Forest.
#include <Dialog.hpp>
#include <QtApplication.hpp>
#include <QtDialog.hpp>

// Include Qt.
#include <QAbstractButton>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "QtDialog"
#include <Log.hpp>

QtDialog::QtDialog(Dialog *dialog, QtApplication *app, QWidget *parent)
    : QDialog(parent),
      dialog_(dialog),
      buttonBox_(new QDialogButtonBox(this))
{
    auto *mainLayout = new QVBoxLayout(this);

    if (dialog_->layout())
    {
        mainLayout->addLayout(app->createLayout(dialog_->layout()));
    }

    mainLayout->addWidget(buttonBox_);

    updateProperties();

    const QPointer<QtDialog> guard(this);

    dialog_->propertiesChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateProperties();
            }
        });

    // Common Dialog::done() already stores the result and emits finished.
    // Use the qualified base call to avoid entering our done() again.
    dialog_->finished.connect(
        [guard](int result)
        {
            if (guard)
            {
                guard->QDialog::done(result);
            }
        });

    QObject::connect(
        buttonBox_,
        &QDialogButtonBox::clicked,
        this,
        [this, guard](QAbstractButton *button)
        {
            const auto standardButton = buttonBox_->standardButton(button);
            const auto role = buttonBox_->buttonRole(button);

            dialog_->buttonClicked(
                static_cast<Dialog::StandardButton>(standardButton));

            // A callback may have destroyed or closed the dialog.
            if (!guard || !guard->isVisible())
            {
                return;
            }

            switch (role)
            {
                case QDialogButtonBox::AcceptRole:
                case QDialogButtonBox::YesRole:
                    guard->accept();
                    break;

                case QDialogButtonBox::RejectRole:
                case QDialogButtonBox::NoRole:
                case QDialogButtonBox::DestructiveRole:
                    guard->reject();
                    break;

                default:
                    // Apply, Reset, Help, etc. keep the dialog open.
                    break;
            }
        });
}

void QtDialog::updateProperties()
{
    setWindowTitle(QString::fromStdString(dialog_->windowTitle()));

    // Your common StandardButton values match QDialogButtonBox.
    const auto buttons = static_cast<QDialogButtonBox::StandardButtons>(
        dialog_->standardButtons());

    if (buttonBox_->standardButtons() != buttons)
    {
        buttonBox_->setStandardButtons(buttons);
    }

    buttonBox_->setVisible(dialog_->standardButtons() != Dialog::NoButton);

    // Clear the previous explicit default.
    for (QAbstractButton *button : buttonBox_->buttons())
    {
        if (auto *pushButton = qobject_cast<QPushButton *>(button))
        {
            pushButton->setDefault(false);
        }
    }

    const auto defaultButton =
        static_cast<QDialogButtonBox::StandardButton>(dialog_->defaultButton());

    if (QPushButton *button = buttonBox_->button(defaultButton))
    {
        button->setDefault(true);
    }

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

    setMaximumWidth(dialog_->maximumWidth() >= 0 ? dialog_->maximumWidth()
                                                 : QWIDGETSIZE_MAX);

    // Remove a previous fixed-height constraint before applying changes.
    setMinimumHeight(0);

    setMaximumHeight(dialog_->maximumHeight() >= 0 ? dialog_->maximumHeight()
                                                   : QWIDGETSIZE_MAX);

    if (dialog_->fixedHeight() >= 0)
    {
        setFixedHeight(dialog_->fixedHeight());
    }
}

void QtDialog::accept()
{
    // Allows ImportFileDialog::accept() to validate its input.
    dialog_->accept();
}

void QtDialog::reject()
{
    dialog_->reject();
}

void QtDialog::done(int result)
{
    dialog_->done(result);
}

void QtDialog::closeEvent(QCloseEvent *event)
{
    // Common close() performs closeEvent validation and then reject().
    // Its finished signal closes the Qt dialog if closure is accepted.
    event->ignore();
    dialog_->close();
}

void QtDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    dialog_->updateSize(event->size().width(), event->size().height());
}

QSize QtDialog::sizeHint() const
{
    const Size hint = dialog_->sizeHint();
    const QSize fallback = QDialog::sizeHint();

    return QSize(hint.width() > 0 ? hint.width() : fallback.width(),
                 hint.height() > 0 ? hint.height() : fallback.height());
}

QSize QtDialog::minimumSizeHint() const
{
    const Size hint = dialog_->minimumSizeHint();
    const QSize fallback = QDialog::minimumSizeHint();

    return QSize(hint.width() > 0 ? hint.width() : fallback.width(),
                 hint.height() > 0 ? hint.height() : fallback.height());
}
