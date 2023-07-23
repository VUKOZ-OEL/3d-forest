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
                                  double voxel,
                                  double radius,
                                  double angle)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Classify ground by steps.
    ClassificationAction classification(&editor);
    classification.start(voxel, radius, angle);
    while (!classification.end())
    {
        classification.next();
    }
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_classification.txt");

    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.add("--voxel", "100");
        arg.add("--radius", "200");
        arg.add("--angle", "60");
        arg.parse(argc, argv);

        classificationCompute(arg.toString("--input"),
                              arg.toDouble("--voxel"),
                              arg.toDouble("--radius"),
                              arg.toDouble("--angle"));

        rc = 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    LOGGER_STOP_FILE;

    return rc;
}
