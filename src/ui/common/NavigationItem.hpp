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

/** @file NavigationItem.hpp */

#ifndef NAVIGATION_ITEM_HPP
#define NAVIGATION_ITEM_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
class Action;
class Plugin;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** NavigationItem. */
class EXPORT_UI_COMMON NavigationItem
{
public:
    enum class Type
    {
        Group,
        Action
    };

    NavigationItem(Type type, const std::string &title, int order);

    ~NavigationItem();

    Type type() const { return type_; }

    bool isGroup() const { return type_ == Type::Group; }
    bool isAction() const { return type_ == Type::Action; }

    const std::string &title() const { return title_; }

    int order() const { return order_; }

    Plugin *owner() const { return owner_; }

    Action *action() const { return action_; }

    NavigationItem *parent() const { return parent_; }

    const std::vector<NavigationItem *> &children() const { return children_; }

private:
    friend class NavigationTree;

    Type type_;
    std::string title_;
    int order_{0};

    NavigationItem *parent_{nullptr};
    std::vector<NavigationItem *> children_;

    Plugin *owner_{nullptr};
    Action *action_{nullptr};
};

#include <WarningsEnable.hpp>

#endif /* NAVIGATION_ITEM_HPP */
