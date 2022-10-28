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

/** @file segmentation.cpp @brief Segmentation tool. */

#include <cstring>

#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <SegmentationThread.hpp>

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

static void saveVoxels(const Editor &editor, const char *path)
{
    const Voxels &voxels = editor.voxels();
    const Layers &layers = editor.layers();

    LOG("number of voxels <" << voxels.size() << ">");

    std::vector<LasFile::Point> points;
    points.resize(voxels.size());
    for (size_t i = 0; i < voxels.size(); i++)
    {
        const Voxel &voxel = voxels.at(i);
        memset(&points[i], 0, sizeof(LasFile::Point));

        points[i].format = 7;

        points[i].x = static_cast<uint32_t>(voxel.meanX_);
        points[i].y = static_cast<uint32_t>(voxel.meanY_);
        points[i].z = static_cast<uint32_t>(voxel.meanZ_);

        points[i].intensity = static_cast<uint16_t>(voxel.descriptor_ * 511.0F);

        Vector3<float> c;
        if (voxel.elementIndex_ > 0 && voxel.elementIndex_ < layers.size())
        {
            c = layers.color(voxel.elementIndex_);
        }
        points[i].red = static_cast<uint16_t>(c[0] * 65535.0F);
        points[i].green = static_cast<uint16_t>(c[1] * 65535.0F);
        points[i].blue = static_cast<uint16_t>(c[2] * 65535.0F);
    }

    LasFile::create(path, points, {0.0001, 0.0001, 0.0001}, {0, 0, 0});
}

static void segmentation(const char *path,
                         int voxelSize,
                         int seedElevationMaximumPercent,
                         int treeHeightMinimumPercent)
{
    // Open the file in editor.
    Editor editor;
    editor.open(path);

    // Compute segmentation.
    SegmentationThread segmentationThread(&editor);
    segmentationThread.create();
    segmentationThread.start(voxelSize,
                             seedElevationMaximumPercent,
                             treeHeightMinimumPercent);
    segmentationThread.wait();

    // Export voxels.
    saveVoxels(editor, "voxels.las");
}

int main(int argc, char *argv[])
{
    const char *path = nullptr;
    int voxelSize = 10;
    int seedElevationMaximumPercent = 5;
    int treeHeightMinimumPercent = 25;

    if (argc > 1)
    {
        path = argv[1];
    }

    if (argc > 2)
    {
        voxelSize = atoi(argv[2]);
    }

    if (argc > 3)
    {
        seedElevationMaximumPercent = atoi(argv[3]);
    }

    if (argc > 4)
    {
        treeHeightMinimumPercent = atoi(argv[4]);
    }

    try
    {
        if (!path)
        {
            path = "dataset.las";
            createTestDataset(path);
        }

        segmentation(path,
                     voxelSize,
                     seedElevationMaximumPercent,
                     treeHeightMinimumPercent);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
