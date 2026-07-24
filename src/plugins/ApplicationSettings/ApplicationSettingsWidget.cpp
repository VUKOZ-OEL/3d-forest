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
#include <Application.hpp>
#include <ApplicationSettingsWidget.hpp>
#include <ComboBox.hpp>
#include <GridLayout.hpp>
#include <Label.hpp>
#include <ThemeIcon.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "ApplicationSettingsWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ApplicationSettingsResources/", name))

ApplicationSettingsWidget::ApplicationSettingsWidget(Application *app)
    : Widget(app),
      app_(app)
{
    LOG_DEBUG(<< "Start creating settings application widget.");

    // Language.
    languageComboBox_ = new ComboBox;
    languageComboBox_->addItem("en");
    languageComboBox_->addItem("cs");
    languageComboBox_->setCurrentText(settings_.languageCode);

    languageComboBox_->activated.connect([this](int value)
                                         { slotLanguageChanged(value); });

    // Layout.
    GridLayout *groupBoxLayout = new GridLayout;

    groupBoxLayout->addWidget(new Label(tr("Language:")), 1, 0);
    groupBoxLayout->addWidget(languageComboBox_, 1, 1);

    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->addLayout(groupBoxLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect([this](const Message &msg) { slotUpdate(msg); });

    slotUpdate({});

    LOG_DEBUG(<< "Finished creating settings application widget.");
}

void ApplicationSettingsWidget::slotUpdate(const Message &msg)
{
    if (msg.sender() == this)
    {
        return;
    }

    if (msg.empty() || msg.contains(Message::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input application settings.");

        setApplicationSettings(app_->editor().settings().applicationSettings());
    }
}

void ApplicationSettingsWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output application settings.");

    app_->suspendThreads();
    app_->editor().setApplicationSettings(settings_);
    app_->emitUpdate(this, {Message::TYPE_SETTINGS});

    if (modifiers)
    {
        app_->updateModifiers();
    }
    else
    {
        app_->updateRender();
    }
}

void ApplicationSettingsWidget::setApplicationSettings(
    const ApplicationSettings &settings)
{
    LOG_DEBUG(<< "Set application settings.");

    settings_ = settings;

    // Language.
    languageComboBox_->setCurrentText(settings_.languageCode, false);
}

void ApplicationSettingsWidget::slotLanguageChanged(int index)
{
    LOG_DEBUG(<< "Set color source to index <" << index << ">.");

    if (index < 0)
    {
        return;
    }

    settings_.languageCode = languageComboBox_->itemText(index);

    dataChanged(true);
}
