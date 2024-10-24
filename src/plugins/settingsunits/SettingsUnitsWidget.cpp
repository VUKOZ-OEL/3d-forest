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
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/SettingsUnitsResources/", name))

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
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotIntermediateLas(double)));

    ppmUserSpinBox_ = new QDoubleSpinBox;
    ppmUserSpinBox_->setRange(1, 10000);
    ppmUserSpinBox_->setValue(1000);
    ppmUserSpinBox_->setSingleStep(1);
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    connect(ppmUserSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotIntermediateUser(double)));

    userDefinedCheckBox_ = new QCheckBox;
    userDefinedCheckBox_->setChecked(settings_.userDefined);
    // userDefinedCheckBox_->setText(tr("Enabled"));
    connect(userDefinedCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotUserDefined(int)));

    QLabel *help = new QLabel;
    help->setToolTip(tr("The values are in points per meter.\n"
                        "Example: las scaling 0.01 is 100 points per meter, "
                        "two points have integer x coordinates 5 and 7 "
                        "which is x length 2 cm.\n"
                        "The user is able to override input file las scaling "
                        "to user defined value."));
    ThemeIcon helpIcon(":/gui/", "question");
    help->setPixmap(helpIcon.pixmap(MainWindow::ICON_SIZE_TEXT));

    // Layout.
    QGridLayout *groupBoxLayout = new QGridLayout;

    groupBoxLayout->addWidget(new QLabel(tr("Las scaling:")), 0, 0);
    groupBoxLayout->addWidget(ppmLasSpinBox_, 0, 1);

    groupBoxLayout->addWidget(new QLabel(tr("User scaling:")), 1, 0);
    groupBoxLayout->addWidget(ppmUserSpinBox_, 1, 1);

    groupBoxLayout->addWidget(new QLabel(tr("User scaling enabled:")), 2, 0);
    groupBoxLayout->addWidget(userDefinedCheckBox_, 2, 1);

    groupBoxLayout->addWidget(help, 3, 0);

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
        LOG_DEBUG_UPDATE(<< "Input units settings.");

        setUnitsSettings(mainWindow_->editor().settings().units());
    }
}

void SettingsUnitsWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output units settings <" << toString(settings_)
                     << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSettingsUnits(settings_);
    mainWindow_->update(this, {Editor::TYPE_SETTINGS});
}

void SettingsUnitsWidget::setUnitsSettings(const SettingsUnits &settings)
{
    LOG_DEBUG(<< "Set units settings <" << toString(settings) << ">.");

    block();

    settings_ = settings;

    ppmLasSpinBox_->setValue(settings_.pointsPerMeterLas[0]);
    ppmUserSpinBox_->setValue(settings_.pointsPerMeterUser[0]);
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    userDefinedCheckBox_->setChecked(settings_.userDefined);

    unblock();
}

void SettingsUnitsWidget::slotIntermediateLas(double v)
{
    LOG_DEBUG(<< "New pointsPerMeterLas value <" << toString(v) << ">.");

    settings_.pointsPerMeterLas[0] = v;
    settings_.pointsPerMeterLas[1] = v;
    settings_.pointsPerMeterLas[2] = v;
    dataChanged();
}

void SettingsUnitsWidget::slotIntermediateUser(double v)
{
    LOG_DEBUG(<< "New pointsPerMeterUser value <" << toString(v) << ">.");

    settings_.pointsPerMeterUser[0] = v;
    settings_.pointsPerMeterUser[1] = v;
    settings_.pointsPerMeterUser[2] = v;
    dataChanged();
}

void SettingsUnitsWidget::slotUserDefined(int v)
{
    (void)v;
    settings_.userDefined = userDefinedCheckBox_->isChecked();
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    dataChanged();
}

void SettingsUnitsWidget::block()
{
    (void)blockSignals(true);
}

void SettingsUnitsWidget::unblock()
{
    (void)blockSignals(false);
}
