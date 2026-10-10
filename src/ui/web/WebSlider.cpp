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

/** @file WebSlider.hpp */

// Include 3D Forest.
#include <Slider.hpp>
#include <WebSlider.hpp>

// Include std.
#include <stdexcept>

// Include local.
#define LOG_MODULE_NAME "WebSlider"
#include <Log.hpp>

WebSlider::WebSlider(WebApplication &app, Slider *slider)
    : WebWidget(app, slider, "Slider"),
      slider_(slider)
{
}

void WebSlider::bind()
{
    WebWidget::bind();
    const std::weak_ptr<WebWidget> weak = shared_from_this();
    slider_->valueUpdated.connect(
        [weak](int)
        {
            if (auto base = weak.lock(); base && base->alive())
                static_cast<WebSlider &>(*base).updateSettings();
        });
    slider_->settingsChanged.connect(
        [weak]
        {
            if (auto base = weak.lock(); base && base->alive())
                static_cast<WebSlider &>(*base).updateSettings();
        });
    updateSettings();
}
void WebSlider::sync(bool enabled, bool visible)
{
    WebWidget::sync(enabled, visible);
    if (alive())
        updateSettings();
}
void WebSlider::updateSettings()
{
    JsonCpp::Value props{JsonCpp::objectValue};
    props["minimum"] = slider_->minimum();
    props["maximum"] = slider_->maximum();
    props["value"] = slider_->value();
    props["singleStep"] = slider_->singleStep();
    props["tickInterval"] = slider_->tickInterval();
    props["tickPosition"] = static_cast<int>(slider_->tickPosition());
    props["orientation"] = slider_->orientation();
    patch(props);
}
void WebSlider::handleEvent(const JsonCpp::Value &event)
{
    const std::string name = event["event"].asString();
    if (name != "valueChanged" && name != "sliderReleased")
        throw std::invalid_argument("Unknown slider event");
    if (!event["value"].isInt())
        throw std::invalid_argument("Slider value must be a 32-bit integer");
    const int value = event["value"].asInt();
    if (value < slider_->minimum() || value > slider_->maximum())
        throw std::invalid_argument(
            "Slider value is outside its current range");

    // Qt's singleStep controls keyboard increments, not permissible values.
    // Do not reject a valid in-range value just because it is off a step grid.
    Slider *slider = slider_;
    const auto life = widgetLife_;
    slider->setValue(value, true);
    // A plugin callback may destroy the slider/panel during setValue().
    if (name == "sliderReleased" && !life.expired() &&
        !slider->signalsBlocked())
        slider->sliderReleased();
}
