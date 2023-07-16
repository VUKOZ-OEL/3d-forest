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

/** @file classification.cpp */

#include <ArgumentParser.hpp>
#include <ClassificationAction.hpp>
#include <Editor.hpp>
#include <Error.hpp>

#define LOG_MODULE_NAME "classification"
#include <Log.hpp>

static void classificationCompute(const std::string &inputPath,
                                  size_t pointsPerCell)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Classify ground by steps.
    ClassificationAction classification(&editor);
    classification.initialize(pointsPerCell);
    while (!classification.end())
    {
        classification.next();
    }
}

int main(int argc, char *argv[])
{
    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--cell-points", "10000");
        arg.parse(argc, argv);

        classificationCompute(arg.toString("--input"),
                              arg.toSize("--cell-points"));
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
