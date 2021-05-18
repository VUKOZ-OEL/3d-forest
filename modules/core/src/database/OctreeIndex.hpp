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

/**
    @file OctreeIndex.hpp
*/

#ifndef OCTREE_INDEX_HPP
#define OCTREE_INDEX_HPP

#include <Aabb.hpp>
#include <ChunkFile.hpp>
#include <limits>
#include <map>
#include <vector>

/** Octree Index. */
class OctreeIndex
{
public:
    static const uint32_t CHUNK_TYPE;

    /** Octree Index Node. */
    struct Node
    {
        uint64_t from;
        uint64_t size;
        uint32_t reserved;
        uint32_t prev;
        uint32_t next[8];
    };

    /** Octree Index Selection. */
    struct Selection
    {
        size_t idx;
        bool partial;
    };

    OctreeIndex();
    ~OctreeIndex();

    void clear();

    const Aabb<double> &boundary() const { return boundary_; }
    size_t size() const { return nodes_.size(); }
    bool empty() const { return (nodes_.empty() || nodes_[0].size == 0); }

    // Select
    void selectLeaves(std::vector<Selection> &selection,
                      const Aabb<double> &window) const;

    void selectNodes(std::vector<Selection> &selection,
                     const Aabb<double> &window) const;

    const Node *selectNode(std::map<const Node *, uint64_t> &used,
                           double x,
                           double y,
                           double z) const;

    const Node *selectLeaf(double x, double y, double z) const;

    // Node
    const Node *root() const;
    const Node *next(const Node *node, size_t idx) const;
    const Node *prev(const Node *node) const;
    const Node *at(size_t idx) const { return &nodes_[idx]; }
    Aabb<double> boundary(const Node *node, const Aabb<double> &box) const;

    // IO
    void read(const std::string &path);
    void read(ChunkFile &file);
    void readPayload(ChunkFile &file, const ChunkFile::Chunk &chunk);
    void write(const std::string &path) const;
    void write(ChunkFile &file) const;
    Json &write(Json &out) const;

    // Build tree
    void insertBegin(const Aabb<double> &boundary,
                     size_t maxSize,
                     size_t maxLevel = 0,
                     bool insertOnlyToLeaves = false);
    uint64_t insert(double x, double y, double z);
    void insertEnd();

protected:
    Aabb<double> boundary_;
    std::vector<Node> nodes_;

    void selectLeaves(std::vector<Selection> &idxList,
                      const Aabb<double> &window,
                      const Aabb<double> &boundary,
                      size_t idx) const;

    void selectNodes(std::vector<Selection> &idxList,
                     const Aabb<double> &window,
                     const Aabb<double> &boundary,
                     size_t idx) const;

    const Node *selectNode(std::map<const Node *, uint64_t> &used,
                           double x,
                           double y,
                           double z,
                           const Aabb<double> &boundary,
                           size_t idx) const;

    const Node *selectLeaf(double x,
                           double y,
                           double z,
                           const Aabb<double> &boundary,
                           size_t idx) const;

    void divide(Aabb<double> &boundary,
                double x,
                double y,
                double z,
                uint64_t code) const;

    Json &write(Json &out, const Node *data, size_t idx) const;

    // Build tree
    /** Octree Index Build Node. */
    struct BuildNode
    {
        uint64_t code;
        uint64_t size;
        std::unique_ptr<BuildNode> next[8];
    };

    std::unique_ptr<BuildNode> root_;

    // Build tree settings
    size_t maxSize_;
    size_t maxLevel_;
    bool insertOnlyToLeaves_;

    uint64_t insertEndToLeaves(Node *data,
                               BuildNode *node,
                               uint32_t prev,
                               uint32_t &idx,
                               uint64_t &from);
    size_t countNodes() const;
    size_t countNodes(BuildNode *node) const;
};

#endif /* OCTREE_INDEX_HPP */
