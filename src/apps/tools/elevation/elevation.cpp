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

/** @file elevation.cpp */

#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <Elevation.hpp>
#include <Error.hpp>
#include <Log.hpp>

static void elevationCompute(const std::string inputPath,
                             size_t pointsPerCell,
                             double cellLengthMinPercent)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Compute elevation by steps.
    Elevation e(&editor);
    int n = e.start(pointsPerCell, cellLengthMinPercent);
    for (int i = 0; i < n; i++)
    {
        std::cout << "Step " << (i + 1) << "/" << n << std::endl;
        e.step();
        // e.exportGroundMesh("ground");
    }

    std::cout << "elevation minimum <" << e.minimum() << ">" << std::endl;
    std::cout << "elevation maximum <" << e.maximum() << ">" << std::endl;
}

static void elevationPrint(const std::string inputPath)
{
    Editor editor;
    editor.open(inputPath);

    uint64_t nPoints = 0;
    double elevationMinimum = 0;
    double elevationMaximum = 0;

    Query query(&editor);
    query.selectBox(editor.clipBoundary());
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

static void elevationPlot(const std::string inputPath,
                          size_t plotSize,
                          double elevationMinimum,
                          double elevationMaximum)
{
}

int main(int argc, char *argv[])
{
    ArgumentParser arg;
    arg.add("--input", "");
    arg.add("--compute", "");
    arg.add("--cell-points", "10000");
    arg.add("--cell-size-min", "5");
    arg.add("--print", "");
    arg.add("--plot", "");
    arg.add("--plot-size", "100");
    arg.add("--plot-min", "0");
    arg.add("--plot-max", "0");
    arg.parse(argc, argv);

    try
    {
        if (arg.contains("--compute"))
        {
            elevationCompute(arg.toString("--input"),
                             arg.toSize("--cell-points"),
                             arg.toDouble("--cell-size-min"));
        }
        else if (arg.contains("--print"))
        {
            elevationPrint(arg.toString("--input"));
        }
        else if (arg.contains("--plot"))
        {
            elevationPlot(arg.toString("--input"),
                          arg.toSize("--plot-size"),
                          arg.toDouble("--plot-min"),
                          arg.toDouble("--plot-max"));
        }
        else
        {
            THROW("Unknown action");
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
