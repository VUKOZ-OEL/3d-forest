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

/** @file TreeAttributesAction.hpp */

#ifndef TREE_ATTRIBUTES_ACTION_HPP
#define TREE_ATTRIBUTES_ACTION_HPP

// Include 3D Forest.
#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
#include <TreeAttributesData.hpp>
#include <TreeAttributesParameters.hpp>
class Editor;
class Segment;

/** Tree Attributes Action. */
class TreeAttributesAction : public ProgressActionInterface
{
public:
    TreeAttributesAction(Editor *editor);
    virtual ~TreeAttributesAction();

    void start(const TreeAttributesParameters &parameters);
    virtual void next();
    void clear();

private:
    Editor *editor_;
    Query query_;

    TreeAttributesParameters parameters_;

    std::map<size_t, size_t> treesMap_; // [tree ID : tree index]
    std::vector<TreeAttributesData> trees_;

    size_t currentTreeIndex_;

    void stepPointsToTrees();
    void stepCalculateTreeAttributes();
    void stepUpdateTreeAttributes();

    void calculateDbh(TreeAttributesData &tree);
    void calculateTreePosition(TreeAttributesData &tree);
    void calculateTreeHeight(TreeAttributesData &tree);

    size_t treeIndex(size_t treeId);
};

#endif /* TREE_ATTRIBUTES_ACTION_HPP */
