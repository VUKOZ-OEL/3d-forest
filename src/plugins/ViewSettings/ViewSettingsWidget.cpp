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

/** @file ViewSettingsWidget.cpp */

// Include 3D Forest.
#include <ColorSwitchWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <ViewSettingsWidget.hpp>

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
#define LOG_MODULE_NAME "ViewSettingsWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ViewSettingsResources/", name))

ViewSettingsWidget::ViewSettingsWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating settings view widget.");

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

    // Color.
    colorSwitchWidget_ = new ColorSwitchWidget;
    connect(colorSwitchWidget_,
            SIGNAL(colorChanged()),
            this,
            SLOT(slotSetColor()));

    // Color source.
    std::vector<std::string> colorSourceString = {"Color",
                                                  "Intensity",
                                                  "Return Number",
                                                  "Number of Returns",
                                                  "Classification",
                                                  "Tree",
                                                  "Species",
                                                  "Management Status",
                                                  "Elevation",
                                                  "Descriptor"};
    colorSourceComboBox_ = new QComboBox;
    for (size_t i = 0; i < colorSourceString.size(); i++)
    {
        colorSourceComboBox_->addItem(colorSourceString[i].c_str());
    }
    colorSourceComboBox_->setCurrentText(
        toString(settings_.colorSource()).c_str());

    connect(colorSourceComboBox_,
            SIGNAL(activated(int)),
            this,
            SLOT(slotColorSourceChanged(int)));

    // Distance-based fading.
    distanceBasedFadingVisibleCheckBox_ = new QCheckBox;
    distanceBasedFadingVisibleCheckBox_->setChecked(
        settings_.distanceBasedFadingVisible());
    distanceBasedFadingVisibleCheckBox_->setToolTip(
        tr("Reduce intensity with increasing distance"));
    distanceBasedFadingVisibleCheckBox_->setText(
        tr("Show distance-based fading"));
    connect(distanceBasedFadingVisibleCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetDistanceBasedFadingVisible(int)));

    // Bounding box.
    sceneBoundingBoxVisibleCheckBox_ = new QCheckBox;
    sceneBoundingBoxVisibleCheckBox_->setChecked(
        settings_.sceneBoundingBoxVisible());
    sceneBoundingBoxVisibleCheckBox_->setText(tr("Show scene bounding box"));
    connect(sceneBoundingBoxVisibleCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotSetSceneBoundingBoxVisible(int)));

    // Options.
    QVBoxLayout *optionsVBoxLayout = new QVBoxLayout;
    optionsVBoxLayout->addWidget(distanceBasedFadingVisibleCheckBox_);
    optionsVBoxLayout->addWidget(sceneBoundingBoxVisibleCheckBox_);

    QGroupBox *optionsGroupBox = new QGroupBox(tr("Options"));
    optionsGroupBox->setLayout(optionsVBoxLayout);

    // Layout.
    QGridLayout *groupBoxLayout = new QGridLayout;

    groupBoxLayout->addWidget(colorSwitchWidget_,
                              0,
                              0,
                              Qt::AlignHCenter | Qt::AlignVCenter);
    groupBoxLayout->addWidget(optionsGroupBox, 0, 1);

    groupBoxLayout->addWidget(new QLabel(tr("Color mode:")), 1, 0);
    groupBoxLayout->addWidget(colorSourceComboBox_, 1, 1);

    groupBoxLayout->addWidget(new QLabel(tr("Point size:")), 2, 0);
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

    LOG_DEBUG(<< "Finished creating settings view widget.");
}

void ViewSettingsWidget::slotUpdate(void *sender,
                                    const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input view settings.");

        setViewSettings(mainWindow_->editor().settings().viewSettings());
    }
}

void ViewSettingsWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output view settings.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setViewSettings(settings_);
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

void ViewSettingsWidget::setViewSettings(const ViewSettings &settings)
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

    // Color source.
    colorSourceComboBox_->setCurrentText(
        toString(settings_.colorSource()).c_str());

    // Distance-based fading.
    distanceBasedFadingVisibleCheckBox_->setChecked(
        settings_.distanceBasedFadingVisible());

    // Bounding box.
    sceneBoundingBoxVisibleCheckBox_->setChecked(
        settings_.sceneBoundingBoxVisible());

    unblock();
}

void ViewSettingsWidget::slotSetPointSize(int v)
{
    settings_.setPointSize(static_cast<double>(v));
    dataChanged();
}

void ViewSettingsWidget::slotSetColor()
{
    QColor fg = colorSwitchWidget_->foregroundColor();
    settings_.setPointColor({fg.redF(), fg.greenF(), fg.blueF()});

    QColor bg = colorSwitchWidget_->backgroundColor();
    settings_.setBackgroundColor({bg.redF(), bg.greenF(), bg.blueF()});

    dataChanged(true);
}

void ViewSettingsWidget::slotColorSourceChanged(int index)
{
    LOG_DEBUG(<< "Set color source to index <" << index << ">.");

    if (index < 0)
    {
        return;
    }

    ViewSettings::ColorSource colorSource;
    fromString(colorSource,
               colorSourceComboBox_->itemText(index).toStdString());
    settings_.setColorSource(colorSource);
    dataChanged(true);
}

void ViewSettingsWidget::slotSetDistanceBasedFadingVisible(int v)
{
    (void)v;
    settings_.setDistanceBasedFadingVisible(
        distanceBasedFadingVisibleCheckBox_->isChecked());
    dataChanged();
}

void ViewSettingsWidget::slotSetSceneBoundingBoxVisible(int v)
{
    (void)v;
    settings_.setSceneBoundingBoxVisible(
        sceneBoundingBoxVisibleCheckBox_->isChecked());
    dataChanged();
}

void ViewSettingsWidget::block()
{
    (void)blockSignals(true);
}

void ViewSettingsWidget::unblock()
{
    (void)blockSignals(false);
}
