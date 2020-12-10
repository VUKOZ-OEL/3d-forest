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
    @file SpatialIndex.cpp
*/

#include <Endian.hpp>
#include <LasFile.hpp>
#include <OctreeIndex.hpp>
#include <SpatialIndex.hpp>

int SpatialIndex_cmp_point(const void *a, const void *b)
{
    const uint8_t *p1 = static_cast<const uint8_t *>(a);
    const uint8_t *p2 = static_cast<const uint8_t *>(b);

    uint64_t c1 = ltoh64(p1);
    uint64_t c2 = ltoh64(p2);

    if (c1 < c2)
    {
        return -1;
    }

    if (c1 > c2)
    {
        return 1;
    }

    return 0;
}

SpatialIndex::SpatialIndex()
{
}

SpatialIndex::~SpatialIndex()
{
}

void SpatialIndex::create(const std::string &outputPath,
                          const std::string &inputPath,
                          size_t maxlevel)
{
    LasFile las;
    las.open(inputPath);

    // Index
    Aabbd boundary;
    boundary.set(las.header.min_x,
                 las.header.min_y,
                 las.header.min_z,
                 las.header.max_x,
                 las.header.max_y,
                 las.header.max_z);

    OctreeIndex index;
    index.setup(boundary, maxlevel);

    uint64_t code;

    // Temporary file
    const char *TMP_FILENAME_POINTS = "tmp_points.bin";
    File tmp_file;
    tmp_file.open(TMP_FILENAME_POINTS, "w");

    size_t point_size = las.header.point_data_record_length;
    size_t tmp_point_size = sizeof(uint64_t) + point_size;
    uint8_t buffer[128];

    // Create index and write points with octant codes to temporary file
    double x;
    double y;
    double z;

    uint64_t npoints = las.header.number_of_point_records;
    for (uint64_t i = 0; i < npoints; i++)
    {
        las.read(&buffer[8]);
        las.transform(x, y, z, &buffer[8]);

        code = index.insert(x, y, z);

        htol64(&buffer[0], code);
        tmp_file.write(buffer, tmp_point_size);
    }
    tmp_file.close();

    File::sort(TMP_FILENAME_POINTS, tmp_point_size, SpatialIndex_cmp_point);
}

#if 0
    // output
    DatabaseFile output;
    output.open(filename_db, "w");
    output.write_point_header(timestamp,
                              npoints,
                              boundary,
                              point_format,
                              data_length);

    // index
    std::unordered_map<uint64_t, size_t> index_map;
    size_t nnodes = index.nodes_.size();
    for (size_t i = 0; i < nnodes; i++)
    {
        if (index.nodes_[i].n > 0)
        {
            index_map[index.nodes_[i].code] = i;
        }
    }

    tmp_point.open(FILENAME_POINTS, "r");
    uint64_t npoints_node = 0;
    uint64_t npoints_node_from = 0;
    code = 0;
    for (uint64_t i = 0; i < npoints; i++)
    {
        tmp_point.read(buffer, tmp_point_size);
        output.write(&buffer[8], tmp_point_size - 8);
        uint64_t c = ltoh64(&buffer[0]);

        if (i == 0)
        {
            code = c;
        }

        if (c != code || i + 1 == npoints)
        {
            OctreeIndex::Node &node = index.nodes_[index_map[code]];
            node.from = npoints_node_from;

            code = c;
            npoints_node = 1;
            npoints_node_from = i;
        }
        else
        {
            npoints_node++;
        }
    }
    tmp_point.close();

    uint64_t index_data_length;
    index_data_length = nnodes * sizeof(OctreeIndex::Node);
    output.write_otree_header(nnodes, levels, index_data_length);
    output.writeindex(index);
#endif
