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

/** @file NavigationTree.hpp */

#ifndef NAVIGATION_TREE_HPP
#define NAVIGATION_TREE_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <NavigationItem.hpp>
#include <NavigationPathItem.hpp>
#include <Signal.hpp>
class Plugin;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** NavigationTree. */
class EXPORT_UI_COMMON NavigationTree
{
public:
    NavigationTree() = default;
    ~NavigationTree();

    NavigationTree(const NavigationTree &) = delete;
    NavigationTree &operator=(const NavigationTree &) = delete;

    void addItem(Plugin *owner,
                 const std::vector<NavigationPathItem> &path,
                 Action *action,
                 int order = 0);

    void removeItem(Action *action);

    void removeItems(Plugin *owner);

    const std::vector<NavigationItem *> &items() const { return items_; }

    Signal<NavigationItem *> itemAdded;

    // Emitted while item is still valid.
    Signal<NavigationItem *> itemAboutToBeRemoved;

private:
    NavigationItem *findGroup(NavigationItem *parent,
                              const std::string &title) const;

    NavigationItem *findOrCreateGroup(NavigationItem *parent,
                                      const std::string &title,
                                      int order);

    NavigationItem *createGroups(const std::vector<NavigationPathItem> &path);

    NavigationItem *findItem(Action *action) const;

    NavigationItem *findItem(const std::vector<NavigationItem *> &items,
                             Action *action) const;

    void removeNavigationItem(NavigationItem *item);

    void removeEmptyGroups(NavigationItem *group);

    void sortItems(std::vector<NavigationItem *> &items);

    void findItems(const std::vector<NavigationItem *> &items,
                   Plugin *owner,
                   std::vector<Action *> &actions) const;

    std::vector<NavigationItem *> items_;
};

#include <WarningsEnable.hpp>

#endif /* NAVIGATION_TREE_HPP */
