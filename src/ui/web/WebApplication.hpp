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

/** @file WebApplication.hpp */

#pragma once

// Include 3D Forest.
#include <Application.hpp>
class Widget;
class WebSession;
class WebUiTree;

// Include 3rd party.
#include <json/json.h>

// Include std.
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebApplication. */
class EXPORT_UI_WEB WebApplication : public Application
{
public:
    using EventHandler = std::function<void(const JsonCpp::Value &)>;
    using Handler = std::function<void(WebApplication &)>;

    explicit WebApplication(WebSession &session);
    ~WebApplication() override;
    WebApplication(const WebApplication &) = delete;
    WebApplication &operator=(const WebApplication &) = delete;

    void init();
    void setInitializeHandler(Handler handler);
    void setWakeUpHandler(Handler handler);
    void wakeUp() override; // The only method callable from another thread.
    void processEvents() override; // Flush only; no nested event loop.

    void setOrganizationName(const std::string &value);
    void setApplicationName(const std::string &value);
    void setApplicationVersion(const std::string &value);
    void setWindowIcon(const std::string &url); // Browser URL, not QIcon.

    void setViewer(Widget *widget) override;
    void removeViewer(Widget *widget) override;
    void showBottomWidget(Widget *widget) override;
    void hideBottomWidget() override;
    void toggleBottomWidget(Widget *widget) override;
    void removeBottomWidget(Widget *widget) override;

    // Backend registration used by web adapters.
    // Common widgets stay owned by your common UI/plugin tree.
    std::string registerWidget(Widget *widget,
                               const std::string &type,
                               JsonCpp::Value properties,
                               EventHandler handler = {});
    void unregisterWidget(Widget *widget); // Called automatically by adapters.
    void setWidgetProperties(Widget *widget, const JsonCpp::Value &properties);
    void setWidgetEventHandler(Widget *widget, EventHandler handler);
    std::string widgetId(Widget *widget) const;
    std::weak_ptr<void> lifetime() const { return lifetime_; }

    std::string getOpenFileName(const std::string &title,
                                const std::string &filter) override;
    std::vector<std::string> getOpenFileNames(
        const std::string &title,
        const std::string &filter) override;
    std::string getSaveFileName(const std::string &caption = "",
                                const std::string &dir = "",
                                const std::string &filter = "",
                                std::string *selectedFilter = nullptr,
                                int options = 0) override;
    int showDialog(Dialog &dialog) override;
    void openDialog(Dialog &dialog) override;
    Pixmap loadPixmap(const std::string &fileName) const override;

private:
    friend class WebSession;
    friend class WebUiTree;
    struct Entry
    {
        Widget *widget{};
        std::string type;
        JsonCpp::Value properties{JsonCpp::objectValue};
        EventHandler handler;
        std::string region{"sidebar"};
    };
    WebSession &session_;
    std::unique_ptr<WebUiTree> uiTree_;
    std::shared_ptr<void> lifetime_{std::make_shared<int>(0)};
    std::map<std::string, Entry> widgets_;
    std::unordered_map<Widget *, std::string> ids_;
    JsonCpp::Value metadata_{JsonCpp::objectValue};
    JsonCpp::Value pending_{JsonCpp::objectValue};
    bool structureDirty_{true};
    bool initialized_{false};
    unsigned long long nextId_{0};
    Handler initializeHandler_;
    Handler wakeUpHandler_;

    void assertThread() const;
    void setMetadata(const char *name, const std::string &value);
    JsonCpp::Value snapshot() const;
    void flush();
    void forceSnapshot();
    void dispatch(const JsonCpp::Value &event);
    void handleWakeUp();
};

#include <WarningsEnable.hpp>
