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

/** @file exampleLasCreate.cpp @brief Create LAS file. */

#include <FileIndexBuilder.hpp>
#include <cstring>

static void createBox(const std::string &path)
{
    // Create points
    std::vector<FileLas::Point> points;

    uint32_t n1 = 3;
    uint32_t n2 = 4;
    uint32_t n3 = 10;
    points.resize(n1 * n2 * n3);

    // Create points: set all attributes to zero
    std::memset(points.data(), 0, sizeof(FileLas::Point) * points.size());

    // Create points: points with resolution 1 point
    for (uint32_t i1 = 0; i1 < n1; i1++)
    {
        for (uint32_t i2 = 0; i2 < n2; i2++)
        {
            for (uint32_t i3 = 0; i3 < n3; i3++)
            {
                uint32_t idx = (i1 * n2 * n3) + (i2 * n3) + i3;
                points[idx].x = i1;
                points[idx].y = i2;
                points[idx].z = i3;
                points[idx].format = 0; // Point format 0
            }
        }
    }

    // Create LAS file
    FileLas::create(path, points);
}

static void createGrid(const std::string &path)
{
    // Create points
    std::vector<FileLas::Point> points;

    uint32_t n1 = 15;
    uint32_t n2 = 15;

    points.resize(n1 * n2);

    // Create points: set all attributes to zero
    std::memset(points.data(), 0, sizeof(FileLas::Point) * points.size());

    // Create points: points with resolution 1 point
    for (uint32_t i1 = 0; i1 < n1; i1++)
    {
        for (uint32_t i2 = 0; i2 < n2; i2++)
        {
            uint32_t idx = (i1 * n2) + i2;
            points[idx].x = i1;
            points[idx].y = i2;
            points[idx].z = 0;
            points[idx].format = 0; // Point format 0
        }
    }

    // Create LAS file with scale and offset
    FileLas::create(path, points, {1,1,1}, {10,0,0});
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
    createIndex("box.las");

    createGrid("grid.las");
    createIndex("grid.las");

    return 0;
}
