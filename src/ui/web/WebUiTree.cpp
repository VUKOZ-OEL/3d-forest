
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
#include <Dialog.hpp>
#include <GridLayout.hpp>
#include <GroupBox.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <Layout.hpp>
#include <MessageBox.hpp>
#include <NavigationItem.hpp>
#include <ProgressDialog.hpp>
#include <Splitter.hpp>
#include <ToolBar.hpp>
#include <Viewer.hpp>
#include <WebApplication.hpp>
#include <WebAssets.hpp>
#include <WebControls.hpp>
#include <WebUiTree.hpp>
#include <WebViewer.hpp>

// Include std.
#include <algorithm>
#include <cctype>
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
        if (auto *viewer = dynamic_cast<Viewer *>(common))
        {
            adapter = std::make_shared<WebViewer>(application_, viewer);
        }
        else
        {
            adapter =
                std::make_shared<WebControls>(application_,
                                              common,
                                              WebControls::typeOf(common));
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
    widgetDialogs_[found->second->id()] = currentDialog_;
    if (auto *bar = dynamic_cast<ToolBar *>(common))
    {
        node["children"] = JsonCpp::Value(JsonCpp::arrayValue);
        for (auto &item : bar->items())
            node["children"].append(item.widget
                                        ? widget(item.widget, enabled, visible)
                                        : JsonCpp::Value());
    }
    if (auto *splitter = dynamic_cast<Splitter *>(common))
    {
        node["children"] = JsonCpp::Value(JsonCpp::arrayValue);
        for (auto *child : splitter->widgets())
            node["children"].append(widget(child, enabled, visible));
    }
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

JsonCpp::Value WebUiTree::navigation(const std::vector<NavigationItem *> &items,
                                     bool loginRequired)
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
        bool requiresLogin = loginRequired;
        if (!item->parent())
        {
            std::string category = item->title();
            std::transform(category.begin(),
                           category.end(),
                           category.begin(),
                           [](unsigned char c)
                           { return char(std::tolower(c)); });
            requiresLogin = category == "edit" || category == "process";
            node["enabled"] = !requiresLogin || application_.loggedIn();
        }
        if (!item->parent())
        {
            std::string category = item->title();
            std::transform(category.begin(),
                           category.end(),
                           category.begin(),
                           [](unsigned char c)
                           { return char(std::tolower(c)); });
            if (category != "file" && category != "edit" &&
                category != "process" && category != "filter" &&
                category != "tool" && category != "settings" &&
                category != "help")
                category = "tool";
            node["icon"] = webResourceImage(":/ApplicationResources/category-" +
                                            category + "-24px.png");
        }
        node["kind"] = item->isGroup() ? "group" : "action";
        node["children"] = navigation(item->children(), requiresLogin);
        if (auto *action = item->action())
        {
            node["title"] = action->text();
            node["toolTip"] = action->toolTip();
            node["checkable"] = action->isCheckable();
            if (action->isCheckable())
                node["checked"] = action->isChecked();
            node["openExternalWebApp"] = action->opensExternalWebApp();
            if (action->panel())
                node["panel"] =
                    widget(action->panel(),
                           !requiresLogin || application_.loggedIn());
            else
            {
                actions_[it->second] = action;
                if (requiresLogin)
                    restrictedActions_.insert(it->second);
            }
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
    restrictedActions_.clear();
    widgetDialogs_.clear();
    currentDialog_.clear();
    JsonCpp::Value next{JsonCpp::objectValue};
    next["navigation"] = navigation(application_.navigation().items());
    next["accountIcon"] =
        webResourceImage(":/ApplicationResources/category-account.svg");
    if (viewer_.widget && !viewer_.lifetime.expired())
        next["viewer"] = widget(viewer_.widget);
    else
        viewer_ = {};
    if (bottom_.widget && !bottom_.lifetime.expired())
        next["bottom"] = widget(bottom_.widget, true, bottomVisible_);
    else
        bottom_ = {};
    next["bottomVisible"] = bottomVisible_ && bottom_.widget;
    next["dialogs"] = JsonCpp::Value(JsonCpp::arrayValue);
    for (auto &d : dialogs_)
    {
        if (!d->dialog || !d->visible)
            continue;
        auto *dialog = d->dialog;
        currentDialog_ = d->id;
        JsonCpp::Value node(JsonCpp::objectValue);
        node["id"] = d->id;
        node["title"] = dialog->windowTitle();
        node["modal"] = d->blocking || dialog->isModal();
        node["buttons"] = dialog->standardButtons();
        node["defaultButton"] = int(dialog->defaultButton());
        node["width"] = dialog->sizeHint().width();
        node["height"] = dialog->sizeHint().height();
        node["fixedHeight"] = dialog->fixedHeight();
        node["maxWidth"] = dialog->maximumWidth();
        node["maxHeight"] = dialog->maximumHeight();
        node["raise"] = d->raise;
        if (dialog->layout())
            node["layout"] = layout(dialog->layout(), true, true);
        if (auto *box = dynamic_cast<MessageBox *>(dialog))
        {
            node["message"] = box->text();
            node["information"] = box->informativeText();
        }
        if (auto *progress = dynamic_cast<ProgressDialog *>(dialog))
        {
            node["progress"] = true;
            node["label"] = progress->labelText();
            node["minimum"] = progress->minimum();
            node["maximum"] = progress->maximum();
            node["value"] = progress->value();
            node["cancelText"] = progress->cancelButtonText();
        }
        next["dialogs"].append(node);
    }
    currentDialog_.clear();
    dialogs_.erase(std::remove_if(dialogs_.begin(),
                                  dialogs_.end(),
                                  [](const auto &d) { return !d->dialog; }),
                   dialogs_.end());
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
        application_.forceUi();
    }
}

bool WebUiTree::dispatch(const JsonCpp::Value &event)
{
    const auto id = event["id"].asString();
    if (!allowed(id))
        throw std::invalid_argument("A modal dialog is active");
    if (!id.empty() && id.front() == 'd')
    {
        auto it =
            std::find_if(dialogs_.begin(),
                         dialogs_.end(),
                         [&](const auto &d)
                         { return d->id == id && d->dialog && d->visible; });
        if (it == dialogs_.end())
            throw std::invalid_argument("Dialog has closed");
        auto state = *it;
        auto *d = state->dialog;
        auto name = event["event"].asString();
        if (name == "close")
        {
            d->close();
            return true;
        }
        if (name == "cancel")
        {
            if (auto *p = dynamic_cast<ProgressDialog *>(d))
            {
                p->cancel();
                return true;
            }
        }
        if (name == "resize")
        {
            if (event["width"].isInt() && event["height"].isInt())
                d->updateSize(std::clamp(event["width"].asInt(), 0, 10000),
                              std::clamp(event["height"].asInt(), 0, 10000));
            return true;
        }
        if (name != "button" || !event["button"].isInt())
            throw std::invalid_argument("Invalid dialog event");
        int b = event["button"].asInt();
        if (b <= 0 || (b & (b - 1)) || (d->standardButtons() & b) != b)
            throw std::invalid_argument("Invalid dialog button");
        d->buttonClicked(Dialog::StandardButton(b));
        if (!state->dialog || !state->visible)
            return true;
        if (dynamic_cast<MessageBox *>(d))
        {
            d->done(b);
            return true;
        }
        switch (b)
        {
            case Dialog::Ok:
            case Dialog::Save:
            case Dialog::SaveAll:
            case Dialog::Open:
            case Dialog::Yes:
            case Dialog::YesToAll:
                d->accept();
                break;
            case Dialog::Cancel:
            case Dialog::Close:
            case Dialog::No:
            case Dialog::NoToAll:
            case Dialog::Abort:
            case Dialog::Discard:
                d->reject();
                break;
            default:
                break;
        }
        return true;
    }
    if (id.empty() || id.front() != 'n')
        return false;
    const auto it = actions_.find(id);
    if (it == actions_.end() || event["event"] != "triggered")
        throw std::invalid_argument("Unknown navigation action");
    Action *action = it->second;
    if (restrictedActions_.count(id) && !application_.loggedIn())
        throw std::invalid_argument("Sign in to use this feature");
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

std::shared_ptr<WebUiTree::DialogState> WebUiTree::show(Dialog &dialog,
                                                        bool blocking)
{
    auto it = std::find_if(dialogs_.begin(),
                           dialogs_.end(),
                           [&](const auto &d) { return d->dialog == &dialog; });
    std::shared_ptr<DialogState> state;
    if (it != dialogs_.end())
    {
        state = *it;
        dialogs_.erase(it);
    }
    else
    {
        state = std::make_shared<DialogState>();
        state->dialog = &dialog;
        state->id = "d" + std::to_string(++nextDialogId_);
        std::weak_ptr<DialogState> weak = state;
        dialog.destroyed.connect(
            [weak]
            {
                if (auto s = weak.lock())
                {
                    s->dialog = nullptr;
                    s->visible = false;
                }
            });
        dialog.finished.connect(
            [weak](int result)
            {
                if (auto s = weak.lock())
                {
                    s->result = result;
                    s->visible = false;
                }
            });
        dialog.hideRequested.connect(
            [weak]
            {
                if (auto s = weak.lock())
                    s->visible = false;
            });
        if (auto *progress = dynamic_cast<ProgressDialog *>(&dialog))
        {
            progress->resetRequested.connect(
                [weak]
                {
                    if (auto s = weak.lock())
                        s->visible = false;
                });
            progress->valueChanged.connect(
                [weak](int value)
                {
                    if (auto s = weak.lock())
                        if (auto *p = dynamic_cast<ProgressDialog *>(s->dialog))
                            if (p->maximum() > p->minimum() &&
                                value == p->maximum())
                                s->visible = false;
                });
        }
        dialog.raiseRequested.connect(
            [weak]
            {
                if (auto s = weak.lock())
                    ++s->raise;
            });
        dialog.activateWindowRequested.connect(
            [weak]
            {
                if (auto s = weak.lock())
                    ++s->raise;
            });
    }
    state->visible = true;
    state->blocking = blocking;
    state->result = Dialog::Rejected;
    dialogs_.push_back(state);
    return state;
}
void WebUiTree::open(Dialog &dialog)
{
    show(dialog, false);
}
int WebUiTree::exec(Dialog &dialog)
{
    auto state = show(dialog, true);
    // Keep the original plugin callback on the stack while processing browser
    // events on this same session thread, including nested modal dialogs.
    while (state->dialog && state->visible)
        application_.pump(true);
    return state->result;
}
bool WebUiTree::allowed(const std::string &id) const
{
    int modal = -1, owner = -1;
    auto w = widgetDialogs_.find(id);
    const auto dialogId = w == widgetDialogs_.end() ? id : w->second;
    for (std::size_t i = 0; i < dialogs_.size(); ++i)
    {
        const auto &s = dialogs_[i];
        if (!s->visible || !s->dialog)
            continue;
        if (s->blocking || s->dialog->isModal())
            modal = int(i);
        if (s->id == dialogId)
            owner = int(i);
    }
    return modal < 0 || owner >= modal;
}
