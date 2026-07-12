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

/** @file UnitsSettingsWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <UnitsSettingsWidget.hpp>

// Include Qt.
#include <CheckBox.hpp>
#include <QDoubleSpinBox>
#include <GridLayout.hpp>
#include <Label.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "UnitsSettingsWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/UnitsSettingsResources/", name))

UnitsSettingsWidget::UnitsSettingsWidget(Application *app)
    : Widget(app),
      app_(app)
{
    // Widgets.
    ppmLasSpinBox_ = new QDoubleSpinBox;
    ppmLasSpinBox_->setRange(1, 1000000);
    ppmLasSpinBox_->setValue(1000);
    ppmLasSpinBox_->setSingleStep(1);
    ppmLasSpinBox_->setEnabled(false);
    connect(ppmLasSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotIntermediateLas(double)));

    ppmUserSpinBox_ = new QDoubleSpinBox;
    ppmUserSpinBox_->setRange(1, 1000000);
    ppmUserSpinBox_->setValue(1000);
    ppmUserSpinBox_->setSingleStep(1);
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    connect(ppmUserSpinBox_,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotIntermediateUser(double)));

    userDefinedCheckBox_ = new CheckBox;
    userDefinedCheckBox_->setChecked(settings_.userDefined);
    // userDefinedCheckBox_->setText(tr("Enabled"));
    connect(userDefinedCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotUserDefined(int)));

    Label *help = new Label;
    help->setToolTip(tr("The values are in points per meter.\n"
                        "Example: las scaling 0.01 is 100 points per meter, "
                        "two points have integer x coordinates 5 and 7 "
                        "which is x length 2 cm.\n"
                        "The user is able to override input file las scaling "
                        "to user defined value."));
    ThemeIcon helpIcon(":/gui/", "question");
    help->setPixmap(helpIcon.pixmap(Application::ICON_SIZE_TEXT));

    // Layout.
    GridLayout *groupBoxLayout = new GridLayout;

    groupBoxLayout->addWidget(new Label(tr("Las scaling:")), 0, 0);
    groupBoxLayout->addWidget(ppmLasSpinBox_, 0, 1);

    groupBoxLayout->addWidget(new Label(tr("User scaling:")), 1, 0);
    groupBoxLayout->addWidget(ppmUserSpinBox_, 1, 1);

    groupBoxLayout->addWidget(new Label(tr("User scaling enabled:")), 2, 0);
    groupBoxLayout->addWidget(userDefinedCheckBox_, 2, 1);

    groupBoxLayout->addWidget(help, 3, 0);

    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->addLayout(groupBoxLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect([this](void *sender, const std::set<Editor::Type> &target)
    {
        slotUpdate(sender, target);
    });

    slotUpdate(nullptr, std::set<Editor::Type>());
}

void UnitsSettingsWidget::slotUpdate(void *sender,
                                     const std::set<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input units settings.");

        setUnitsSettings(app_->editor().settings().unitsSettings());
    }
}

void UnitsSettingsWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output units settings <" << toString(settings_)
                     << ">.");

    app_->suspendThreads();
    app_->editor().setUnitsSettings(settings_);
    app_->emitUpdate(this, {Editor::TYPE_SETTINGS});
}

void UnitsSettingsWidget::setUnitsSettings(const UnitsSettings &settings)
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

void UnitsSettingsWidget::slotIntermediateLas(double v)
{
    LOG_DEBUG(<< "New pointsPerMeterLas value <" << toString(v) << ">.");

    settings_.pointsPerMeterLas[0] = v;
    settings_.pointsPerMeterLas[1] = v;
    settings_.pointsPerMeterLas[2] = v;
    dataChanged();
}

void UnitsSettingsWidget::slotIntermediateUser(double v)
{
    LOG_DEBUG(<< "New pointsPerMeterUser value <" << toString(v) << ">.");

    settings_.pointsPerMeterUser[0] = v;
    settings_.pointsPerMeterUser[1] = v;
    settings_.pointsPerMeterUser[2] = v;
    dataChanged();
}

void UnitsSettingsWidget::slotUserDefined(int v)
{
    (void)v;
    settings_.userDefined = userDefinedCheckBox_->isChecked();
    ppmUserSpinBox_->setEnabled(settings_.userDefined);
    dataChanged();
}

void UnitsSettingsWidget::block()
{
    (void)blockSignals(true);
}

void UnitsSettingsWidget::unblock()
{
    (void)blockSignals(false);
}
