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

/** @file exampleeditor.cpp @brief Editor example. */

#include <EditorBase.hpp>
#include <FileIndexBuilder.hpp>
#include <cstring>

static void createDataSet(const std::string &path,
                          const Vector3<double> &translate = {0, 0, 0})
{
    // Create points
    std::vector<FileLas::Point> points;

    uint32_t nx = 2;
    uint32_t ny = 3;
    uint32_t nz = 4;
    points.resize(nx * ny * nz);

    std::memset(points.data(), 0, sizeof(FileLas::Point) * points.size());

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
                points[idx].format = 0;
            }
        }
    }

    // Create LAS file
    FileLas::create(path, points, {1,1,1}, translate);

    // Create LAS file index
    FileIndexBuilder::Settings settings;
    FileIndexBuilder::index(path, path, settings);
}

static void edit(const std::string &path)
{
    EditorBase editor;
    editor.addFile(path);

}

int main()
{
    try
    {
        std::string path = "dataset.las";

        createDataSet(path, {4,0,0});
        edit(path);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
