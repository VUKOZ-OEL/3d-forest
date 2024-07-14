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

/** @file SettingsUnitsWidget.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <SettingsUnitsWidget.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "SettingsUnitsWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/settings/", name))

SettingsUnitsWidget::SettingsUnitsWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Widgets.
    ppmLasSpinBox_ = new QDoubleSpinBox;
    ppmLasSpinBox_->setRange(1, 10000);
    ppmLasSpinBox_->setValue(1000);
    ppmLasSpinBox_->setSingleStep(1);
    ppmLasSpinBox_->setEnabled(false);
    connect(ppmLasSpinBox_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotIntermediateLas(int)));

    ppmUserSpinBox_ = new QDoubleSpinBox;
    ppmUserSpinBox_->setRange(1, 10000);
    ppmUserSpinBox_->setValue(1000);
    ppmUserSpinBox_->setSingleStep(1);
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    connect(ppmUserSpinBox_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotIntermediateUser(int)));

    userDefinedCheckBox_ = new QCheckBox;
    userDefinedCheckBox_->setChecked(settings_.userDefined);
    userDefinedCheckBox_->setText(tr("Enabled"));
    connect(userDefinedCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotUserDefined(int)));

    // Layout.
    QGridLayout *groupBoxLayout = new QGridLayout;

    groupBoxLayout->addWidget(new QLabel(tr("Las scaling:")), 0, 0);
    groupBoxLayout->addWidget(ppmLasSpinBox_, 0, 1);

    groupBoxLayout->addWidget(new QLabel(tr("User scaling:")), 1, 0);
    groupBoxLayout->addWidget(ppmUserSpinBox_, 1, 1);

    groupBoxLayout->addWidget(userDefinedCheckBox_, 2, 1);

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
}

void SettingsUnitsWidget::slotUpdate(void *sender,
                                     const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SETTINGS))
    {
        setSettingsIn(mainWindow_->editor().settings().units);
    }
}

void SettingsUnitsWidget::setSettingsOut()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setSettingsUnits(settings_);
    mainWindow_->update(this, {Editor::TYPE_SETTINGS});
}

void SettingsUnitsWidget::slotIntermediateLas(double v)
{
    settings_.pointsPerMeterLas[0] = v;
    settings_.pointsPerMeterLas[1] = v;
    settings_.pointsPerMeterLas[2] = v;
    setSettingsOut();
}

void SettingsUnitsWidget::slotIntermediateUser(double v)
{
    settings_.pointsPerMeterUser[0] = v;
    settings_.pointsPerMeterUser[1] = v;
    settings_.pointsPerMeterUser[2] = v;
    setSettingsOut();
}

void SettingsUnitsWidget::slotUserDefined(int v)
{
    (void)v;
    settings_.userDefined = userDefinedCheckBox_->isChecked();
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    setSettingsOut();
}

void SettingsUnitsWidget::setSettingsIn(const SettingsUnits &settings)
{
    block();

    settings_ = settings;

    ppmLasSpinBox_->setValue(settings_.pointsPerMeterLas[0]);
    ppmUserSpinBox_->setValue(settings_.pointsPerMeterUser[0]);
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    userDefinedCheckBox_->setChecked(settings_.userDefined);

    unblock();
}

void SettingsUnitsWidget::block()
{
    (void)blockSignals(true);
}

void SettingsUnitsWidget::unblock()
{
    (void)blockSignals(false);
}
