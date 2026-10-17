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

/** @file WebWidget.cpp */

// Include 3D Forest.
#include <GroupBox.hpp>
#include <Label.hpp>
#include <WebApplication.hpp>
#include <WebWidget.hpp>

// Include std.
#include <stdexcept>

// Include local.
#define LOG_MODULE_NAME "WebWidget"
#include <Log.hpp>

WebWidget::WebWidget(WebApplication &app,
                     Widget *widget,
                     const std::string &type)
    : application_(app),
      widget_(widget),
      widgetLife_(widget->lifetime())
{
    id_ = application_.registerWidget(widget,
                                      type,
                                      JsonCpp::Value{JsonCpp::objectValue});
}
WebWidget::~WebWidget()
{
    // unregisterWidget uses pointer identity only; the common object may be
    // gone.
    application_.unregisterWidget(widget_);
}
void WebWidget::bind()
{
    const std::weak_ptr<WebWidget> weak = shared_from_this();
    application_.setWidgetEventHandler(widget_,
                                       [weak](const JsonCpp::Value &event)
                                       {
                                           auto self = weak.lock();
                                           if (!self || !self->alive())
                                               throw std::invalid_argument(
                                                   "Widget has been removed");
                                           self->handleEvent(event);
                                       });
}
void WebWidget::patch(const JsonCpp::Value &properties)
{
    if (alive())
        application_.setWidgetProperties(widget_, properties);
}
void WebWidget::sync(bool enabled, bool visible)
{
    if (!alive())
        return;
    JsonCpp::Value props{JsonCpp::objectValue};
    props["enabled"] = enabled;
    props["visible"] = visible;
    props["name"] = widget_->name();
    props["toolTip"] = widget_->toolTip();
    if (auto *label = dynamic_cast<Label *>(widget_))
        props["text"] = label->text();
    if (auto *group = dynamic_cast<GroupBox *>(widget_))
        props["title"] = group->title();
    patch(props);
}
void WebWidget::handleEvent(const JsonCpp::Value &)
{
    throw std::invalid_argument("This widget has no web event adapter yet");
}
