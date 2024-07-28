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

/** @file dbh.cpp
    @brief DBH calculation command line tool.
*/

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <DbhAction.hpp>
#include <DbhParameters.hpp>
#include <Editor.hpp>
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "dbh"
#include <Log.hpp>

static void dbhCompute(const std::string &inputPath,
                       const DbhParameters &parameters)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Calculate DBHs by steps.
    DbhAction dbh(&editor);
    dbh.start(parameters);
    while (!dbh.end())
    {
        dbh.next();
    }

    editor.saveProject(editor.projectPath());
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_dbh.txt");

    try
    {
        DbhParameters p;

        ArgumentParser arg(
            "calculates DBH (Diameter at Breast Height) for trees");
        arg.add("-i",
                "--input",
                "",
                "Path to the input file to be processed. Accepted formats "
                "include .las, and .json project file.",
                true);
        arg.add("-e",
                "--elevation",
                toString(p.elevation),
                "Calculate DBH at given elevation [m]");
        arg.add("-t",
                "--tolerance",
                toString(p.elevationTolerance),
                "DBH elevation elevationTolerance +- [m]");

        if (arg.parse(argc, argv))
        {
            p.elevation = arg.toDouble("--elevation");
            p.elevationTolerance = arg.toDouble("--tolerance");

            dbhCompute(arg.toString("--input"), p);
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
