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

/** @file WebWidget.hpp */

#pragma once

// Include 3D Forest.
#include <Widget.hpp>
class WebApplication;

// Include 3rd party.
#include <json/json.h>

// Include std.
#include <memory>
#include <string>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebWidget.

    A backend adapter; it does not own the common widget.
*/
class EXPORT_UI_WEB WebWidget : public std::enable_shared_from_this<WebWidget>
{
public:
    WebWidget(WebApplication &application,
              Widget *widget,
              const std::string &type);
    virtual ~WebWidget();
    WebWidget(const WebWidget &) = delete;
    WebWidget &operator=(const WebWidget &) = delete;

    const std::string &id() const { return id_; }
    bool alive() const { return !widgetLife_.expired(); }
    virtual void bind(); // Called after make_shared().
    virtual void sync(bool enabled, bool visible);
    virtual void handleEvent(const JsonCpp::Value &event);

protected:
    WebApplication &application_;
    Widget *widget_;
    std::weak_ptr<void> widgetLife_;
    std::string id_;
    void patch(const JsonCpp::Value &properties);
};

#include <WarningsEnable.hpp>
