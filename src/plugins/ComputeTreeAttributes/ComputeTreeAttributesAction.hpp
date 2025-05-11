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

/** @file ComputeTreeAttributesAction.hpp */

#ifndef COMPUTE_TREE_ATTRIBUTES_ACTION_HPP
#define COMPUTE_TREE_ATTRIBUTES_ACTION_HPP

// Include 3D Forest.
#include <ComputeTreeAttributesData.hpp>
#include <ComputeTreeAttributesParameters.hpp>
#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;
class Segment;

/** Compute Tree Attributes Action. */
class ComputeTreeAttributesAction : public ProgressActionInterface
{
public:
    ComputeTreeAttributesAction(Editor *editor);
    virtual ~ComputeTreeAttributesAction();

    void start(const ComputeTreeAttributesParameters &parameters);
    virtual void next();
    void clear();

private:
    Editor *editor_;
    Query query_;

    ComputeTreeAttributesParameters parameters_;

    std::map<size_t, size_t> treesMap_; // [tree ID : tree index]
    std::vector<ComputeTreeAttributesData> trees_;

    size_t currentTreeIndex_;

    void stepPointsToTrees();
    void stepCalculateComputeTreeAttributes();
    void stepUpdateComputeTreeAttributes();

    void calculateDbh(ComputeTreeAttributesData &tree);
    void calculateDbhRht(ComputeTreeAttributesData &tree);
    void calculateDbhLsr(ComputeTreeAttributesData &tree);

    void calculateTreePosition(ComputeTreeAttributesData &tree);

    void validateAttributes(TreeAttributes &treeAttributes);

    size_t treeIndex(size_t treeId);
};

#endif /* COMPUTE_TREE_ATTRIBUTES_ACTION_HPP */
