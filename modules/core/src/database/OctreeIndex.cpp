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

const uint32_t OctreeIndex::CHUNK_ID_OCTREE = 0x4F494458U;

#if 0
OctreeIndex::Cell::Cell(uint64_t code, uint64_t from, uint64_t n, uint8_t inside)
    : code_(code), from_(from), n_(n), inside_(inside)
{
    // empty
}
#endif

OctreeIndex::OctreeIndex() : maxlevel_(1)
{
    // empty
}

OctreeIndex::~OctreeIndex()
{
    // empty
}

void OctreeIndex::setMaxLevel(size_t maxlevel)
{
    maxlevel_ = maxlevel;
    nodeSize_ = 2 + (2 * maxlevel_);
}

void OctreeIndex::addLevel()
{
    size_t n = 8 * nodeSize_;
    size_t s = nodes.size();
    nodes.resize(s + n);
    for (size_t i = 0; i < n; i++)
    {
        nodes[s + i] = 0;
    }
}

void OctreeIndex::setup(const Aabbd &boundary, size_t maxlevel)
{
    setMaxLevel(maxlevel);
    boundary_ = boundary;
    nodes.clear();
    addLevel();
}

uint64_t OctreeIndex::insert(double x, double y, double z)
{
    uint64_t code = 0;
    uint64_t c;
    uint64_t pos = 0;
    uint64_t idx;
    double px;
    double py;
    double pz;
    double x1, y1, z1, x2, y2, z2;
    Aabbd octant = boundary_;

    for (size_t i = 0; i < maxlevel_; i++)
    {
        octant.getCenter(px, py, pz);

        code = code << (i * 3);

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

        code = code | ((static_cast<uint64_t>(i) & 0xff) << 56);

        idx = (pos * nodeSize_) + c;
        nodes[idx + OFFSET_CODE] = code;

        if (i + 1 == maxlevel_)
        {
            nodes[idx + OFFSET_SIZE]++;
        }
        else
        {
            if (nodes[idx + OFFSET_NEXT] == 0)
            {
                nodes[idx + OFFSET_NEXT] = nodes.size();
                addLevel();
            }
            pos = nodes[idx + OFFSET_NEXT];
        }
    }

    return code;
}
#if 0
void OctreeIndex::select(std::vector<Cell> &cells,
                    const Aabbd &window,
                    int maxlevel) const
{
    if (maxlevel == 0)
    {
        maxlevel = maxlevel_;
    }

    select(cells, window, boundary_, 0, 1, maxlevel);
}

void OctreeIndex::select(std::vector<Cell> &cells,
                    const Aabbd &window,
                    const Aabbd &boundary,
                    size_t pos,
                    int level,
                    int maxlevel) const
{
    double px;
    double py;
    double pz;
    Aabbd octant;

    boundary.getCenter(px, py, pz);

    for (size_t i = 0; i < 8; i++)
    {
        const Node &v = nodes_[pos + i];

        if (v.next > 0 || v.n > 0)
        {
            octant = boundary;

            divide(octant, px, py, pz, i);

            if (octant.isInside(window))
            {
                selectall(cells, pos + i, level, maxlevel);
            }
            else if (octant.intersects(window))
            {
                if (v.n > 0)
                {
                    cells.push_back(Cell(v.code, v.from, v.n, 0));
                }

                if (level < maxlevel)
                {
                    select(cells, window, octant, v.next, level + 1, maxlevel);
                }
            }
        }
    }
}

void OctreeIndex::selectall(std::vector<Cell> &cells,
                       size_t pos,
                       int level,
                       int maxlevel) const
{
    if (nodes_[pos].n > 0)
    {
        const Node &v = nodes_[pos];
        cells.push_back(Cell(v.code, v.from, v.n, 1));
    }

    if (nodes_[pos].next > 0 && level < maxlevel)
    {
        for (size_t i = 0; i < 8; i++)
        {
            selectall(cells, nodes_[pos].next + i, level + 1, maxlevel);
        }
    }
}

void OctreeIndex::divide(Aabbd &boundary,
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

void OctreeIndex::setnode(uint64_t code, uint64_t from)
{
    (void)code;
    (void)from;
}
#endif

#if 0
void OctreeIndex::read(ChunkFile &f)
{
    uint8_t buffer[512];

    nodes.resize(ncells);

    for (size_t i = 0; i < ncells; i++)
    {
        f.read(buffer, sizeof(buffer));

        node.code = ltoh64(&buffer[0]);
        node.from[0] = ltoh64(&buffer[8]);
        node.n[0] = ltoh64(&buffer[16]);
        node.next = ltoh64(&buffer[24]);
    }
}

void OctreeIndex::write(ChunkFile &f) const
{
    uint8_t buffer[512];

    for (size_t i = 0; i < nodes.size(); i++)
    {
        htol64(&buffer[0], node.code);
        htol64(&buffer[8], node.from[0]);
        htol64(&buffer[16], node.n[0]);
        htol64(&buffer[24], node.next);

        f.write(buffer, sizeof(buffer));
    }
}
#endif

Json &OctreeIndex::serialize(Json &out) const
{
#if 0
    boundary_.serialize(out["boundary"]);
    out["levels"] = maxlevel_;
    out["nodes_size"] = nodes.size();

    Json &out_nodes = out["nodes"];
    for (size_t i = 0; i < nodes.size(); i++)
    {
        Json &out_node = out_nodes[i];
        out_node["code"] = nodes[i].code;
        out_node["from"] = nodes[i].from[0];
        out_node["n"] = nodes[i].n[0];
        out_node["next"] = nodes[i].next;
    }
#endif
    return out;
}
