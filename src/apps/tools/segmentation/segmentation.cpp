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
    std::cout << "Open file '" << path << "' in editor" << std::endl;
    Editor editor;
    editor.open(path);

    std::cout << "Compute segmentation" << std::endl;
    SegmentationL1 segmentationL1(&editor);
    segmentationL1.applyParameters(parameters);
    while (segmentationL1.next())
        ;

    std::cout << "Debug information" << std::endl;
    const SegmentationL1Context &ctx = segmentationL1.context();
    std::cout << ctx.numberOfPoints << " total points" << std::endl;
    std::cout << ctx.samples.size() << " sample points" << std::endl;
    for (size_t i = 0; i < ctx.samples.size(); i++)
    {
        std::cout << "point[" << i << "] is " << ctx.samples[i] << std::endl;
    }
}

int main(int argc, char *argv[])
{
    int rc;

    globalLogThread = std::make_shared<LogThread>();

    LoggerStdout logger;
    globalLogThread->setCallback(&logger);

    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--iterations", "");
        arg.add("--count", "");
        arg.add("--test-data", "");
        arg.parse(argc, argv);

        SegmentationL1Parameters parameters;

        (void)arg.read("--count", parameters.numberOfSamples);
        (void)arg.read("--iterations", parameters.numberOfIterations);

        if (arg.contains("--test-data"))
        {
            createTestDataset(arg.toString("--input"));
        }

        segmentation(arg.toString("--input"), parameters);

        rc = 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        rc = 1;
    }
    catch (...)
    {
        std::cerr << "error: unknown" << std::endl;
        rc = 1;
    }

    globalLogThread->setCallback(nullptr);
    globalLogThread->stop();

    return rc;
}
