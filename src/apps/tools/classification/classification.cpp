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

/** @file classification.cpp
    @brief Ground classification command line tool.
*/

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <ClassificationAction.hpp>
#include <ClassificationParameters.hpp>
#include <Editor.hpp>
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "classification"
#include <Log.hpp>

static void classificationCompute(const std::string &inputPath,
                                  const ClassificationParameters &parameters)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Classify ground by steps.
    ClassificationAction classification(&editor);
    classification.start(parameters);
    while (!classification.end())
    {
        classification.next();
    }

    editor.saveProject(editor.projectPath());
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_classification.txt");

    try
    {
        ClassificationParameters p;

        ArgumentParser arg("classifies ground points");
        arg.add("-i",
                "--input",
                "",
                "Path to the input file to be processed. Accepted formats "
                "include .las, and .json project file.",
                true);
        arg.add("-v", "--voxel", toString(p.voxelRadius), "Voxel radius [m]");
        arg.add("-r",
                "--search-radius",
                toString(p.searchRadius),
                "Neighborhood search radius [m]");
        arg.add("-a",
                "--angle",
                toString(p.angle),
                "Maximum ground angle [deg]");
        arg.add("-c",
                "--clean",
                toString(p.cleanGroundClassifications),
                "Clean ground classifications at start {true, false}");
        arg.add("-ca",
                "--clean-all",
                toString(p.cleanAllClassifications),
                "Clean all classifications at start {true, false}");

        if (arg.parse(argc, argv))
        {
            p.voxelRadius = arg.toDouble("--voxel");
            p.searchRadius = arg.toDouble("--search-radius");
            p.angle = arg.toDouble("--angle");
            p.cleanGroundClassifications = arg.toBool("--clean");
            p.cleanAllClassifications = arg.toBool("--clean-all");

            classificationCompute(arg.toString("--input"), p);
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
