
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

/** @file WebUiTree.cpp */

// Include 3D Forest.
#include <Action.hpp>
#include <GridLayout.hpp>
#include <GroupBox.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <Layout.hpp>
#include <NavigationItem.hpp>
#include <Slider.hpp>
#include <WebApplication.hpp>
#include <WebSlider.hpp>
#include <WebUiTree.hpp>

// Include std.
#include <stdexcept>

// Include local.
#define LOG_MODULE_NAME "WebUiTree"
#include <Log.hpp>

WebUiTree::WebUiTree(WebApplication &app) : application_(app)
{
    const std::weak_ptr<void> guard = lifetime_;
    // Remove ID mappings before deletion, including allocator address reuse.
    app.navigation().itemAboutToBeRemoved.connect(
        [this, guard](NavigationItem *item)
        {
            if (!guard.expired())
                forgetNavigation(item);
        });
}
WebUiTree::~WebUiTree()
{
    lifetime_.reset();
    adapters_.clear();
}
void WebUiTree::forgetNavigation(NavigationItem *item)
{
    if (auto it = navigationIds_.find(item); it != navigationIds_.end())
    {
        actions_.erase(it->second);
        navigationIds_.erase(it);
    }
    for (auto *child : item->children())
        forgetNavigation(child);
}

JsonCpp::Value WebUiTree::widget(Widget *common, bool enabled, bool visible)
{
    if (!common)
        return {};
    if (!seenWidgets_.insert(common).second)
        throw std::logic_error(
            "Common UI must be an ownership tree, not shared/cyclic widgets");
    auto found = adapters_.find(common);
    if (found != adapters_.end() && !found->second->alive())
    {
        adapters_.erase(found);
        found = adapters_.end();
    }
    if (found == adapters_.end())
    {
        std::shared_ptr<WebWidget> adapter;
        if (auto *slider = dynamic_cast<Slider *>(common))
            adapter = std::make_shared<WebSlider>(application_, slider);
        else
        {
            const char *type = dynamic_cast<Label *>(common)      ? "Label"
                               : dynamic_cast<GroupBox *>(common) ? "GroupBox"
                               : common->layout()                 ? "Widget"
                                                  : "Unsupported";
            adapter = std::make_shared<WebWidget>(application_, common, type);
        }
        found = adapters_.emplace(common, std::move(adapter)).first;
        found->second->bind();
    }
    enabled = enabled && common->isEnabled();
    visible = visible && common->isVisible();
    found->second->sync(enabled, visible);
    JsonCpp::Value node{JsonCpp::objectValue};
    node["kind"] = "widget";
    node["id"] = found->second->id();
    if (common->layout())
        node["layout"] = layout(common->layout(), enabled, visible);
    return node;
}

JsonCpp::Value WebUiTree::layout(Layout *common, bool enabled, bool visible)
{
    if (!seenLayouts_.insert(common).second)
        throw std::logic_error(
            "Shared or cyclic common layouts are not supported");
    auto *grid = dynamic_cast<GridLayout *>(common);
    JsonCpp::Value node{JsonCpp::objectValue};
    node["kind"] = grid                                 ? "grid"
                   : dynamic_cast<HBoxLayout *>(common) ? "hbox"
                                                        : "vbox";
    node["spacing"] = common->spacing();
    node["margins"] = JsonCpp::Value{JsonCpp::arrayValue};
    for (int v : {common->leftMargin(),
                  common->topMargin(),
                  common->rightMargin(),
                  common->bottomMargin()})
        node["margins"].append(v);
    node["items"] = JsonCpp::Value{JsonCpp::arrayValue};
    std::size_t index = 0;
    for (const auto &item : common->items())
    {
        JsonCpp::Value child{JsonCpp::objectValue};
        if (item.widget())
            child = widget(item.widget(), enabled, visible);
        else if (item.layout())
            child = layout(item.layout(), enabled, visible);
        else
        {
            child["kind"] =
                item.type() == LayoutItem::StretchItem ? "stretch" : "spacing";
            child["spacing"] = item.spacing();
        }
        child["stretch"] = item.stretch();
        child["alignment"] = item.alignment();
        if (grid)
        {
            const auto &p = grid->positionAt(index);
            child["row"] = p.row;
            child["column"] = p.column;
            child["rowSpan"] = p.rowSpan;
            child["columnSpan"] = p.columnSpan;
        }
        node["items"].append(std::move(child));
        ++index;
    }
    return node;
}

JsonCpp::Value WebUiTree::navigation(const std::vector<NavigationItem *> &items)
{
    JsonCpp::Value result{JsonCpp::arrayValue};
    for (auto *item : items)
    {
        seenNavigation_.insert(item);
        auto [it, inserted] = navigationIds_.try_emplace(item);
        if (inserted)
            it->second = "n" + std::to_string(++nextNavigationId_);
        JsonCpp::Value node{JsonCpp::objectValue};
        node["id"] = it->second;
        node["title"] = item->title();
        node["kind"] = item->isGroup() ? "group" : "action";
        node["children"] = navigation(item->children());
        if (auto *action = item->action())
        {
            node["title"] = action->text();
            node["toolTip"] = action->toolTip();
            if (action->panel())
                node["panel"] = widget(action->panel());
            else
                actions_[it->second] = action;
        }
        result.append(std::move(node));
    }
    return result;
}

void WebUiTree::sync()
{
    seenWidgets_.clear();
    seenLayouts_.clear();
    seenNavigation_.clear();
    actions_.clear();
    JsonCpp::Value next{JsonCpp::objectValue};
    next["navigation"] = navigation(application_.navigation().items());
    if (viewer_.widget && !viewer_.lifetime.expired())
        next["viewer"] = widget(viewer_.widget);
    else
        viewer_ = {};
    if (bottom_.widget && !bottom_.lifetime.expired())
        next["bottom"] = widget(bottom_.widget, true, bottomVisible_);
    else
        bottom_ = {};
    next["bottomVisible"] = bottomVisible_ && bottom_.widget;
    for (auto it = adapters_.begin(); it != adapters_.end();)
        if (!seenWidgets_.count(it->first))
            it = adapters_.erase(it);
        else
            ++it;
    for (auto it = navigationIds_.begin(); it != navigationIds_.end();)
        if (!seenNavigation_.count(it->first))
            it = navigationIds_.erase(it);
        else
            ++it;
    if (next != description_)
    {
        description_ = std::move(next);
        application_.forceSnapshot();
    }
}

bool WebUiTree::dispatch(const JsonCpp::Value &event)
{
    const auto id = event["id"].asString();
    if (id.empty() || id.front() != 'n')
        return false;
    const auto it = actions_.find(id);
    if (it == actions_.end() || event["event"] != "triggered")
        throw std::invalid_argument("Unknown navigation action");
    Action *action = it->second;
    action->trigger();
    return true;
}
void WebUiTree::setViewer(Widget *w)
{
    if (w && bottom_.widget == w)
    {
        bottom_ = {};
        bottomVisible_ = false;
    }
    viewer_ = w ? Root{w, w->lifetime()} : Root{};
}
void WebUiTree::removeViewer(Widget *w)
{
    if (viewer_.widget == w)
        viewer_ = {};
}
void WebUiTree::showBottomWidget(Widget *w)
{
    if (!w)
        throw std::invalid_argument("Null bottom widget");
    if (viewer_.widget == w)
        viewer_ = {};
    bottom_ = {w, w->lifetime()};
    bottomVisible_ = true;
}
void WebUiTree::hideBottomWidget()
{
    bottomVisible_ = false;
}
void WebUiTree::toggleBottomWidget(Widget *w)
{
    if (bottom_.widget == w && !bottom_.lifetime.expired() && bottomVisible_)
        hideBottomWidget();
    else
        showBottomWidget(w);
}
void WebUiTree::removeBottomWidget(Widget *w)
{
    if (bottom_.widget == w)
    {
        bottom_ = {};
        bottomVisible_ = false;
    }
}
