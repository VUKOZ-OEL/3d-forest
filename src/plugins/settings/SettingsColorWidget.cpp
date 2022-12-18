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

#include <MainWindow.hpp>
#include <SettingsColorWidget.hpp>
#include <ThemeIcon.hpp>

#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/settings/", name))

SettingsColorWidget::SettingsColorWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Fog
    fogCheckBox_ = new QCheckBox;
    fogCheckBox_->setChecked(settings_.isFogEnabled());
    fogCheckBox_->setToolTip(tr("Reduce intensity with increasing distance"));
    fogCheckBox_->setText(tr("Fog"));
    connect(fogCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetFogEnabled(int)));

    // Color
    colorFgButton_ = new QPushButton(tr("Foreground"));
    connect(colorFgButton_, SIGNAL(clicked()), this, SLOT(slotSetColorFg()));

    colorBgButton_ = new QPushButton(tr("Background"));
    connect(colorBgButton_, SIGNAL(clicked()), this, SLOT(slotSetColorBg()));

    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget(colorFgButton_);
    colorLayout->addWidget(colorBgButton_);
    colorLayout->addWidget(fogCheckBox_);
    colorLayout->addStretch();

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

    QHBoxLayout *pointSizeLayout = new QHBoxLayout;
    pointSizeLayout->addWidget(new QLabel(tr("Point Size")));
    pointSizeLayout->addWidget(pointSizeSlider_);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(colorLayout);
    mainLayout->addLayout(pointSizeLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(const QSet<Editor::Type> &)));
}

void SettingsColorWidget::slotUpdate(const QSet<Editor::Type> &target)
{
    if (!target.empty() && !target.contains(Editor::TYPE_SETTINGS))
    {
        return;
    }

    setSettings(mainWindow_->editor().settings().view());
}

void SettingsColorWidget::settingsChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setSettingsView(settings_);
    mainWindow_->updateRender();
}

void SettingsColorWidget::settingsChangedApply()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setSettingsView(settings_);
    mainWindow_->updateModifiers();
}

void SettingsColorWidget::slotSetPointSize(int v)
{
    settings_.setPointSize(static_cast<float>(v));
    settingsChanged();
}

void SettingsColorWidget::slotSetFogEnabled(int v)
{
    (void)v;
    settings_.setFogEnabled(fogCheckBox_->isChecked());
    settingsChanged();
}

void SettingsColorWidget::slotSetColorFg()
{
    Vector3<float> rgb = settings_.pointColor();

    if (colorDialog(rgb))
    {
        settings_.setPointColor(rgb);
        setColor(colorFgButton_, rgb);
        settingsChangedApply();
    }
}

void SettingsColorWidget::slotSetColorBg()
{
    Vector3<float> rgb = settings_.backgroundColor();

    if (colorDialog(rgb))
    {
        settings_.setBackgroundColor(rgb);
        setColor(colorBgButton_, rgb);
        settingsChangedApply();
    }
}

bool SettingsColorWidget::colorDialog(Vector3<float> &rgb)
{
    QColor color;
    color.setRgbF(rgb[0], rgb[1], rgb[2]);

    QColorDialog dialog(color, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return false;
    }

    color = dialog.selectedColor();
    rgb[0] = static_cast<float>(color.redF());
    rgb[1] = static_cast<float>(color.greenF());
    rgb[2] = static_cast<float>(color.blueF());

    return true;
}

void SettingsColorWidget::setColor(QPushButton *button,
                                   const Vector3<float> &rgb)
{
    QColor color;
    color.setRgbF(rgb[0], rgb[1], rgb[2]);

    QPixmap pixmap(24, 24);
    pixmap.fill(color);

    QIcon icon(pixmap);

    button->setIcon(icon);
    button->setIconSize(QSize(10, 10));
}

void SettingsColorWidget::setSettings(const SettingsView &settings)
{
    block();

    settings_ = settings;

    setColor(colorFgButton_, settings_.pointColor());
    setColor(colorBgButton_, settings_.backgroundColor());
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
