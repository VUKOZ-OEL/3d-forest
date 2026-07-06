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

/** @file TreeTableSetManagementStatus.hpp */

#ifndef TREE_TABLE_ACTIONTUS_HPP
#define TREE_TABLE_ACTIONTUS_HPP

// Include Std.
#include <cstdint>
#include <unordered_set>

// Include 3D Forest.
class Application;

/** Tree Table Action. */
class TreeTableAction
{
public:
    static void showTrees(Application *app,
                          std::unordered_set<size_t> idList);

    static void hideTrees(Application *app,
                          std::unordered_set<size_t> idList);

    static void readMesh(Application *app,
                         const std::unordered_set<size_t> &idList,
                         const std::string &meshName);

    static void deleteMesh(Application *app,
                           const std::unordered_set<size_t> &idList,
                           const std::string &meshName);
};

#endif /* TREE_TABLE_ACTION_HPP */
