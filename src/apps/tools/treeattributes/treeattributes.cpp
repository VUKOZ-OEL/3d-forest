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

/** @file treeattributes.cpp
    @brief Command line tool to calculate tree attributes.
*/

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <Error.hpp>
#include <TreeAttributesAction.hpp>
#include <TreeAttributesParameters.hpp>

// Include local.
#define LOG_MODULE_NAME "treeattributes"
#include <Log.hpp>

static void compute(const std::string &inputPath,
                    const TreeAttributesParameters &parameters)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Calculate tree attributes by steps.
    TreeAttributesAction treeAttributes(&editor);
    treeAttributes.start(parameters);
    while (!treeAttributes.end())
    {
        treeAttributes.next();
    }

    editor.saveProject(editor.projectPath());
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_tree_attributes.txt");

    try
    {
        TreeAttributesParameters p;

        ArgumentParser arg("calculates tree attributes");

        arg.add("-f",
                "--file",
                "",
                "Path to the input file to be processed. Accepted formats "
                "include .las, and .json project file.",
                true);

        arg.add("",
                "--position-height-range",
                toString(p.treePositionHeightRange),
                "Tree position height range [m]");

        arg.add("",
                "--dbh-elevation",
                toString(p.dbhElevation),
                "Calculate DBH at given elevation [m]");

        arg.add("",
                "--dbh-range",
                toString(p.dbhElevationRange),
                "DBH elevation range +- [m]");

        if (arg.parse(argc, argv))
        {
            p.treePositionHeightRange = arg.toDouble("--position-height-range");
            p.dbhElevation = arg.toDouble("--dbh-elevation");
            p.dbhElevationRange = arg.toDouble("--dbh-range");

            compute(arg.toString("--file"), p);
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
