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
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_segmentation.txt");

    try
    {
        SegmentationParameters parameters;

        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--voxel", toString(parameters.voxelSize));
        arg.add("--descriptor", toString(parameters.descriptor));
        arg.add("--trunk-radius", toString(parameters.trunkRadius));
        arg.add("--leaf-radius", toString(parameters.leafRadius));
        arg.add("--elevation-min", toString(parameters.elevationMin));
        arg.add("--elevation-max", toString(parameters.elevationMax));
        arg.add("--tree-height", toString(parameters.treeHeight));
        arg.add("--z", toString(parameters.zCoordinatesAsElevation));
        arg.add("--trunks", toString(parameters.segmentOnlyTrunks));
        arg.parse(argc, argv);

        parameters.voxelSize = arg.toDouble("--voxel");
        parameters.descriptor = arg.toDouble("--descriptor");
        parameters.trunkRadius = arg.toDouble("--trunk-radius");
        parameters.leafRadius = arg.toDouble("--leaf-radius");
        parameters.elevationMin = arg.toDouble("--elevation-min");
        parameters.elevationMax = arg.toDouble("--elevation-max");
        parameters.treeHeight = arg.toDouble("--tree-height");
        parameters.zCoordinatesAsElevation = arg.toBool("--z");
        parameters.segmentOnlyTrunks = arg.toBool("--trunks");

        segmentationCompute(arg.toString("--input"), parameters);

        rc = 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    LOGGER_STOP_FILE;

    return rc;
}
