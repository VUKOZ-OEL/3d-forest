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
#include <cstdint>
#include <ostream>
#include <vector>

/** Octree Index. */
class OctreeIndex
{
public:
    static const uint32_t CHUNK_ID_OCTREE;

    enum Offset : size_t
    {
        OFFSET_CODE = 0,
        OFFSET_NEXT = 1,
        OFFSET_FROM = 2,
        OFFSET_SIZE = 3
    };

    /* Node: code, next, fromL0, sizeL0, fromL1, sizeL1, .., code, .. */
    std::vector<uint64_t> nodes;

#if 0
    /** Cell. */
    struct Cell
    {
        uint64_t code_;
        uint64_t from_;
        uint64_t n_;
        uint8_t inside_;

        Cell() = default;
        Cell(uint64_t code, uint64_t from, uint64_t n, uint8_t inside);
    };
    void select(std::vector<Cell> &cells,
                const Aabbd &window,
                int maxlevel = 0) const;
    void setnode(uint64_t code, uint64_t from);
#endif

    OctreeIndex();
    ~OctreeIndex();

    void setup(const Aabbd &boundary, size_t maxlevel);

    uint64_t insert(double x, double y, double z);

    // void read(ChunkFile &f);
    // void write(ChunkFile &f) const;

    size_t getMaxLevel() const { return maxlevel_; }
    size_t getNodeSize() const { return nodeSize_; }

    Json &serialize(Json &out) const;

protected:
    size_t maxlevel_;
    size_t nodeSize_;
    Aabbd boundary_;

    void setMaxLevel(size_t maxlevel);
    void addLevel();

#if 0
    void select(std::vector<Cell> &cells,
                const Aabbd &window,
                const Aabbd &boundary,
                size_t pos,
                int level,
                int maxlevel) const;

    void selectall(std::vector<Cell> &cells,
                   size_t pos,
                   int level,
                   int maxlevel) const;

    void divide(Aabbd &boundary,
                double x,
                double y,
                double z,
                size_t code) const;
#endif
};

#endif /* OCTREE_INDEX_HPP */
