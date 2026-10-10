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

/** @file WebApplication.cpp */

// Include 3D Forest.
#include <Pixmap.hpp>
#include <WebApplication.hpp>
#include <WebSession.hpp>
#include <WebUiTree.hpp>

// Include std.
#include <stdexcept>
#include <utility>

// Include local.
#define LOG_MODULE_NAME "WebApplication"
#include <Log.hpp>

namespace
{
[[noreturn]] void unsupported(const char *operation)
{
    throw std::logic_error(std::string(operation) +
                           " is not implemented by the minimal web backend");
}
} // namespace

WebApplication::WebApplication(WebSession &session)
    : session_(session),
      uiTree_(std::make_unique<WebUiTree>(*this))
{
}
WebApplication::~WebApplication()
{
    uiTree_.reset();
    lifetime_.reset();
}

void WebApplication::assertThread() const
{
    if (!session_.isCurrentThread())
        throw std::logic_error(
            "WebApplication accessed outside its session thread");
}

void WebApplication::setInitializeHandler(Handler handler)
{
    assertThread();
    if (initialized_)
        throw std::logic_error("Application is already initialized");
    initializeHandler_ = std::move(handler);
}

void WebApplication::setWakeUpHandler(Handler handler)
{
    assertThread();
    wakeUpHandler_ = std::move(handler);
}

void WebApplication::init()
{
    assertThread();
    if (initialized_)
        throw std::logic_error("init() called twice");
    initialized_ = true;
    if (initializeHandler_)
        initializeHandler_(*this);
    else
        load();
    uiTree_->sync();
}

void WebApplication::wakeUp()
{
    session_.wakeUp();
}
void WebApplication::handleWakeUp()
{
    Application::processEvents();
    processRenderRequest();
    if (wakeUpHandler_)
        wakeUpHandler_(*this);
}
void WebApplication::processEvents()
{
    assertThread();
    flush();
}

void WebApplication::setMetadata(const char *name, const std::string &value)
{
    assertThread();
    if (metadata_[name] == value)
        return;
    metadata_[name] = value;
    structureDirty_ = true;
}
void WebApplication::setOrganizationName(const std::string &v)
{
    setMetadata("organization", v);
}
void WebApplication::setApplicationName(const std::string &v)
{
    setMetadata("name", v);
}
void WebApplication::setApplicationVersion(const std::string &v)
{
    setMetadata("version", v);
}
void WebApplication::setWindowIcon(const std::string &v)
{
    setMetadata("icon", v);
}

std::string WebApplication::registerWidget(Widget *widget,
                                           const std::string &type,
                                           JsonCpp::Value properties,
                                           EventHandler handler)
{
    assertThread();
    if (!widget || type.empty() || !properties.isObject())
        throw std::invalid_argument("Invalid widget registration");
    if (ids_.count(widget))
        throw std::logic_error("Widget is already registered");
    const auto id = "w" + std::to_string(++nextId_);
    widgets_.emplace(
        id,
        Entry{widget, type, std::move(properties), std::move(handler)});
    ids_.emplace(widget, id);
    structureDirty_ = true;
    return id;
}

std::string WebApplication::widgetId(Widget *widget) const
{
    assertThread();
    if (!widget)
        return {};
    auto it = ids_.find(widget);
    if (it == ids_.end())
        throw std::logic_error(
            "Register the widget with the web backend first");
    return it->second;
}

void WebApplication::unregisterWidget(Widget *widget)
{
    assertThread();
    auto it = ids_.find(widget);
    if (it == ids_.end())
        return;
    pending_.removeMember(it->second);
    widgets_.erase(it->second);
    ids_.erase(it);
    structureDirty_ = true;
}

void WebApplication::setWidgetProperties(Widget *widget,
                                         const JsonCpp::Value &properties)
{
    assertThread();
    if (!properties.isObject())
        throw std::invalid_argument("Properties must be an object");
    const auto id = widgetId(widget);
    auto &entry = widgets_.at(id);
    for (const auto &key : properties.getMemberNames())
    {
        if (entry.properties.isMember(key) &&
            entry.properties[key] == properties[key])
            continue;
        entry.properties[key] = properties[key];
        pending_[id][key] = properties[key];
    }
}

void WebApplication::setWidgetEventHandler(Widget *widget, EventHandler handler)
{
    assertThread();
    widgets_.at(widgetId(widget)).handler = std::move(handler);
}

void WebApplication::setViewer(Widget *widget)
{
    assertThread();
    uiTree_->setViewer(widget);
}
void WebApplication::removeViewer(Widget *widget)
{
    assertThread();
    uiTree_->removeViewer(widget);
}
void WebApplication::showBottomWidget(Widget *widget)
{
    assertThread();
    uiTree_->showBottomWidget(widget);
}
void WebApplication::hideBottomWidget()
{
    assertThread();
    uiTree_->hideBottomWidget();
}
void WebApplication::toggleBottomWidget(Widget *widget)
{
    assertThread();
    uiTree_->toggleBottomWidget(widget);
}
void WebApplication::removeBottomWidget(Widget *widget)
{
    assertThread();
    uiTree_->removeBottomWidget(widget);
}

JsonCpp::Value WebApplication::snapshot() const
{
    JsonCpp::Value result{JsonCpp::objectValue};
    result["type"] = "snapshot";
    result["application"] = metadata_;
    result["ui"] = uiTree_->description();
    result["widgets"] = JsonCpp::Value{JsonCpp::arrayValue};
    for (const auto &[id, entry] : widgets_)
    {
        JsonCpp::Value item{JsonCpp::objectValue};
        item["id"] = id;
        item["type"] = entry.type;
        item["properties"] = entry.properties;
        item["region"] = entry.region;
        result["widgets"].append(std::move(item));
    }
    return result;
}

void WebApplication::forceSnapshot()
{
    structureDirty_ = true;
}
void WebApplication::flush()
{
    assertThread();
    uiTree_->sync();
    if (!session_.canPublish())
        return;
    if (structureDirty_)
    {
        session_.publish(snapshot());
        structureDirty_ = false;
        pending_ = JsonCpp::Value{JsonCpp::objectValue};
    }
    else if (!pending_.empty())
    {
        JsonCpp::Value update{JsonCpp::objectValue};
        update["type"] = "patch";
        update["widgets"] = pending_;
        session_.publish(std::move(update));
        pending_ = JsonCpp::Value{JsonCpp::objectValue};
    }
}

void WebApplication::dispatch(const JsonCpp::Value &event)
{
    assertThread();
    if (!event["id"].isString() || !event["event"].isString())
        throw std::invalid_argument("Invalid widget event");
    uiTree_->sync();
    if (uiTree_->dispatch(event))
        return;
    const auto it = widgets_.find(event["id"].asString());
    if (it == widgets_.end())
        throw std::invalid_argument("Unknown widget");
    const auto &props = it->second.properties;
    if (props.get("enabled", true).asBool() == false ||
        props.get("visible", true).asBool() == false)
        throw std::invalid_argument("Widget is disabled or hidden");
    // Copy: the callback is allowed to unregister/destroy its own widget.
    auto handler = it->second.handler;
    if (!handler)
        throw std::invalid_argument("Widget does not accept events");
    handler(event); // Adapter validates event names, types, and bounds.
}

std::string WebApplication::getOpenFileName(const std::string &,
                                            const std::string &)
{
    unsupported("getOpenFileName");
}
std::vector<std::string> WebApplication::getOpenFileNames(const std::string &,
                                                          const std::string &)
{
    unsupported("getOpenFileNames");
}
std::string WebApplication::getSaveFileName(const std::string &,
                                            const std::string &,
                                            const std::string &,
                                            std::string *,
                                            int)
{
    unsupported("getSaveFileName");
}
int WebApplication::showDialog(Dialog &)
{
    unsupported("showDialog");
}
void WebApplication::openDialog(Dialog &)
{
    unsupported("openDialog");
}
Pixmap WebApplication::loadPixmap(const std::string &) const
{
    return Pixmap{};
}
