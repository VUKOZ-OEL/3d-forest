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

/** @file elevation.cpp  @brief Elevation tool.*/

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <ElevationAction.hpp>
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "elevation"
#include <Log.hpp>

static void elevationCompute(const std::string &inputPath, double voxelSize)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Compute elevation by steps.
    ElevationAction elevation(&editor);
    elevation.start(voxelSize);
    while (!elevation.end())
    {
        elevation.next();
    }
}

static void elevationPrint(const std::string &inputPath)
{
    Editor editor;
    editor.open(inputPath);

    uint64_t nPoints = 0;
    double elevationMinimum = 0;
    double elevationMaximum = 0;

    Query query(&editor);
    query.where().setBox(editor.clipBoundary());
    query.exec();
    while (query.next())
    {
        double elevation = query.elevation();

        if (nPoints == 0)
        {
            elevationMinimum = elevation;
            elevationMaximum = elevation;
        }
        else
        {
            updateRange(elevation, elevationMinimum, elevationMaximum);
        }

        nPoints++;
    }

    std::cout << "number of points  : " << nPoints << std::endl;
    std::cout << "minimum elevation : " << elevationMinimum << std::endl;
    std::cout << "maximum elevation : " << elevationMaximum << std::endl;
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_elevation.txt");

    try
    {
        ArgumentParser arg("compute elevation of points above ground");
        arg.add("-i",
                "--input",
                "",
                "Path to the input file to be processed. Accepted formats "
                "include .las.",
                true);
        arg.add("-v", "--voxel", "100");
        arg.add("-p", "--print");
        arg.parse(argc, argv);

        if (arg.contains("--print"))
        {
            elevationPrint(arg.toString("--input"));
        }
        else
        {
            elevationCompute(arg.toString("--input"), arg.toDouble("--voxel"));
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
