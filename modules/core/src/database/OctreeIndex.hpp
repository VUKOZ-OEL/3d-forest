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
        uint64_t prev;
        uint64_t next[8];
    };

    /** Octree Index Selection. */
    struct Selection
    {
        size_t idx;
        bool partial;
    };

    OctreeIndex();
    ~OctreeIndex();

    const Aabb<double> &boundary() const { return boundary_; }
    size_t size() const { return nodeSize_; }

    void selectLeaves(std::vector<Selection> &selection,
                      const Aabb<double> &window) const;
    void selectNodes(std::vector<Selection> &selection,
                     const Aabb<double> &window) const;

    const Node *root() const;
    const Node *next(const Node *node, size_t idx) const;
    const Node *prev(const Node *node) const;
    const Node *at(size_t idx) const;

    void read(ChunkFile &file);
    void readPayload(ChunkFile &file, const ChunkFile::Chunk &chunk);
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
    size_t nodeSize_;
    Aabb<double> boundary_;
    std::vector<uint64_t> nodes_; // [{From, Size, Next[8]}, {}, ..]

    void selectLeaves(std::vector<Selection> &idxList,
                      const Aabb<double> &window,
                      const Aabb<double> &boundary,
                      size_t pos) const;

    void selectNodes(std::vector<Selection> &idxList,
                     const Aabb<double> &window,
                     const Aabb<double> &boundary,
                     size_t pos) const;

    void divide(Aabb<double> &boundary,
                double x,
                double y,
                double z,
                size_t code) const;

    Json &write(Json &out, const uint64_t *data, size_t idx) const;

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

    uint64_t insertEndToLeaves(uint64_t *data,
                               BuildNode *node,
                               size_t prev,
                               size_t &idx,
                               uint64_t &from);
    size_t countNodes() const;
    size_t countNodes(BuildNode *node) const;
};

#endif /* OCTREE_INDEX_HPP */
