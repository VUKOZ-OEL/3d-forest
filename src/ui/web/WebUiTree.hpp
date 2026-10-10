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

/** @file WebUiTree.hpp */

#pragma once

// Include 3D Forest.
class WebApplication;
class WebWidget;
class Widget;
class Layout;
class Action;
class NavigationItem;

// Include 3rd party.
#include <json/json.h>

// Include std.
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebUiTree.

    Reconciles the existing common navigation/layout tree on the session
    thread.
    Structural reconciliation is deferred until mutations have completed.
*/
class EXPORT_UI_WEB WebUiTree
{
public:
    explicit WebUiTree(WebApplication &application);
    ~WebUiTree();
    void sync();
    const JsonCpp::Value &description() const { return description_; }
    bool dispatch(const JsonCpp::Value &event);
    void setViewer(Widget *widget);
    void removeViewer(Widget *widget);
    void showBottomWidget(Widget *widget);
    void hideBottomWidget();
    void toggleBottomWidget(Widget *widget);
    void removeBottomWidget(Widget *widget);

private:
    struct Root
    {
        Widget *widget{nullptr};
        std::weak_ptr<void> lifetime;
    };
    WebApplication &application_;
    std::shared_ptr<void> lifetime_{std::make_shared<int>(0)};
    std::map<Widget *, std::shared_ptr<WebWidget>> adapters_;
    std::map<NavigationItem *, std::string> navigationIds_;
    std::map<std::string, Action *> actions_;
    std::set<Widget *> seenWidgets_;
    std::set<Layout *> seenLayouts_;
    std::set<NavigationItem *> seenNavigation_;
    unsigned long long nextNavigationId_{0};
    Root viewer_;
    Root bottom_;
    bool bottomVisible_{false};
    JsonCpp::Value description_{JsonCpp::objectValue};

    JsonCpp::Value widget(Widget *widget,
                          bool enabled = true,
                          bool visible = true);
    JsonCpp::Value layout(Layout *layout, bool enabled, bool visible);
    JsonCpp::Value navigation(const std::vector<NavigationItem *> &items);
    void forgetNavigation(NavigationItem *item);
};

#include <WarningsEnable.hpp>
