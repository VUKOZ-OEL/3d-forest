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

/** @file FileIndex.cpp */

#include <Endian.hpp>
#include <FileIndex.hpp>
#include <cstring>
#include <queue>

const uint32_t FileIndex::CHUNK_TYPE = 0x38584449U; /**< Signature "IDX8" */
#define OCTREE_INDEX_CHUNK_MAJOR_VERSION 1
#define OCTREE_INDEX_CHUNK_MINOR_VERSION 0
#define OCTREE_INDEX_MAX_LEVEL 17
#define OCTREE_INDEX_HEADER_SIZE_1_0 56

FileIndex::FileIndex()
{
}

FileIndex::~FileIndex()
{
}

void FileIndex::clear()
{
    nodes_.clear();
    boundary_.clear();
    boundaryFile_.clear();
    root_.reset();
}

void FileIndex::translate(const Vector3<double> &v)
{
    boundary_ = boundaryFile_;
    boundary_.translate(v);
}

void FileIndex::selectLeaves(std::vector<Selection> &selection,
                             const Aabb<double> &window,
                             size_t id) const
{
    if (!empty())
    {
        selectLeaves(selection, window, boundary_, 0, id);
    }
}

void FileIndex::selectNodes(std::vector<Selection> &selection,
                            const Aabb<double> &window,
                            size_t id) const
{
    if (!empty())
    {
        selectNodes(selection, window, boundary_, 0, id);
    }
}

const FileIndex::Node *FileIndex::selectNode(
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

const FileIndex::Node *FileIndex::selectLeaf(double x, double y, double z) const
{
    if (size() > 0)
    {
        return selectLeaf(x, y, z, boundary_, 0);
    }

    return nullptr;
}

void FileIndex::selectLeaves(std::vector<Selection> &selection,
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

void FileIndex::selectNodes(std::vector<Selection> &selection,
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

const FileIndex::Node *FileIndex::selectNode(
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

const FileIndex::Node *FileIndex::selectLeaf(double x,
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

void FileIndex::divide(Aabb<double> &boundary,
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

const FileIndex::Node *FileIndex::root() const
{
    return &nodes_[0];
}

const FileIndex::Node *FileIndex::next(const Node *node, size_t idx) const
{
    if (node->next[idx])
    {
        return &nodes_[node->next[idx]];
    }

    return nullptr;
}

const FileIndex::Node *FileIndex::prev(const Node *node) const
{
    if (node == root())
    {
        return nullptr;
    }

    return &nodes_[node->prev - 1];
}

Aabb<double> FileIndex::boundary(const Node *node,
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

void FileIndex::insertBegin(const Aabb<double> &boundary,
                            size_t maxSize,
                            size_t maxLevel,
                            bool insertOnlyToLeaves)
{
    // Initialization
    clear();
    boundary_ = boundary;
    boundaryFile_ = boundary_;
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
        maxSize_ = 0; /**< @todo This value should be used. */
    }
}

void FileIndex::insertEnd(const Aabb<double> &boundary)
{
    if (!boundary.empty())
    {
        boundary_ = boundary;
        boundaryFile_ = boundary_;
    }

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

uint64_t FileIndex::insertEndToLeaves(Node *data,
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

size_t FileIndex::countNodes() const
{
    if (root_)
    {
        return countNodes(root_.get());
    }

    return 0;
}

size_t FileIndex::countNodes(BuildNode *node) const
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

uint64_t FileIndex::insert(double x, double y, double z)
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

        code = code << 3;

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

void FileIndex::read(const std::string &path)
{
    FileChunk file;
    file.open(path, "r");
    read(file);
    file.close();
}

void FileIndex::read(const std::string &path, uint64_t offset)
{
    FileChunk file;
    file.open(path, "r");
    file.seek(offset);
    read(file);
    file.close();
}

void FileIndex::read(FileChunk &file)
{
    // Chunk header
    FileChunk::Chunk chunk;
    file.read(chunk);

    // Chunk payload
    readPayload(file, chunk);
}

void FileIndex::readPayload(FileChunk &file, const FileChunk::Chunk &chunk)
{
    file.validate(chunk,
                  CHUNK_TYPE,
                  OCTREE_INDEX_CHUNK_MAJOR_VERSION,
                  OCTREE_INDEX_CHUNK_MINOR_VERSION);

    std::vector<uint8_t> buffer;
    buffer.resize(chunk.headerLength + chunk.dataLength);
    uint8_t *ptr = buffer.data();

    // Header
    file.read(ptr, chunk.headerLength);

    size_t n = static_cast<size_t>(ltoh64(&ptr[0]));
    double wx1 = ltohd(&ptr[8 + (0 * 8)]);
    double wy1 = ltohd(&ptr[8 + (1 * 8)]);
    double wz1 = ltohd(&ptr[8 + (2 * 8)]);
    double wx2 = ltohd(&ptr[8 + (3 * 8)]);
    double wy2 = ltohd(&ptr[8 + (4 * 8)]);
    double wz2 = ltohd(&ptr[8 + (5 * 8)]);
    boundaryFile_.set(wx1, wy1, wz1, wx2, wy2, wz2);
    boundary_ = boundaryFile_;

    // Data
    nodes_.resize(n);
    std::memset(nodes_.data(), 0, sizeof(Node) * n);

    file.read(ptr, chunk.dataLength);

    for (size_t i = 0; i < nodes_.size(); i++)
    {
        nodes_[i].reserved = ltoh32(ptr);
        nodes_[i].prev = ltoh32(ptr + 4);
        ptr += 8;

        uint32_t c = 0;
        uint32_t nextMask = nodes_[i].reserved & 0xffU;
        for (int b = 0; b < 8; b++)
        {
            if (nextMask & (1U << b))
            {
                nodes_[i].next[b] = ltoh32(ptr);
                ptr += 4;
                c++;
            }
        }
        if (c & 1U)
        {
            ptr += 4;
        }

        nodes_[i].from = ltoh64(ptr);
        nodes_[i].size = ltoh64(ptr + 8);
        nodes_[i].offset = ltoh64(ptr + 16);
        ptr += 24;
    }
}

void FileIndex::write(const std::string &path) const
{
    FileChunk file;
    file.open(path, "w");
    write(file);
    file.close();
}

void FileIndex::write(FileChunk &file) const
{
    // Chunk
    FileChunk::Chunk chunk;
    chunk.type = CHUNK_TYPE;
    chunk.majorVersion = OCTREE_INDEX_CHUNK_MAJOR_VERSION;
    chunk.minorVersion = OCTREE_INDEX_CHUNK_MINOR_VERSION;
    chunk.headerLength = OCTREE_INDEX_HEADER_SIZE_1_0;

    // Chunk size
    chunk.dataLength = 0;
    std::vector<uint32_t> headers;
    headers.resize(nodes_.size());
    for (size_t i = 0; i < nodes_.size(); i++)
    {
        uint32_t c = 0;
        uint32_t nextMask = 0;
        for (int b = 0; b < 8; b++)
        {
            if (nodes_[i].next[b])
            {
                nextMask |= 1U << b;
                c++;
            }
        }
        if (c & 1U)
        {
            c++;
        }
        headers[i] = nextMask;
        chunk.dataLength += c;
    }
    chunk.dataLength *= 4;
    chunk.dataLength += nodes_.size() * 32;

    // Chunk write
    file.write(chunk);

    // Header
    std::vector<uint8_t> buffer;
    buffer.resize(chunk.headerLength + chunk.dataLength);
    uint8_t *ptr = buffer.data();

    htol64(&ptr[0], nodes_.size());
    htold(&ptr[8 + (0 * 8)], boundaryFile_.min(0));
    htold(&ptr[8 + (1 * 8)], boundaryFile_.min(1));
    htold(&ptr[8 + (2 * 8)], boundaryFile_.min(2));
    htold(&ptr[8 + (3 * 8)], boundaryFile_.max(0));
    htold(&ptr[8 + (4 * 8)], boundaryFile_.max(1));
    htold(&ptr[8 + (5 * 8)], boundaryFile_.max(2));
    file.write(buffer.data(), chunk.headerLength);

    // Data
    for (size_t i = 0; i < nodes_.size(); i++)
    {
        htol32(ptr, headers[i]);
        htol32(ptr + 4, nodes_[i].prev);
        ptr += 8;

        uint32_t c = 0;
        for (int b = 0; b < 8; b++)
        {
            if (nodes_[i].next[b])
            {
                htol32(ptr, nodes_[i].next[b]);
                ptr += 4;
                c++;
            }
        }
        if (c & 1U)
        {
            htol32(ptr, 0);
            ptr += 4;
        }

        htol64(ptr, nodes_[i].from);
        htol64(ptr + 8, nodes_[i].size);
        htol64(ptr + 16, nodes_[i].offset);
        ptr += 24;
    }
    file.write(buffer.data(), chunk.dataLength);
}

Json &FileIndex::write(Json &out) const
{
    if (size() > 0)
    {
        return write(out["root"], nodes_.data(), 0);
    }

    return out;
}

Json &FileIndex::write(Json &out, const Node *data, size_t idx) const
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

std::ostream &operator<<(std::ostream &os, const FileIndex &obj)
{
    Json json;
    os << obj.write(json).serialize();
    return os;
}
