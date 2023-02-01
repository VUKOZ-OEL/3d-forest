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
#include <SegmentationL1.hpp>

#define LOG_MODULE_NAME "SegmentationTool"
#include <Log.hpp>

static void appendPoint(std::vector<LasFile::Point> *points,
                        int32_t x,
                        int32_t y,
                        int32_t z)
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

static void segmentation(const std::string &path,
                         const SegmentationL1Parameters &parameters)
{
    // Open the input file in editor.
    Editor editor;
    editor.open(path);

    // Compute segmentation.
    SegmentationL1 segmentationL1(&editor);
    segmentationL1.applyParameters(parameters);
    while (segmentationL1.step())
        ;
}

int main(int argc, char *argv[])
{
    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--test-data", "");
        arg.parse(argc, argv);

        SegmentationL1Parameters parameters;

        if (arg.contains("--test-data"))
        {
            createTestDataset(arg.toString("--input"));
        }

        segmentation(arg.toString("--input"), parameters);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
