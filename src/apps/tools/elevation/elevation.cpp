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

#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <ElevationAction.hpp>
#include <Error.hpp>
#include <Log.hpp>

static void elevationCompute(const std::string &inputPath,
                             size_t pointsPerCell,
                             double cellLengthMinPercent,
                             const std::string &outputGroundMesh)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Compute elevation by steps.
    ElevationAction elevation(&editor);
    elevation.initialize(pointsPerCell, cellLengthMinPercent);
    while (!elevation.end())
    {
        elevation.next();

        if (!outputGroundMesh.empty())
        {
            elevation.exportGroundMesh(outputGroundMesh);
        }
    }

    std::cout << "elevation minimum <" << elevation.minimum() << ">"
              << std::endl;
    std::cout << "elevation maximum <" << elevation.maximum() << ">"
              << std::endl;
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
    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--cell-points", "10000");
        arg.add("--cell-size-min", "5");
        arg.add("--output-ground-mesh", "");
        arg.add("--print", "");
        arg.parse(argc, argv);

        if (arg.contains("--print"))
        {
            elevationPrint(arg.toString("--input"));
        }
        else
        {
            elevationCompute(arg.toString("--input"),
                             arg.toSize("--cell-points"),
                             arg.toDouble("--cell-size-min"),
                             arg.toString("--output-ground-mesh"));
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
