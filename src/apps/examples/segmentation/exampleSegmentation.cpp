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

/** @file exampleSegmentation.cpp @brief Segmentation example. */

#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <SegmentationThread.hpp>

#define PATH "dataset.las"

static void exampleSegmentation()
{
    Editor editor;
    editor.open(PATH);

    int voxelSize = 10;
    int threshold = 50;

    SegmentationThread segmentationThread(&editor);
    segmentationThread.create();
    segmentationThread.start(voxelSize, threshold);
    segmentationThread.wait();

    const Voxels &voxels = editor.voxels();
    LOG("size <" << voxels.size() << ">");
    for (size_t i = 0; i < voxels.size(); i++)
    {
        const Voxels::Voxel &voxel = voxels.at(i);
        LOG("i <" << voxel.i << ">");
    }
}

static void add(std::vector<LasFile::Point> *points,
                uint32_t x,
                uint32_t y,
                uint32_t z)
{
    LasFile::Point pt;

    std::memset(&pt, 0, sizeof(LasFile::Point));

    pt.x = x;
    pt.y = y;
    pt.z = z;

    points->push_back(pt);
}

static void createDataset()
{
    // Create a dataset.
    std::vector<LasFile::Point> points;

    add(&points, 0, 0, 0);
    add(&points, 1, 0, 0);
    add(&points, 0, 1, 0);
    add(&points, 1, 1, 0);

    LasFile::create(PATH, points, {1, 1, 1}, {0, 0, 0});

    // Create index file.
    IndexFileBuilder::Settings settings;
    settings.maxSize1 = 2;

    IndexFileBuilder::index(PATH, PATH, settings);
}

int main()
{
    try
    {
        createDataset();
        exampleSegmentation();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
