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

/** @file classifyGround.cpp */

#include <ClassifyGround.hpp>
#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>

static void classifyGround(const char *inputPath, size_t pointsPerCell)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Classify ground by steps.
    ClassifyGround cg(&editor);
    int n = cg.start(pointsPerCell);
    for (int i = 0; i < n; i++)
    {
        std::cout << "Step " << (i + 1) << "/" << n << std::endl;
        cg.step();
        // cg.exportGroundMesh("ground");
    }
}

int main(int argc, char *argv[])
{
    const char *inputPath = nullptr;
    size_t pointsPerCell = 10000;

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

    try
    {
        classifyGround(inputPath, pointsPerCell);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
