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

#include <Editor.hpp>
#include <Elevation.hpp>
#include <Error.hpp>
#include <Log.hpp>

static void elevation(const char *inputPath,
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

int main(int argc, char *argv[])
{
    const char *inputPath = nullptr;
    size_t pointsPerCell = 10000;
    double cellLengthMinPercent = 5.;

    if (argc > 1)
    {
        inputPath = argv[1];
    }

    if (argc > 2)
    {
        int v = atoi(argv[2]);
        if (v > 0)
        {
            pointsPerCell = static_cast<size_t>(v);
        }
    }

    if (argc > 3)
    {
        double v = atof(argv[3]);
        if (v > 0.)
        {
            cellLengthMinPercent = v;
        }
    }

    try
    {
        elevation(inputPath, pointsPerCell, cellLengthMinPercent);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
