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

/** @file examplelascreate.cpp @brief Create LAS file. */

#include <FileIndexBuilder.hpp>
#include <Vector3.hpp>
#include <cstring>

static void createBox(const std::string &path)
{
    // Create points
    std::vector<FileLas::Point> points;

    uint32_t nx = 3;
    uint32_t ny = 4;
    uint32_t nz = 5;
    points.resize(nx * ny * nz);

    // Create points: set all attributes to zero
    std::memset(points.data(), 0, sizeof(FileLas::Point) * points.size());

    // Create points: points with resolution 1 point
    for (uint32_t z = 0; z < nz; z++)
    {
        for (uint32_t y = 0; y < ny; y++)
        {
            for (uint32_t x = 0; x < nx; x++)
            {
                uint32_t idx = (x * ny * nz) + (y * nz) + z;
                points[idx].x = x;
                points[idx].y = y;
                points[idx].z = z;
                points[idx].format = 6;
            }
        }
    }

    // Create LAS file
    FileLas::create(path, points);
}

static void createGrid(const std::string &path,
                       const Vector3<double> &translate = {0, 0, 0},
                       uint8_t version_minor = 4)
{
    // Create points
    std::vector<FileLas::Point> points;

    size_t nx = 16;
    size_t ny = 16;
    size_t s1_65535 = 257;
    size_t s2_65535 = 4369;

    if (version_minor < 4)
    {
        nx = 8;
        ny = 8;
        s1_65535 = 1040;
        s2_65535 = 9362;
    }

    points.resize(nx * ny);

    std::memset(points.data(), 0, sizeof(FileLas::Point) * points.size());

    size_t idx = 0;
    for (size_t y = 0; y < ny; y++)
    {
        for (size_t x = 0; x < nx; x++)
        {
            uint16_t intensity = static_cast<uint16_t>(idx * s1_65535);

            if (version_minor < 4)
            {
                points[idx].format = 2; // rgb
            }
            else
            {
                points[idx].format = 7; // rgb
            }

            points[idx].x = static_cast<uint32_t>(x);
            points[idx].y = static_cast<uint32_t>(y);
            points[idx].z = 0;

            points[idx].red = static_cast<uint16_t>(x * s2_65535);
            points[idx].green = static_cast<uint16_t>(y * s2_65535);
            points[idx].blue = 0;

            points[idx].user_red = static_cast<uint16_t>(y * s2_65535);
            points[idx].user_green = 0;
            points[idx].user_blue = static_cast<uint16_t>(x * s2_65535);

            points[idx].intensity = intensity;
            points[idx].return_number = static_cast<uint8_t>(x);
            points[idx].number_of_returns = static_cast<uint8_t>(y);
            points[idx].classification_flags = 0;
            points[idx].scanner_channel = y & 3U;
            points[idx].scan_direction_flag = y & 4U;
            points[idx].edge_of_flight_line = y & 8U;
            if (version_minor < 4)
            {
                points[idx].classification = static_cast<uint8_t>(idx & 0xfU);
            }
            else
            {
                points[idx].classification = static_cast<uint8_t>(idx);
            }
            points[idx].source_id = intensity;

            points[idx].user_layer = static_cast<uint32_t>(x);

            idx++;
        }
    }

    // Create LAS file with scale and offset
    FileLas::create(path, points, {1,1,1}, translate, version_minor);
}

static void createIndex(const std::string &path)
{
    // Create LAS file index
    FileIndexBuilder::Settings settings;
    FileIndexBuilder::index(path, path, settings);
}

int main()
{
    createBox("box.las");
    //createIndex("box.las");

    //createGrid("grid_v2.las", {4,0,0}, 2);
    //createIndex("grid_v2.las");

    createGrid("grid.las", {13,0,0}, 4);
    //createIndex("grid.las");

    return 0;
}
