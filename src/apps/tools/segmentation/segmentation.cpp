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

#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <SegmentationThread.hpp>

#include <WarningsOff.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <WarningsOn.h>

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

static void createTestDataset(const std::string &path)
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

static void saveVoxels(const Editor &editor, const std::string &path)
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

static void saveMap(const SegmentationThread &st, const std::string &path)
{
    int w;
    int h;
    int components;
    int rowBytes;
    std::vector<unsigned char> image;

    st.segmentationMap().toImage(&w, &h, &components, &rowBytes, &image);

    stbi_write_png(path.c_str(), w, h, components, image.data(), rowBytes);
}

static void segmentation(const std::string &path,
                         int voxelSize,
                         int seedElevationMinimumPercent,
                         int seedElevationMaximumPercent,
                         int treeHeightMinimumPercent,
                         int searchRadius,
                         int neighborPoints,
                         const std::string &outputVoxels,
                         const std::string &outputMap)
{
    // Open the file in editor.
    Editor editor;
    editor.open(path);

    // Compute segmentation.
    SegmentationThread segmentationThread(&editor);
    segmentationThread.create();
    segmentationThread.start(voxelSize,
                             seedElevationMinimumPercent,
                             seedElevationMaximumPercent,
                             treeHeightMinimumPercent,
                             searchRadius,
                             neighborPoints);
    segmentationThread.wait();

    // Optional export.
    if (!outputVoxels.empty())
    {
        saveVoxels(editor, outputVoxels);
    }

    if (!outputMap.empty())
    {
        saveMap(segmentationThread, outputMap);
    }
}

int main(int argc, char *argv[])
{
    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--test-data", "");
        arg.add("--voxel-size", "10");
        arg.add("--min", "1");
        arg.add("--max", "5");
        arg.add("--height", "10");
        arg.add("--radius", "1000");
        arg.add("--neighbors", "10");
        arg.add("--output-voxels", "");
        arg.add("--output-map", "");
        arg.parse(argc, argv);

        if (arg.contains("--test-data"))
        {
            createTestDataset(arg.toString("--input"));
        }

        segmentation(arg.toString("--input"),
                     arg.toInt("--voxel-size"),
                     arg.toInt("--min"),
                     arg.toInt("--max"),
                     arg.toInt("--height"),
                     arg.toInt("--radius"),
                     arg.toInt("--neighbors"),
                     arg.toString("--output-voxels"),
                     arg.toString("--output-map"));
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
