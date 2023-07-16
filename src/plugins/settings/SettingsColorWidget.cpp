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

/** @file SettingsColorWidget.cpp */

#include <ColorSwitchWidget.hpp>
#include <MainWindow.hpp>
#include <SettingsColorWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "SettingsColorWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/settings/", name))

SettingsColorWidget::SettingsColorWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Color
    colorSwitchWidget_ = new ColorSwitchWidget;
    connect(colorSwitchWidget_,
            SIGNAL(colorChanged()),
            this,
            SLOT(slotSetColor()));

    // Fog
    fogCheckBox_ = new QCheckBox;
    fogCheckBox_->setChecked(settings_.isFogEnabled());
    fogCheckBox_->setToolTip(tr("Reduce intensity with increasing distance"));
    fogCheckBox_->setText(tr("Show Depth"));
    connect(fogCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetFogEnabled(int)));

    QVBoxLayout *optionsVBoxLayout = new QVBoxLayout;
    optionsVBoxLayout->addWidget(fogCheckBox_);

    QGroupBox *optionsGroupBox = new QGroupBox(tr("Options"));
    optionsGroupBox->setLayout(optionsVBoxLayout);

    // Color source
    colorSourceComboBox_ = new QComboBox;
    for (size_t i = 0; i < settings_.colorSourceSize(); i++)
    {
        colorSourceComboBox_->addItem(settings_.colorSourceString(i));
    }
    for (size_t i = 0; i < settings_.colorSourceSize(); i++)
    {
        if (settings_.isColorSourceEnabled(i))
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

    // Point size
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

    // Layout
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

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());
}

void SettingsColorWidget::slotUpdate(void *sender,
                                     const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SETTINGS))
    {
        setSettingsIn(mainWindow_->editor().settings().view());
    }
}

void SettingsColorWidget::setSettingsOut(bool modifiers)
{
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

void SettingsColorWidget::slotColorSourceChanged(int index)
{
    LOG_DEBUG(<< "Set color source to <" << index << ">.");
    if (index < 0)
    {
        return;
    }
    size_t i = static_cast<size_t>(index);
    settings_.setColorSourceEnabledAll(false);
    settings_.setColorSourceEnabled(i, true);
    setSettingsOut(true);
}

void SettingsColorWidget::slotSetPointSize(int v)
{
    settings_.setPointSize(static_cast<double>(v));
    setSettingsOut();
}

void SettingsColorWidget::slotSetFogEnabled(int v)
{
    (void)v;
    settings_.setFogEnabled(fogCheckBox_->isChecked());
    setSettingsOut();
}

void SettingsColorWidget::slotSetColor()
{
    QColor fg = colorSwitchWidget_->foregroundColor();
    settings_.setPointColor({fg.redF(), fg.greenF(), fg.blueF()});

    QColor bg = colorSwitchWidget_->backgroundColor();
    settings_.setBackgroundColor({bg.redF(), bg.greenF(), bg.blueF()});

    setSettingsOut(true);
}

void SettingsColorWidget::setSettingsIn(const SettingsView &settings)
{
    block();

    settings_ = settings;

    // Foreground color
    auto fgv = settings_.pointColor();
    QColor fg;
    fg.setRgbF(static_cast<float>(fgv[0]),
               static_cast<float>(fgv[1]),
               static_cast<float>(fgv[2]));
    colorSwitchWidget_->setForegroundColor(fg);

    // Background color
    auto bgv = settings_.backgroundColor();
    QColor bg;
    bg.setRgbF(static_cast<float>(bgv[0]),
               static_cast<float>(bgv[1]),
               static_cast<float>(bgv[2]));
    colorSwitchWidget_->setBackgroundColor(bg);

    // Point size
    pointSizeSlider_->setValue(static_cast<int>(settings_.pointSize()));

    unblock();
}

void SettingsColorWidget::block()
{
    (void)blockSignals(true);
}

void SettingsColorWidget::unblock()
{
    (void)blockSignals(false);
}
