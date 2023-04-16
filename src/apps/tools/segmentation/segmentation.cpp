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
    LOG_PRINT(<< "Open file '" << path << "' in editor");
    Editor editor;
    editor.open(path);

    LOG_PRINT(<< "Compute segmentation");
    SegmentationL1 segmentationL1(&editor);
    segmentationL1.applyParameters(parameters, false);
    while (segmentationL1.next())
        ;

    LOG_PRINT(<< "Debug information");
    const SegmentationL1Context &ctx = segmentationL1.context();
    LOG_PRINT(<< ctx.samples.size() << " sample points");
    for (size_t i = 0; i < ctx.samples.size(); i++)
    {
        LOG_PRINT(<< "point[" << i << "] is " << ctx.samples[i]);
    }
}

int main(int argc, char *argv[])
{
    int rc = 0;

    LOGGER_START_STDOUT;

    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--test", "");
        arg.add("--samples-percent", "");
        arg.add("--descriptor-min", "");
        arg.add("--descriptor-max", "");
        arg.add("--radius-pca", "");
        arg.add("--radius-min", "");
        arg.add("--radius-max", "");
        arg.add("--iterations", "");
        arg.parse(argc, argv);

        SegmentationL1Parameters parameters;

        (void)arg.read("--samples-percent", parameters.numberOfSamples);
        (void)arg.read("--descriptor-min", parameters.sampleDescriptorMinimum);
        (void)arg.read("--descriptor-max", parameters.sampleDescriptorMaximum);
        (void)arg.read("--radius-pca", parameters.neighborhoodRadiusPca);
        (void)arg.read("--radius-min", parameters.neighborhoodRadiusMinimum);
        (void)arg.read("--radius-max", parameters.neighborhoodRadiusMaximum);
        (void)arg.read("--iterations", parameters.numberOfIterations);

        if (arg.contains("--test"))
        {
            createTestDataset(arg.toString("--input"));
        }

        segmentation(arg.toString("--input"), parameters);
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

    LOGGER_STOP_STDOUT;

    return rc;
}
