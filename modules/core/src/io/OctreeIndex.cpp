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
#define OCTREE_INDEX_HEADER_SIZE_1_0 56

OctreeIndex::OctreeIndex()
{
}

OctreeIndex::~OctreeIndex()
{
}

void OctreeIndex::clear()
{
    nodes_.clear();
    boundary_.clear();
    root_.reset();
}

void OctreeIndex::selectLeaves(std::vector<Selection> &selection,
                               const Aabb<double> &window,
                               size_t id) const
{
    if (!empty())
    {
        selectLeaves(selection, window, boundary_, 0, id);
    }
}

void OctreeIndex::selectNodes(std::vector<Selection> &selection,
                              const Aabb<double> &window,
                              size_t id) const
{
    if (!empty())
    {
        selectNodes(selection, window, boundary_, 0, id);
    }
}

const OctreeIndex::Node *OctreeIndex::selectNode(
    std::map<const Node *, uint64_t> &used,
    double x,
    double y,
    double z) const
{
    if (size() > 0)
    {
        return selectNode(used, x, y, z, boundary_, 0);
    }

    return nullptr;
}

const OctreeIndex::Node *OctreeIndex::selectLeaf(double x,
                                                 double y,
                                                 double z) const
{
    if (size() > 0)
    {
        return selectLeaf(x, y, z, boundary_, 0);
    }

    return nullptr;
}

void OctreeIndex::selectLeaves(std::vector<Selection> &selection,
                               const Aabb<double> &window,
                               const Aabb<double> &boundary,
                               size_t idx,
                               size_t id) const
{
    // Select all
    if (boundary.isInside(window))
    {
        selection.push_back({id, idx, false});
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
    const Node *node = &nodes_[idx];
    bool leaf = true;

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            octant = boundary;
            divide(octant, px, py, pz, i);
            selectLeaves(selection, window, octant, node->next[i], id);
            leaf = false;
        }
    }

    // Partial
    if (leaf)
    {
        selection.push_back({id, idx, true});
    }
}

void OctreeIndex::selectNodes(std::vector<Selection> &selection,
                              const Aabb<double> &window,
                              const Aabb<double> &boundary,
                              size_t idx,
                              size_t id) const
{
    // Outside
    if (!boundary.intersects(window))
    {
        return;
    }

    // Select all or partial
    if (boundary.isInside(window))
    {
        selection.push_back({id, idx, false});
    }
    else
    {
        selection.push_back({id, idx, true});
    }

    // Octants
    double px;
    double py;
    double pz;
    Aabb<double> octant;
    const Node *node = &nodes_[idx];

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            octant = boundary;
            divide(octant, px, py, pz, i);
            selectNodes(selection, window, octant, node->next[i], id);
        }
    }
}

const OctreeIndex::Node *OctreeIndex::selectNode(
    std::map<const Node *, uint64_t> &used,
    double x,
    double y,
    double z,
    const Aabb<double> &boundary,
    size_t idx) const
{
    // Outside
    if (!boundary.isInside(x, y, z))
    {
        return nullptr;
    }

    const Node *node = &nodes_[idx];
    if (used[node] < node->size)
    {
        return node;
    }

    // Octants
    double px;
    double py;
    double pz;
    Aabb<double> octant;
    const Node *ret;

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            octant = boundary;
            divide(octant, px, py, pz, i);
            ret = selectNode(used, x, y, z, octant, node->next[i]);
            if (ret)
            {
                return ret;
            }
        }
    }

    // Leaf
    return node;
}

const OctreeIndex::Node *OctreeIndex::selectLeaf(double x,
                                                 double y,
                                                 double z,
                                                 const Aabb<double> &boundary,
                                                 size_t idx) const
{
    // Outside
    if (!boundary.isInside(x, y, z))
    {
        return nullptr;
    }

    // Octants
    double px;
    double py;
    double pz;
    Aabb<double> octant;
    const Node *node = &nodes_[idx];
    const Node *ret;

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            octant = boundary;
            divide(octant, px, py, pz, i);
            ret = selectLeaf(x, y, z, octant, node->next[i]);
            if (ret)
            {
                return ret;
            }
        }
    }

    // Leaf
    return node;
}

void OctreeIndex::divide(Aabb<double> &boundary,
                         double x,
                         double y,
                         double z,
                         uint64_t code) const
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
    return &nodes_[0];
}

const OctreeIndex::Node *OctreeIndex::next(const Node *node, size_t idx) const
{
    if (node->next[idx])
    {
        return &nodes_[node->next[idx]];
    }

    return nullptr;
}

const OctreeIndex::Node *OctreeIndex::prev(const Node *node) const
{
    if (node == root())
    {
        return nullptr;
    }

    return &nodes_[node->prev - 1];
}

Aabb<double> OctreeIndex::boundary(const Node *node,
                                   const Aabb<double> &box) const
{
    // Top
    const Node *data = nodes_.data();
    const Node *prev;
    const Node *next;
    uint64_t code = 0;
    int levels = 0;

    while (node && node->prev)
    {
        levels++;
        prev = node;
        node = &data[node->prev - 1];

        for (size_t i = 0; i < 8; i++)
        {
            next = &data[node->next[i]];
            if (next == prev)
            {
                code = code << 3;
                code = code | static_cast<uint64_t>(i);
            }
        }
    }

    // Down
    Aabb<double> boundary = box;
    double px;
    double py;
    double pz;

    while (levels)
    {
        boundary.getCenter(px, py, pz);
        divide(boundary, px, py, pz, code & 7ULL);
        code = code >> 3;
        levels--;
    }

    return boundary;
}

void OctreeIndex::insertBegin(const Aabb<double> &boundary,
                              size_t maxSize,
                              size_t maxLevel,
                              bool insertOnlyToLeaves)
{
    // Initialization
    clear();
    boundary_ = boundary;
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
        size_t nodes = countNodes();
        nodes_.resize(nodes);

        // Build tree to array
        uint32_t idx = 0;
        uint64_t from = 0;
        Node *data = nodes_.data();

        if (insertOnlyToLeaves_)
        {
            (void)insertEndToLeaves(data, root_.get(), 0, idx, from);
        }
        else
        {
            std::queue<uint32_t> qprev;
            std::queue<BuildNode *> qnode;
            BuildNode *node;
            uint32_t prev;
            uint32_t used = 0;
            qnode.push(root_.get());
            qprev.push(0);

            while (!qnode.empty())
            {
                // Add
                node = qnode.front();
                qnode.pop();
                prev = qprev.front();
                qprev.pop();

                data[idx].from = from;
                data[idx].size = node->size;
                data[idx].prev = prev;

                // Continue
                for (size_t i = 0; i < 8; i++)
                {
                    if (node->next[i])
                    {
                        used++;
                        data[idx].next[i] = used;
                        qnode.push(node->next[i].get());
                        qprev.push(idx + 1);
                    }
                }

                idx++;
                from += node->size;
            }
        }

        // Cleanup
        root_.reset();
    }
}

uint64_t OctreeIndex::insertEndToLeaves(Node *data,
                                        BuildNode *node,
                                        uint32_t prev,
                                        uint32_t &idx,
                                        uint64_t &from)
{
    const uint32_t idxCopy = idx;
    uint64_t n = node->size;

    data[idx].from = from;
    data[idx].prev = prev;

    idx++;
    prev = idx;
    from += n;

    for (size_t i = 0; i < 8; i++)
    {
        if (node->next[i])
        {
            data[idxCopy].next[i] = idx;
            n += insertEndToLeaves(data, node->next[i].get(), prev, idx, from);
        }
    }

    data[idxCopy].size = n;

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

void OctreeIndex::read(const std::string &path)
{
    ChunkFile file;
    file.open(path, "r");
    read(file);
    file.close();
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
    size_t n = static_cast<size_t>(ltoh64(&buffer[0]));
    double wx1 = ltohd(&buffer[8 + (0 * 8)]);
    double wy1 = ltohd(&buffer[8 + (1 * 8)]);
    double wz1 = ltohd(&buffer[8 + (2 * 8)]);
    double wx2 = ltohd(&buffer[8 + (3 * 8)]);
    double wy2 = ltohd(&buffer[8 + (4 * 8)]);
    double wz2 = ltohd(&buffer[8 + (5 * 8)]);
    boundary_.set(wx1, wy1, wz1, wx2, wy2, wz2);

    // Data
    nodes_.resize(n);
    uint8_t *data = reinterpret_cast<uint8_t *>(nodes_.data());
    file.read(data, chunk.dataLength);
    for (size_t i = 0; i < nodes_.size(); i++)
    {
        nodes_[i].from = ltoh64(&data[i * sizeof(Node)]);
        nodes_[i].size = ltoh64(&data[i * sizeof(Node) + 8]);
        nodes_[i].reserved = ltoh32(&data[i * sizeof(Node) + 16]);
        nodes_[i].prev = ltoh32(&data[i * sizeof(Node) + 20]);
        nodes_[i].next[0] = ltoh32(&data[i * sizeof(Node) + 24]);
        nodes_[i].next[1] = ltoh32(&data[i * sizeof(Node) + 28]);
        nodes_[i].next[2] = ltoh32(&data[i * sizeof(Node) + 32]);
        nodes_[i].next[3] = ltoh32(&data[i * sizeof(Node) + 36]);
        nodes_[i].next[4] = ltoh32(&data[i * sizeof(Node) + 40]);
        nodes_[i].next[5] = ltoh32(&data[i * sizeof(Node) + 44]);
        nodes_[i].next[6] = ltoh32(&data[i * sizeof(Node) + 48]);
        nodes_[i].next[7] = ltoh32(&data[i * sizeof(Node) + 52]);
    }
}

void OctreeIndex::write(const std::string &path) const
{
    ChunkFile file;
    file.open(path, "w");
    write(file);
    file.close();
}

void OctreeIndex::write(ChunkFile &file) const
{
    // Chunk
    ChunkFile::Chunk chunk;
    chunk.type = CHUNK_TYPE;
    chunk.majorVersion = OCTREE_INDEX_CHUNK_MAJOR_VERSION;
    chunk.minorVersion = OCTREE_INDEX_CHUNK_MINOR_VERSION;
    chunk.headerLength = OCTREE_INDEX_HEADER_SIZE_1_0;
    chunk.dataLength = nodes_.size() * sizeof(Node);
    file.write(chunk);

    // Header
    uint8_t buffer[64];
    htol64(&buffer[0], nodes_.size());
    htold(&buffer[8 + (0 * 8)], boundary_.min(0));
    htold(&buffer[8 + (1 * 8)], boundary_.min(1));
    htold(&buffer[8 + (2 * 8)], boundary_.min(2));
    htold(&buffer[8 + (3 * 8)], boundary_.max(0));
    htold(&buffer[8 + (4 * 8)], boundary_.max(1));
    htold(&buffer[8 + (5 * 8)], boundary_.max(2));
    file.write(buffer, chunk.headerLength);

    // Data
    std::vector<uint8_t> nodes;
    nodes.resize(nodes_.size() * sizeof(Node));
    uint8_t *data = reinterpret_cast<uint8_t *>(nodes.data());
    for (size_t i = 0; i < nodes_.size(); i++)
    {
        htol64(&data[i * sizeof(Node)], nodes_[i].from);
        htol64(&data[i * sizeof(Node) + 8], nodes_[i].size);
        htol32(&data[i * sizeof(Node) + 16], nodes_[i].reserved);
        htol32(&data[i * sizeof(Node) + 20], nodes_[i].prev);
        htol32(&data[i * sizeof(Node) + 24], nodes_[i].next[0]);
        htol32(&data[i * sizeof(Node) + 28], nodes_[i].next[1]);
        htol32(&data[i * sizeof(Node) + 32], nodes_[i].next[2]);
        htol32(&data[i * sizeof(Node) + 36], nodes_[i].next[3]);
        htol32(&data[i * sizeof(Node) + 40], nodes_[i].next[4]);
        htol32(&data[i * sizeof(Node) + 44], nodes_[i].next[5]);
        htol32(&data[i * sizeof(Node) + 48], nodes_[i].next[6]);
        htol32(&data[i * sizeof(Node) + 52], nodes_[i].next[7]);
    }
    file.write(data, chunk.dataLength);
}

Json &OctreeIndex::write(Json &out) const
{
    if (size() > 0)
    {
        return write(out["root"], nodes_.data(), 0);
    }

    return out;
}

Json &OctreeIndex::write(Json &out, const Node *data, size_t idx) const
{
    out["from"] = data[idx].from;
    out["count"] = data[idx].size;

    size_t used = 0;
    for (size_t i = 0; i < 8; i++)
    {
        if (data[idx].next[i])
        {
            out["nodes"][used]["octant"] = i;
            write(out["nodes"][used], data, data[idx].next[i]);
            used++;
        }
    }

    return out;
}
