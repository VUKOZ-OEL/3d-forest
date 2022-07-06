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

static void save(const Voxels &voxels, const char *path)
{
    LOG("number of voxels <" << voxels.size() << ">");

    std::vector<LasFile::Point> points;
    points.resize(voxels.size());
    for (size_t i = 0; i < voxels.size(); i++)
    {
        const Voxels::Voxel &voxel = voxels.at(i);
        memset(&points[i], 0, sizeof(LasFile::Point));
        points[i].format = 6;
        points[i].x = static_cast<uint32_t>(voxel.x);
        points[i].y = static_cast<uint32_t>(voxel.y);
        points[i].z = static_cast<uint32_t>(voxel.z);
        points[i].intensity = static_cast<uint16_t>(voxel.i * 655.35F);
    }

    LasFile::create(path, points, {1, 1, 1}, {0, 0, 0});
}

static void exampleSegmentation(const char *path, int voxelSize, int threshold)
{
    // Open the file in editor.
    Editor editor;
    editor.open(path);

    // Compute segmentation.
    SegmentationThread segmentationThread(&editor);
    segmentationThread.create();
    segmentationThread.start(voxelSize, threshold);
    segmentationThread.wait();

    // Export voxels.
    save(editor.voxels(), "voxels.las");
}

static void appendPoint(std::vector<LasFile::Point> *points,
                        uint32_t x,
                        uint32_t y,
                        uint32_t z)
{
    LasFile::Point pt;

    std::memset(&pt, 0, sizeof(LasFile::Point));

    pt.x = x;
    pt.y = y;
    pt.z = z;
    pt.classification = LasFile::CLASS_UNASSIGNED;

    points->push_back(pt);
}

static void createTestDataset(const char *path)
{
    // Create a dataset.
    std::vector<LasFile::Point> points;

    appendPoint(&points, 0, 0, 0);
    appendPoint(&points, 1, 0, 0);
    appendPoint(&points, 0, 1, 0);
    appendPoint(&points, 1, 1, 0);

    LasFile::create(path, points, {1, 1, 1}, {0, 0, 0});

    // Create index file.
    IndexFileBuilder::Settings settings;
    settings.maxSize1 = 2;

    IndexFileBuilder::index(path, path, settings);
}

int main(int argc, char *argv[])
{
    const char *path = nullptr;
    int voxelSize = 10;
    int threshold = 50;

    if (argc > 1)
    {
        path = argv[1];
    }

    if (argc > 2)
    {
        voxelSize = atoi(argv[2]);
    }

    try
    {
        if (!path)
        {
            path = "dataset.las";
            createTestDataset(path);
        }

        exampleSegmentation(path, voxelSize, threshold);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
