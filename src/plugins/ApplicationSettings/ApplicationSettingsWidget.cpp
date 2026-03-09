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

/** @file ApplicationSettingsWidget.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <ApplicationSettingsWidget.hpp>

// Include Qt.
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ApplicationSettingsWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ApplicationSettingsResources/", name))

ApplicationSettingsWidget::ApplicationSettingsWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating settings application widget.");

    // Language.
    languageComboBox_ = new QComboBox;
    languageComboBox_->addItem("en");
    languageComboBox_->addItem("cs");
    QString languageCode = QString::fromStdString(settings_.languageCode);
    languageComboBox_->setCurrentText(languageCode);

    connect(languageComboBox_,
            SIGNAL(activated(int)),
            this,
            SLOT(slotLanguageChanged(int)));

    // Layout.
    QGridLayout *groupBoxLayout = new QGridLayout;

    groupBoxLayout->addWidget(new QLabel(tr("Language:")), 1, 0);
    groupBoxLayout->addWidget(languageComboBox_, 1, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(groupBoxLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());

    LOG_DEBUG(<< "Finished creating settings application widget.");
}

void ApplicationSettingsWidget::slotUpdate(void *sender,
                                    const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input application settings.");

        setApplicationSettings(mainWindow_->editor().settings().applicationSettings());
    }
}

void ApplicationSettingsWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output application settings.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setApplicationSettings(settings_);
    mainWindow_->emitUpdate(this, {Editor::TYPE_SETTINGS});

    if (modifiers)
    {
        mainWindow_->updateModifiers();
    }
    else
    {
        mainWindow_->updateRender();
    }
}

void ApplicationSettingsWidget::setApplicationSettings(const ApplicationSettings &settings)
{
    LOG_DEBUG(<< "Set application settings.");

    block();

    settings_ = settings;

    // Language.
    QString languageCode = QString::fromStdString(settings_.languageCode);
    languageComboBox_->setCurrentText(languageCode);

    unblock();
}

void ApplicationSettingsWidget::slotLanguageChanged(int index)
{
    LOG_DEBUG(<< "Set color source to index <" << index << ">.");

    if (index < 0)
    {
        return;
    }

    settings_.languageCode = languageComboBox_->itemText(index).toStdString();

    dataChanged(true);
}

void ApplicationSettingsWidget::block()
{
    (void)blockSignals(true);
}

void ApplicationSettingsWidget::unblock()
{
    (void)blockSignals(false);
}
