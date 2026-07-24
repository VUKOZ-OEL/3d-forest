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
#include <Application.hpp>
#include <CheckBox.hpp>
#include <Color.hpp>
#include <ColorSwitch.hpp>
#include <ComboBox.hpp>
#include <GridLayout.hpp>
#include <GroupBox.hpp>
#include <Label.hpp>
#include <Slider.hpp>
#include <ThemeIcon.hpp>
#include <Ui.hpp>
#include <VBoxLayout.hpp>
#include <ViewSettingsWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewSettingsWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ViewSettingsResources/", name))

ViewSettingsWidget::ViewSettingsWidget(Application *app)
    : Widget(app),
      app_(app)
{
    LOG_DEBUG(<< "Start creating settings view widget.");

    // Point size.
    pointSizeSlider_ = new Slider;
    pointSizeSlider_->setMinimum(1);
    pointSizeSlider_->setMaximum(5);
    pointSizeSlider_->setSingleStep(1);
    pointSizeSlider_->setTickInterval(1);
    pointSizeSlider_->setTickPosition(Slider::TicksAbove);
    pointSizeSlider_->setOrientation(Ui::Horizontal);
    pointSizeSlider_->valueChanged.connect([this](int value)
                                           { slotSetPointSize(value); });

    // Color.
    colorSwitch_ = new ColorSwitch;
    colorSwitch_->colorChanged.connect([this]() { slotSetColor(); });

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
    colorSourceComboBox_ = new ComboBox;
    for (size_t i = 0; i < colorSourceString.size(); i++)
    {
        colorSourceComboBox_->addItem(colorSourceString[i]);
    }
    colorSourceComboBox_->setCurrentText(toString(settings_.colorSource()));

    colorSourceComboBox_->activated.connect([this](int value)
                                            { slotColorSourceChanged(value); });

    // Distance-based fading.
    distanceBasedFadingVisibleCheckBox_ = new CheckBox;
    distanceBasedFadingVisibleCheckBox_->setChecked(
        settings_.distanceBasedFadingVisible());
    distanceBasedFadingVisibleCheckBox_->setToolTip(
        tr("Reduce intensity with increasing distance"));
    distanceBasedFadingVisibleCheckBox_->setText(
        tr("Show distance-based fading"));
    distanceBasedFadingVisibleCheckBox_->stateChanged.connect(
        [this](int value) { slotSetDistanceBasedFadingVisible(value); });

    // Bounding box.
    sceneBoundingBoxVisibleCheckBox_ = new CheckBox;
    sceneBoundingBoxVisibleCheckBox_->setChecked(
        settings_.sceneBoundingBoxVisible());
    sceneBoundingBoxVisibleCheckBox_->setText(tr("Show scene bounding box"));
    sceneBoundingBoxVisibleCheckBox_->stateChanged.connect(
        [this](int value) { slotSetSceneBoundingBoxVisible(value); });

    // Options.
    VBoxLayout *optionsVBoxLayout = new VBoxLayout;
    optionsVBoxLayout->addWidget(distanceBasedFadingVisibleCheckBox_);
    optionsVBoxLayout->addWidget(sceneBoundingBoxVisibleCheckBox_);

    GroupBox *optionsGroupBox = new GroupBox(tr("Options"));
    optionsGroupBox->setLayout(optionsVBoxLayout);

    // Layout.
    GridLayout *groupBoxLayout = new GridLayout;

    groupBoxLayout->addWidget(colorSwitch_,
                              0,
                              0,
                              Ui::AlignHCenter | Ui::AlignVCenter);
    groupBoxLayout->addWidget(optionsGroupBox, 0, 1);

    groupBoxLayout->addWidget(new Label(tr("Color mode:")), 1, 0);
    groupBoxLayout->addWidget(colorSourceComboBox_, 1, 1);

    groupBoxLayout->addWidget(new Label(tr("Point size:")), 2, 0);
    groupBoxLayout->addWidget(pointSizeSlider_, 2, 1);

    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->addLayout(groupBoxLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect([this](const Message &msg) { slotUpdate(msg); });

    slotUpdate({});

    LOG_DEBUG(<< "Finished creating settings view widget.");
}

void ViewSettingsWidget::slotUpdate(const Message &msg)
{
    if (msg.sender() == this)
    {
        return;
    }

    if (msg.empty() || msg.contains(Message::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input view settings.");

        setViewSettings(app_->editor().settings().viewSettings());
    }
}

void ViewSettingsWidget::dataChanged(bool modifiers)
{
    LOG_DEBUG_UPDATE(<< "Output view settings.");

    app_->suspendThreads();
    app_->editor().setViewSettings(settings_);
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

void ViewSettingsWidget::setViewSettings(const ViewSettings &settings)
{
    LOG_DEBUG(<< "Set view settings.");

    block();

    settings_ = settings;

    // Foreground color.
    auto fgv = settings_.pointColor();
    Color fg;
    fg.setRgbF(static_cast<float>(fgv[0]),
               static_cast<float>(fgv[1]),
               static_cast<float>(fgv[2]));
    colorSwitch_->setForegroundColor(fg);

    // Background color.
    auto bgv = settings_.backgroundColor();
    Color bg;
    bg.setRgbF(static_cast<float>(bgv[0]),
               static_cast<float>(bgv[1]),
               static_cast<float>(bgv[2]));
    colorSwitch_->setBackgroundColor(bg);

    // Point size.
    pointSizeSlider_->setValue(static_cast<int>(settings_.pointSize()));

    // Color source.
    colorSourceComboBox_->setCurrentText(toString(settings_.colorSource()));

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
    Color fg = colorSwitch_->foregroundColor();
    settings_.setPointColor({fg.redF(), fg.greenF(), fg.blueF()});

    Color bg = colorSwitch_->backgroundColor();
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
    fromString(colorSource, colorSourceComboBox_->itemText(index));
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
