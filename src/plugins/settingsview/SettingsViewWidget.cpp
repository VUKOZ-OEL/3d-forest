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

/** @file SettingsViewWidget.cpp */

// Include 3D Forest.
#include <ColorSwitchWidget.hpp>
#include <MainWindow.hpp>
#include <SettingsViewWidget.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "SettingsViewWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/settingsview/", name))

SettingsViewWidget::SettingsViewWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Color.
    colorSwitchWidget_ = new ColorSwitchWidget;
    connect(colorSwitchWidget_,
            SIGNAL(colorChanged()),
            this,
            SLOT(slotSetColor()));

    // Fog.
    fogEnabledCheckBox_ = new QCheckBox;
    fogEnabledCheckBox_->setChecked(settings_.fogEnabled());
    fogEnabledCheckBox_->setToolTip(
        tr("Reduce intensity with increasing distance"));
    fogEnabledCheckBox_->setText(tr("Show Depth"));
    connect(fogEnabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetFogEnabled(int)));

    // Bounding Box.
    showSceneBoundingBoxEnabledCheckBox_ = new QCheckBox;
    showSceneBoundingBoxEnabledCheckBox_->setChecked(
        settings_.showSceneBoundingBoxEnabled());
    showSceneBoundingBoxEnabledCheckBox_->setText(
        tr("Show Scene Bounding Box"));
    connect(showSceneBoundingBoxEnabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetShowSceneBoundingBoxEnabled(int)));

    // Attributes.
    showAttributesEnabledCheckBox_ = new QCheckBox;
    showAttributesEnabledCheckBox_->setChecked(
        settings_.showAttributesEnabled());
    showAttributesEnabledCheckBox_->setText(tr("Show Tree Attributes"));
    connect(showAttributesEnabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetShowAttributesEnabled(int)));

    QVBoxLayout *optionsVBoxLayout = new QVBoxLayout;
    optionsVBoxLayout->addWidget(showAttributesEnabledCheckBox_);
    optionsVBoxLayout->addWidget(showSceneBoundingBoxEnabledCheckBox_);
    optionsVBoxLayout->addWidget(fogEnabledCheckBox_);

    QGroupBox *optionsGroupBox = new QGroupBox(tr("Options"));
    optionsGroupBox->setLayout(optionsVBoxLayout);

    // Color source.
    colorSourceComboBox_ = new QComboBox;
    for (size_t i = 0; i < settings_.colorSourceSize(); i++)
    {
        colorSourceComboBox_->addItem(settings_.colorSourceString(i));
    }
    for (size_t i = 0; i < settings_.colorSourceSize(); i++)
    {
        if (settings_.colorSourceEnabled(i))
        {
            colorSourceComboBox_->setCurrentText(
                settings_.colorSourceString(i));
            break;
        }
    }

    connect(colorSourceComboBox_,
            SIGNAL(activated(int)),
            this,
            SLOT(slotColorSourceChanged(int)));

    // Point size.
    pointSizeSlider_ = new QSlider;
    pointSizeSlider_->setMinimum(1);
    pointSizeSlider_->setMaximum(5);
    pointSizeSlider_->setSingleStep(1);
    pointSizeSlider_->setTickInterval(1);
    pointSizeSlider_->setTickPosition(QSlider::TicksAbove);
    pointSizeSlider_->setOrientation(Qt::Horizontal);
    connect(pointSizeSlider_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetPointSize(int)));

    // Layout.
    QGridLayout *groupBoxLayout = new QGridLayout;

    groupBoxLayout->addWidget(colorSwitchWidget_,
                              0,
                              0,
                              Qt::AlignHCenter | Qt::AlignVCenter);
    groupBoxLayout->addWidget(optionsGroupBox, 0, 1);

    groupBoxLayout->addWidget(new QLabel(tr("Color Mode:")), 1, 0);
    groupBoxLayout->addWidget(colorSourceComboBox_, 1, 1);

    groupBoxLayout->addWidget(new QLabel(tr("Point Size:")), 2, 0);
    groupBoxLayout->addWidget(pointSizeSlider_, 2, 1);

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

void SettingsViewWidget::slotUpdate(void *sender,
                                    const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input view settings.");

        setViewSettings(mainWindow_->editor().settings().view());
    }
}

void SettingsViewWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output view settings.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSettingsView(settings_);
    mainWindow_->update(this, {Editor::TYPE_SETTINGS});

    if (modifiers)
    {
        mainWindow_->updateModifiers();
    }
    else
    {
        mainWindow_->updateRender();
    }
}

void SettingsViewWidget::setViewSettings(const SettingsView &settings)
{
    LOG_DEBUG(<< "Set view settings.");

    block();

    settings_ = settings;

    // Foreground color.
    auto fgv = settings_.pointColor();
    QColor fg;
    fg.setRgbF(static_cast<float>(fgv[0]),
               static_cast<float>(fgv[1]),
               static_cast<float>(fgv[2]));
    colorSwitchWidget_->setForegroundColor(fg);

    // Background color.
    auto bgv = settings_.backgroundColor();
    QColor bg;
    bg.setRgbF(static_cast<float>(bgv[0]),
               static_cast<float>(bgv[1]),
               static_cast<float>(bgv[2]));
    colorSwitchWidget_->setBackgroundColor(bg);

    // Point size.
    pointSizeSlider_->setValue(static_cast<int>(settings_.pointSize()));

    unblock();
}

void SettingsViewWidget::slotColorSourceChanged(int index)
{
    LOG_DEBUG(<< "Set color source to <" << index << ">.");

    if (index < 0)
    {
        return;
    }
    size_t i = static_cast<size_t>(index);
    settings_.setColorSourceEnabledAll(false);
    settings_.setColorSourceEnabled(i, true);
    dataChanged(true);
}

void SettingsViewWidget::slotSetPointSize(int v)
{
    settings_.setPointSize(static_cast<double>(v));
    dataChanged();
}

void SettingsViewWidget::slotSetFogEnabled(int v)
{
    (void)v;
    settings_.setFogEnabled(fogEnabledCheckBox_->isChecked());
    dataChanged();
}

void SettingsViewWidget::slotSetShowSceneBoundingBoxEnabled(int v)
{
    (void)v;
    settings_.setShowSceneBoundingBoxEnabled(
        showSceneBoundingBoxEnabledCheckBox_->isChecked());
    dataChanged();
}

void SettingsViewWidget::slotSetShowAttributesEnabled(int v)
{
    (void)v;
    settings_.setShowAttributesEnabled(
        showAttributesEnabledCheckBox_->isChecked());
    dataChanged();
}

void SettingsViewWidget::slotSetColor()
{
    QColor fg = colorSwitchWidget_->foregroundColor();
    settings_.setPointColor({fg.redF(), fg.greenF(), fg.blueF()});

    QColor bg = colorSwitchWidget_->backgroundColor();
    settings_.setBackgroundColor({bg.redF(), bg.greenF(), bg.blueF()});

    dataChanged(true);
}

void SettingsViewWidget::block()
{
    (void)blockSignals(true);
}

void SettingsViewWidget::unblock()
{
    (void)blockSignals(false);
}
