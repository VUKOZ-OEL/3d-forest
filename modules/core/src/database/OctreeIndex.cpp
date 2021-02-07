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
    @file OctreeIndex.cpp
*/

#include <Endian.hpp>
#include <OctreeIndex.hpp>
#include <cstring>
#include <queue>

const uint32_t OctreeIndex::CHUNK_TYPE = 0x38584449U; //! Signature "IDX8"
#define OCTREE_INDEX_CHUNK_MAJOR_VERSION 1
#define OCTREE_INDEX_CHUNK_MINOR_VERSION 0
#define OCTREE_INDEX_MAX_LEVEL 17
#define OCTREE_INDEX_NODE_SIZE 11
#define OCTREE_INDEX_OFFSET_FROM 0
#define OCTREE_INDEX_OFFSET_SIZE 1
#define OCTREE_INDEX_OFFSET_PREV 2
#define OCTREE_INDEX_OFFSET_NEXT 3
#define OCTREE_INDEX_HEADER_SIZE_1_0 56

OctreeIndex::OctreeIndex() : nodeSize_(0)
{
}

OctreeIndex::~OctreeIndex()
{
}

void OctreeIndex::selectLeaves(std::vector<Selection> &selection,
                               const Aabb<double> &window) const
{
    // Not empty
    if ((nodeSize_ > 0) && (nodes_[OCTREE_INDEX_OFFSET_SIZE] > 0))
    {
        selectLeaves(selection, window, boundary_, 0);
    }
}

void OctreeIndex::selectNodes(std::vector<Selection> &selection,
                              const Aabb<double> &window) const
{
    // Not empty
    if ((nodeSize_ > 0) && (nodes_[OCTREE_INDEX_OFFSET_SIZE] > 0))
    {
        selectNodes(selection, window, boundary_, 0);
    }
}

void OctreeIndex::selectLeaves(std::vector<Selection> &selection,
                               const Aabb<double> &window,
                               const Aabb<double> &boundary,
                               size_t pos) const
{
    const Node *node = reinterpret_cast<const Node *>(&nodes_[pos]);

    // Select all
    if (boundary.isInside(window))
    {
        size_t id = pos / OCTREE_INDEX_NODE_SIZE;
        selection.push_back({id, false});
        return;
    }

    // Outside
    if (!boundary.intersects(window))
    {
        return;
    }

    // Octants
    double px;
    double py;
    double pz;
    Aabb<double> octant;
    bool leaf = true;

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            octant = boundary;
            divide(octant, px, py, pz, i);
            selectLeaves(selection, window, octant, node->next[i]);
            leaf = false;
        }
    }

    // Partial
    if (leaf)
    {
        size_t id = pos / OCTREE_INDEX_NODE_SIZE;
        selection.push_back({id, true});
    }
}

void OctreeIndex::selectNodes(std::vector<Selection> &selection,
                              const Aabb<double> &window,
                              const Aabb<double> &boundary,
                              size_t pos) const
{
    const Node *node = reinterpret_cast<const Node *>(&nodes_[pos]);

    // Outside
    if (!boundary.intersects(window))
    {
        return;
    }

    // Select all or partial
    size_t id = pos / OCTREE_INDEX_NODE_SIZE;
    if (boundary.isInside(window))
    {
        selection.push_back({id, true});
    }
    else
    {
        selection.push_back({id, false});
    }

    // Octants
    double px;
    double py;
    double pz;
    Aabb<double> octant;

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            octant = boundary;
            divide(octant, px, py, pz, i);
            selectNodes(selection, window, octant, node->next[i]);
        }
    }
}

void OctreeIndex::divide(Aabb<double> &boundary,
                         double x,
                         double y,
                         double z,
                         size_t code) const
{
    double x1, y1, z1, x2, y2, z2;

    if (code & 1)
    {
        x1 = x;
        x2 = boundary.max(0);
    }
    else
    {
        x1 = boundary.min(0);
        x2 = x;
    }

    if (code & 2)
    {
        y1 = y;
        y2 = boundary.max(1);
    }
    else
    {
        y1 = boundary.min(1);
        y2 = y;
    }

    if (code & 4)
    {
        z1 = z;
        z2 = boundary.max(2);
    }
    else
    {
        z1 = boundary.min(2);
        z2 = z;
    }

    boundary.set(x1, y1, z1, x2, y2, z2);
}

const OctreeIndex::Node *OctreeIndex::root() const
{
    return reinterpret_cast<const Node *>(&nodes_[0]);
}

const OctreeIndex::Node *OctreeIndex::next(const Node *node, size_t idx) const
{
    const uint64_t ptr = node->next[idx];
    if (ptr)
    {
        return reinterpret_cast<const Node *>(&nodes_[ptr]);
    }

    return nullptr;
}

const OctreeIndex::Node *OctreeIndex::prev(const Node *node) const
{
    if (node == root())
    {
        return nullptr;
    }

    return reinterpret_cast<const Node *>(&nodes_[node->prev]);
}

const OctreeIndex::Node *OctreeIndex::at(size_t idx) const
{
    const size_t ptr = idx * OCTREE_INDEX_NODE_SIZE;
    return reinterpret_cast<const Node *>(&nodes_[ptr]);
}

void OctreeIndex::insertBegin(const Aabb<double> &boundary,
                              size_t maxSize,
                              size_t maxLevel,
                              bool insertOnlyToLeaves)
{
    // Initialization
    nodeSize_ = 0;
    boundary_ = boundary;
    nodes_.clear();
    root_ = std::make_unique<BuildNode>();

    // Build tree settings
    maxSize_ = maxSize;
    maxLevel_ = maxLevel;
    insertOnlyToLeaves_ = insertOnlyToLeaves;

    if ((maxLevel_ == 0) || (maxLevel_ > OCTREE_INDEX_MAX_LEVEL))
    {
        maxLevel_ = OCTREE_INDEX_MAX_LEVEL;
    }

    if (insertOnlyToLeaves_)
    {
        maxSize_ = 0; // TBD is to use this value
    }
}

void OctreeIndex::insertEnd()
{
    if (root_)
    {
        // Create 1d array tree representation
        nodeSize_ = countNodes();
        nodes_.resize(nodeSize_ * OCTREE_INDEX_NODE_SIZE);

        // Build tree to array
        size_t idx = 0;
        uint64_t from = 0;
        uint64_t *data = nodes_.data();

        if (insertOnlyToLeaves_)
        {
            (void)insertEndToLeaves(data, root_.get(), 0, idx, from);
        }
        else
        {
            std::queue<size_t> qprev;
            std::queue<BuildNode *> qnode;
            BuildNode *node;
            size_t prev;
            size_t used = 0;
            qnode.push(root_.get());
            qprev.push(0);
            while (!qnode.empty())
            {
                // Add
                node = qnode.front();
                qnode.pop();
                prev = qprev.front();
                qprev.pop();

                data[idx + OCTREE_INDEX_OFFSET_FROM] = from;
                data[idx + OCTREE_INDEX_OFFSET_SIZE] = node->size;
                data[idx + OCTREE_INDEX_OFFSET_PREV] = prev;

                // Continue
                for (size_t i = 0; i < 8; i++)
                {
                    if (node->next[i])
                    {
                        used += OCTREE_INDEX_NODE_SIZE;
                        data[idx + OCTREE_INDEX_OFFSET_NEXT + i] = used;
                        qnode.push(node->next[i].get());
                        qprev.push(idx);
                    }
                }

                idx += OCTREE_INDEX_NODE_SIZE;
                from += node->size;
            }
        }

        // Cleanup
        root_.reset();
    }
}

uint64_t OctreeIndex::insertEndToLeaves(uint64_t *data,
                                        BuildNode *node,
                                        size_t prev,
                                        size_t &idx,
                                        uint64_t &from)
{
    const size_t next = idx + OCTREE_INDEX_OFFSET_NEXT;
    const size_t idxCopy = idx;
    uint64_t n = node->size;

    data[idx + OCTREE_INDEX_OFFSET_FROM] = from;
    data[idx + OCTREE_INDEX_OFFSET_PREV] = prev;

    prev = idx;
    idx += OCTREE_INDEX_NODE_SIZE;
    from += n;

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            data[next + i] = idx;
            n += insertEndToLeaves(data, node->next[i].get(), prev, idx, from);
        }
    }

    data[idxCopy + OCTREE_INDEX_OFFSET_SIZE] = n;

    return n;
}

size_t OctreeIndex::countNodes() const
{
    if (root_)
    {
        return countNodes(root_.get());
    }

    return 0;
}

size_t OctreeIndex::countNodes(BuildNode *node) const
{
    size_t n = 1;

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            n += countNodes(node->next[i].get());
        }
    }

    return n;
}

uint64_t OctreeIndex::insert(double x, double y, double z)
{
    uint64_t code = 0;
    uint64_t ecode = 0;
    uint64_t c;
    double px;
    double py;
    double pz;
    double x1, y1, z1, x2, y2, z2;
    Aabb<double> octant = boundary_;
    BuildNode *node = root_.get();

    for (size_t level = 0; level < maxLevel_; level++)
    {
        if (node->size < maxSize_)
        {
            node->size++;
            return ecode;
        }

        octant.getCenter(px, py, pz);

        code = code << (level * 3);

        if (x > px)
        {
            code |= 1;
            x1 = px;
            x2 = octant.max(0);
        }
        else
        {
            x1 = octant.min(0);
            x2 = px;
        }

        if (y > py)
        {
            code |= 2;
            y1 = py;
            y2 = octant.max(1);
        }
        else
        {
            y1 = octant.min(1);
            y2 = py;
        }

        if (z > pz)
        {
            code |= 4;
            z1 = pz;
            z2 = octant.max(2);
        }
        else
        {
            z1 = octant.min(2);
            z2 = pz;
        }

        octant.set(x1, y1, z1, x2, y2, z2);

        c = code & 7;

        if (!insertOnlyToLeaves_)
        {
            ecode = code | ((static_cast<uint64_t>(level + 1) & 0xff) << 56);
        }
        else
        {
            ecode = code;
        }

        if (level + 1 == maxLevel_)
        {
            node->size++;
        }
        else
        {
            if (!node->next[c])
            {
                node->next[c] = std::make_unique<BuildNode>();
                node->next[c]->code = ecode;
            }
            node = node->next[c].get();
        }
    }

    return ecode;
}

void OctreeIndex::read(ChunkFile &file)
{
    // Chunk header
    ChunkFile::Chunk chunk;
    file.read(chunk);

    // Chunk payload
    readPayload(file, chunk);
}

void OctreeIndex::readPayload(ChunkFile &file, const ChunkFile::Chunk &chunk)
{
    file.validate(chunk,
                  CHUNK_TYPE,
                  OCTREE_INDEX_CHUNK_MAJOR_VERSION,
                  OCTREE_INDEX_CHUNK_MINOR_VERSION);

    // Header
    uint8_t buffer[64];
    file.read(buffer, chunk.headerLength);
    nodeSize_ = ltoh64(&buffer[0]);
    double wx1 = ltohd(&buffer[8 + (0 * 8)]);
    double wy1 = ltohd(&buffer[8 + (1 * 8)]);
    double wz1 = ltohd(&buffer[8 + (2 * 8)]);
    double wx2 = ltohd(&buffer[8 + (3 * 8)]);
    double wy2 = ltohd(&buffer[8 + (4 * 8)]);
    double wz2 = ltohd(&buffer[8 + (5 * 8)]);
    boundary_.set(wx1, wy1, wz1, wx2, wy2, wz2);

    // Data
    nodes_.resize(nodeSize_ * OCTREE_INDEX_NODE_SIZE);
    uint8_t *data = reinterpret_cast<uint8_t *>(nodes_.data());
    file.read(data, chunk.dataLength);
    for (size_t i = 0; i < nodes_.size(); i++)
    {
        nodes_[i] = ltoh64(&data[i * 8]);
    }
}

void OctreeIndex::write(ChunkFile &file) const
{
    // Chunk
    ChunkFile::Chunk chunk;
    chunk.type = CHUNK_TYPE;
    chunk.majorVersion = OCTREE_INDEX_CHUNK_MAJOR_VERSION;
    chunk.minorVersion = OCTREE_INDEX_CHUNK_MINOR_VERSION;
    chunk.headerLength = OCTREE_INDEX_HEADER_SIZE_1_0;
    chunk.dataLength = nodes_.size() * sizeof(uint64_t);
    file.write(chunk);

    // Header
    uint8_t buffer[64];
    htol64(&buffer[0], nodeSize_);
    htold(&buffer[8 + (0 * 8)], boundary_.min(0));
    htold(&buffer[8 + (1 * 8)], boundary_.min(1));
    htold(&buffer[8 + (2 * 8)], boundary_.min(2));
    htold(&buffer[8 + (3 * 8)], boundary_.max(0));
    htold(&buffer[8 + (4 * 8)], boundary_.max(1));
    htold(&buffer[8 + (5 * 8)], boundary_.max(2));
    file.write(buffer, chunk.headerLength);

    // Data
    std::vector<uint64_t> nodes;
    size_t n = nodes_.size();
    nodes.resize(n);
    uint8_t *data = reinterpret_cast<uint8_t *>(nodes.data());
    for (size_t i = 0; i < n; i++)
    {
        htol64(&data[i * 8], nodes_[i]);
    }
    file.write(data, chunk.dataLength);
}

Json &OctreeIndex::write(Json &out) const
{
    if (nodes_.size() >= OCTREE_INDEX_NODE_SIZE)
    {
        return write(out["root"], nodes_.data(), 0);
    }
    return out;
}

Json &OctreeIndex::write(Json &out, const uint64_t *data, size_t idx) const
{
    out["from"] = data[idx + OCTREE_INDEX_OFFSET_FROM];
    out["count"] = data[idx + OCTREE_INDEX_OFFSET_SIZE];

    const size_t next = idx + OCTREE_INDEX_OFFSET_NEXT;
    size_t used = 0;
    for (size_t i = 0; i < 8; i++)
    {
        if (data[next + i])
        {
            out["nodes"][used]["octant"] = i;
            write(out["nodes"][used], data, data[next + i]);
            used++;
        }
    }

    return out;
}
