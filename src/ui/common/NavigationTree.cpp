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

/** @file NavigationTree.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <Action.hpp>
#include <NavigationTree.hpp>

// Include local.
#define LOG_MODULE_NAME "NavigationTree"
#include <Log.hpp>

NavigationTree::~NavigationTree()
{
    for (NavigationItem *item : items_)
    {
        delete item;
    }
}

void NavigationTree::addItem(Plugin *owner,
                             const std::vector<NavigationPathItem> &parentPath,
                             Action *action,
                             int order)
{
    if (!owner)
    {
        return;
    }

    if (!action)
    {
        return;
    }

    // Do not add the same Action twice.
    if (findItem(action))
    {
        return;
    }

    NavigationItem *parent = createGroups(parentPath);

    NavigationItem *item =
        new NavigationItem(NavigationItem::Type::Action, action->text(), order);

    item->parent_ = parent;
    item->owner_ = owner;
    item->action_ = action;

    std::vector<NavigationItem *> &siblings =
        parent ? parent->children_ : items_;

    siblings.push_back(item);
    sortItems(siblings);

    itemAdded(item);
}

void NavigationTree::removeItem(Action *action)
{
    removeNavigationItem(findItem(action));
}

void NavigationTree::removeNavigationItem(NavigationItem *item)
{
    if (!item)
    {
        return;
    }

    NavigationItem *parent = item->parent_;

    itemAboutToBeRemoved(item);

    std::vector<NavigationItem *> &siblings =
        parent ? parent->children_ : items_;

    const auto it = std::find(siblings.begin(), siblings.end(), item);

    if (it != siblings.end())
    {
        siblings.erase(it);
    }

    Action *action = item->action_;
    delete item;
    delete action;

    removeEmptyGroups(parent);
}

NavigationItem *NavigationTree::findGroup(NavigationItem *parent,
                                          const std::string &title) const
{
    const std::vector<NavigationItem *> &children =
        parent ? parent->children_ : items_;

    for (NavigationItem *item : children)
    {
        if (item->type_ == NavigationItem::Type::Group && item->title_ == title)
        {
            return item;
        }
    }

    return nullptr;
}

NavigationItem *NavigationTree::findOrCreateGroup(NavigationItem *parent,
                                                  const std::string &title,
                                                  int order)
{
    if (NavigationItem *existing = findGroup(parent, title))
    {
        return existing;
    }

    NavigationItem *group =
        new NavigationItem(NavigationItem::Type::Group, title, order);

    group->parent_ = parent;

    std::vector<NavigationItem *> &siblings =
        parent ? parent->children_ : items_;

    siblings.push_back(group);
    sortItems(siblings);

    itemAdded(group);

    return group;
}

NavigationItem *NavigationTree::createGroups(
    const std::vector<NavigationPathItem> &path)
{
    NavigationItem *parent = nullptr;

    for (const NavigationPathItem &part : path)
    {
        if (part.title.empty())
        {
            continue;
        }

        parent = findOrCreateGroup(parent, part.title, part.order);
    }

    return parent;
}

NavigationItem *NavigationTree::findItem(
    const std::vector<NavigationItem *> &items,
    Action *action) const
{
    for (NavigationItem *item : items)
    {
        if (item->type_ == NavigationItem::Type::Action &&
            item->action_ == action)
        {
            return item;
        }

        if (NavigationItem *found = findItem(item->children_, action))
        {
            return found;
        }
    }

    return nullptr;
}

NavigationItem *NavigationTree::findItem(Action *action) const
{
    if (!action)
    {
        return nullptr;
    }

    return findItem(items_, action);
}

void NavigationTree::removeEmptyGroups(NavigationItem *group)
{
    while (group && group->type_ == NavigationItem::Type::Group &&
           group->children_.empty())
    {
        NavigationItem *parent = group->parent_;

        itemAboutToBeRemoved(group);

        std::vector<NavigationItem *> &siblings =
            parent ? parent->children_ : items_;

        const auto it = std::find(siblings.begin(), siblings.end(), group);

        if (it != siblings.end())
        {
            siblings.erase(it);
        }

        delete group;
        group = parent;
    }
}

void NavigationTree::sortItems(std::vector<NavigationItem *> &items)
{
    std::sort(items.begin(),
              items.end(),
              [](const NavigationItem *a, const NavigationItem *b)
              {
                  if (a->order() != b->order())
                  {
                      return a->order() < b->order();
                  }

                  return a->title() < b->title();
              });
}

void NavigationTree::findItems(const std::vector<NavigationItem *> &items,
                               Plugin *owner,
                               std::vector<Action *> &actions) const
{
    for (NavigationItem *item : items)
    {
        if (item->type() == NavigationItem::Type::Action &&
            item->owner() == owner && item->action())
        {
            actions.push_back(item->action());
        }

        findItems(item->children(), owner, actions);
    }
}

void NavigationTree::removeItems(Plugin *owner)
{
    if (!owner)
    {
        return;
    }

    // First collect actions because removing items changes the tree.
    std::vector<Action *> actions;

    findItems(items_, owner, actions);

    for (Action *action : actions)
    {
        removeItem(action);
    }
}
