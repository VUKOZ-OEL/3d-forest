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
#include <WebFileDialog.hpp>
#include <WebSession.hpp>
#include <WebUiTree.hpp>

// Include std.
#include <algorithm>
#include <stdexcept>
#include <utility>

// Include local.
#define LOG_MODULE_NAME "WebApplication"
#include <Log.hpp>

WebApplication::WebApplication(WebSession &session)
    : session_(session),
      uiTree_(std::make_unique<WebUiTree>(*this))
{
    loggedIn_ = session.loggedIn();
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
    skipNextUiSync_ = false;
    Application::processEvents();
    processRenderRequest();
    if (wakeUpHandler_)
        wakeUpHandler_(*this);
}
void WebApplication::processEvents()
{
    assertThread();
    pump(false);
}

bool WebApplication::openLocalWebApp(int port)
{
    assertThread();
    if (port < 1 || port > 65535)
        return false;

    session_.setLocalServicePort(port);
    externalLink_ = JsonCpp::Value{JsonCpp::objectValue};
    externalLink_["url"] = "/marteloscope/" + session_.token() + "/";
    externalLink_["text"] = "Open 3d-Marteloscope";
    externalLinkDirty_ = true;
    return true;
}

void WebApplication::closeLocalWebApp()
{
    assertThread();
    session_.setLocalServicePort(0);
    externalLink_ = JsonCpp::Value{JsonCpp::objectValue};
    externalLinkDirty_ = true;
}

std::string WebApplication::localWebAppBasePath() const
{
    return "marteloscope/" + session_.token();
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

bool WebApplication::authenticationEnabled() const
{
    return session_.authenticationEnabled();
}

void WebApplication::setLoggedIn(bool value)
{
    assertThread();
    if (loggedIn_ == value)
        return;
    loggedIn_ = value;
    signalLoginChanged(value);
    forceUi();
}

JsonCpp::Value WebApplication::snapshot() const
{
    JsonCpp::Value result{JsonCpp::objectValue};
    result["type"] = "snapshot";
    result["eventSequence"] = eventSequence_;
    result["application"] = metadata_;
    result["externalLink"] = externalLink_;
    result["loggedIn"] = loggedIn_;
    result["authenticationEnabled"] = authenticationEnabled();
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
    skipNextUiSync_ = false;
    structureDirty_ = true;
}
void WebApplication::flush()
{
    assertThread();
    if (skipNextUiSync_)
    {
        skipNextUiSync_ = false;
        return;
    }
    uiTree_->sync();
    if (!session_.canPublish())
        return;
    if (structureDirty_)
    {
        session_.publish(snapshot());
        structureDirty_ = false;
        uiDirty_ = false;
        externalLinkDirty_ = false;
        publishedEventSequence_ = eventSequence_;
        pending_ = JsonCpp::Value{JsonCpp::objectValue};
    }
    else if (!pending_.empty() || uiDirty_ || externalLinkDirty_ ||
             publishedEventSequence_ != eventSequence_)
    {
        JsonCpp::Value update{JsonCpp::objectValue};
        update["type"] = "patch";
        update["eventSequence"] = eventSequence_;
        publishedEventSequence_ = eventSequence_;
        update["widgets"] = pending_;
        if (externalLinkDirty_)
        {
            update["externalLink"] = externalLink_;
            externalLinkDirty_ = false;
        }
        if (uiDirty_)
            update["ui"] = uiTree_->description();
        uiDirty_ = false;
        session_.publish(std::move(update));
        pending_ = JsonCpp::Value{JsonCpp::objectValue};
    }
}

void WebApplication::dispatch(const JsonCpp::Value &event)
{
    assertThread();
    if (!event["id"].isString() || !event["event"].isString())
        throw std::invalid_argument("Invalid widget event");
    // A read-only binary pull does not mutate common controls. Avoid walking
    // and serializing every table/layout for each point chunk. Render wakeups
    // and all ordinary UI events still use the normal reconciliation path.
    if (event["event"] == "viewerPull")
    {
        const auto it = widgets_.find(event["id"].asString());
        if (it == widgets_.end() || it->second.type != "Viewer" ||
            !it->second.properties.get("enabled", true).asBool() ||
            !it->second.properties.get("visible", true).asBool())
            throw std::invalid_argument("Viewer is unavailable");
        auto handler = it->second.handler;
        if (!handler)
            throw std::invalid_argument("Viewer is unavailable");
        handler(event);
        skipNextUiSync_ = true;
        return;
    }
    skipNextUiSync_ = false;
    if (event.isMember("sequence"))
    {
        if (!event["sequence"].isUInt64())
            throw std::invalid_argument("Invalid event sequence");
        eventSequence_ =
            (std::max)(eventSequence_, event["sequence"].asUInt64());
    }
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

std::string WebApplication::getOpenFileName(const std::string &title,
                                            const std::string &filter)
{
    auto files = WebFileDialog::choose(*this,
                                       title,
                                       filter,
                                       false,
                                       false,
                                       {},
                                       nullptr,
                                       0);
    return files.empty() ? std::string{} : files.front();
}
std::vector<std::string> WebApplication::getOpenFileNames(
    const std::string &title,
    const std::string &filter)
{
    return WebFileDialog::choose(*this,
                                 title,
                                 filter,
                                 true,
                                 false,
                                 {},
                                 nullptr,
                                 0);
}
std::string WebApplication::getSaveFileName(const std::string &caption,
                                            const std::string &dir,
                                            const std::string &filter,
                                            std::string *selectedFilter,
                                            int options)
{
    auto files = WebFileDialog::choose(*this,
                                       caption,
                                       filter,
                                       false,
                                       true,
                                       dir,
                                       selectedFilter,
                                       options);
    return files.empty() ? std::string{} : files.front();
}
int WebApplication::showDialog(Dialog &dialog)
{
    assertThread();
    return uiTree_->exec(dialog);
}
void WebApplication::openDialog(Dialog &dialog)
{
    assertThread();
    uiTree_->open(dialog);
    flush();
}
void WebApplication::pump(bool wait)
{
    assertThread();
    flush();
    session_.pumpOne(*this, wait);
    flush();
}
Pixmap WebApplication::loadPixmap(const std::string &) const
{
    // WebAssets sends encoded resource images directly to the browser.
    return Pixmap{};
}

bool WebApplication::sendViewerPacket(const std::string &packet)
{
    assertThread();
    if (packet.size() > 64 + 16384 * 16)
        throw std::invalid_argument("Viewer packet exceeds chunk limit");
    if (!session_.connection_ || !session_.connection_->connected())
        return false;
    session_.connection_->send(packet, drogon::WebSocketMessageType::Binary);
    return true;
}

void WebApplication::setViewerMemoryBudget(std::size_t bytes)
{
    assertThread();
    if (initialized_ || bytes < 1024 * 1024 || bytes > 128 * 1024 * 1024)
        throw std::invalid_argument(
            "Set viewer budget to 1..128 MiB before init()");
    viewerMemoryBudget_ = bytes;
}
