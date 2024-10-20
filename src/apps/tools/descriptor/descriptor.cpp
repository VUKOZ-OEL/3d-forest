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

/** @file descriptor.cpp
    @brief Descriptor calculation command line tool.
*/

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <ComputeDescriptorAction.hpp>
#include <ComputeDescriptorParameters.hpp>
#include <Editor.hpp>
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "descriptor"
#include <Log.hpp>

static void descriptorCompute(const std::string &inputPath,
                              const ComputeDescriptorParameters &parameters)
{
    // Open input file in editor.
    Editor editor;
    editor.open(inputPath);

    // Calculate descriptors by steps.
    ComputeDescriptorAction descriptor(&editor);
    descriptor.start(parameters);
    while (!descriptor.end())
    {
        descriptor.next();
    }

    editor.saveProject(editor.projectPath());
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_descriptor.txt");

    try
    {
        ComputeDescriptorParameters p;

        ArgumentParser arg("calculates descriptor values for points");
        arg.add("-f",
                "--file",
                "",
                "Path to the input file to be processed. Accepted formats "
                "include .las, and .json project file.",
                true);
        arg.add("-m", "--method", "density", "Method {density,pca}");
        arg.add("-v", "--voxel", toString(p.voxelRadius), "Voxel radius [m]");
        arg.add("-r",
                "--search-radius",
                toString(p.searchRadius),
                "Neighborhood search radius [m]");
        arg.add("-g",
                "--include-ground",
                toString(p.includeGroundPoints),
                "Include ground points {true, false}");

        if (arg.parse(argc, argv))
        {
            if (arg.toString("--method") == "density")
            {
                p.method = ComputeDescriptorParameters::METHOD_DENSITY;
            }
            else if (arg.toString("--method") == "pca")
            {
                p.method = ComputeDescriptorParameters::METHOD_PCA_INTENSITY;
            }
            else
            {
                THROW("Invalid method option. "
                      "Try '--help' for more information.");
            }

            p.voxelRadius = arg.toDouble("--voxel");
            p.searchRadius = arg.toDouble("--search-radius");
            p.includeGroundPoints = arg.toBool("--include-ground");

            descriptorCompute(arg.toString("--file"), p);
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
