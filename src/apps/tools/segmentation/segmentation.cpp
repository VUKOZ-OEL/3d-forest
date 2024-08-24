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

/** @file segmentation.cpp
    @brief Tree segmentation command line tool.
*/

// Include std.
#include <cstring>

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <Error.hpp>
#include <SegmentationAction.hpp>

// Include local.
#define LOG_MODULE_NAME "segmentation"
#include <Log.hpp>

static void segmentationCompute(const std::string &inputPath,
                                const SegmentationParameters &parameters)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Repeatedly call tree segmentation until it is complete.
    SegmentationAction segmentation(&editor);
    segmentation.start(parameters);
    while (!segmentation.end())
    {
        segmentation.next();
    }

    editor.saveProject(editor.projectPath());
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_segmentation.txt");

    try
    {
        SegmentationParameters p;

        ArgumentParser arg("computes point segmentation to trees");
        arg.add("-f",
                "--file",
                "",
                "Path to the input file to be processed. Accepted formats "
                "include .las, and .json project file",
                true);
        arg.add("-v", "--voxel", toString(p.voxelRadius), "Voxel radius [m]");
        arg.add("-w",
                "--wood",
                toString(p.woodThresholdMin),
                "Minimal leaf-to-wood threshold value [%]");
        arg.add("-c",
                "--wood-channel",
                "descriptor",
                "Leaf-to-wood channel {descriptor,intensity}");
        arg.add("-t",
                "--trunk-search-radius",
                toString(p.searchRadiusTrunkPoints),
                "Maximal distance to connect trunk points [m]");
        arg.add("-l",
                "--leaf-search-radius",
                toString(p.searchRadiusLeafPoints),
                "Maximal distance to connect leaf points [m]");
        arg.add("-e",
                "--elevation-min",
                toString(p.treeBaseElevationMin),
                "Look for tree base in elevation range - minimum [m]");
        arg.add("-E",
                "--elevation-max",
                toString(p.treeBaseElevationMax),
                "Look for tree base in elevation range - maximum [m]");
        arg.add("-H",
                "--tree-height-min",
                toString(p.treeHeightMin),
                "Minimal height of tree [m]");
        arg.add("-z",
                "--z-elevation",
                toString(p.zCoordinatesAsElevation),
                "Use z-coordinates instead of ground elevation {true, false}");
        arg.add("-s",
                "--trunks",
                toString(p.segmentOnlyTrunks),
                "Segment only trunks (fast preview) {true, false}");

        if (arg.parse(argc, argv))
        {
            if (arg.toString("--wood-channel") == "descriptor")
            {
                p.leafToWoodChannel =
                    SegmentationParameters::CHANNEL_DESCRIPTOR;
            }
            else if (arg.toString("--wood-channel") == "intensity")
            {
                p.leafToWoodChannel = SegmentationParameters::CHANNEL_INTENSITY;
            }
            else
            {
                THROW("Invalid descriptor channel option. "
                      "Try '--help' for more information.");
            }

            p.voxelRadius = arg.toDouble("--voxel");
            p.woodThresholdMin = arg.toDouble("--wood");
            p.searchRadiusTrunkPoints = arg.toDouble("--trunk-search-radius");
            p.searchRadiusLeafPoints = arg.toDouble("--leaf-search-radius");
            p.treeBaseElevationMin = arg.toDouble("--elevation-min");
            p.treeBaseElevationMax = arg.toDouble("--elevation-max");
            p.treeHeightMin = arg.toDouble("--tree-height-min");
            p.zCoordinatesAsElevation = arg.toBool("--z-elevation");
            p.segmentOnlyTrunks = arg.toBool("--trunks");

            segmentationCompute(arg.toString("--file"), p);
        }

        rc = 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    LOGGER_STOP_FILE;

    return rc;
}
