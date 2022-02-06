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

/** @file classify.cpp @brief Classify points in given file. */

#include <EditorDatabase.hpp>
#include <Error.hpp>
#include <FileIndexBuilder.hpp>
#include <Time.hpp>
#include <cstring>

static void classify(const char *inputPath, int nGridsMax)
{
    if (!inputPath)
    {
        THROW("Missing input file path argument");
    }

    EditorDatabase db;
    db.open(inputPath);

    double zMax = db.clipBoundary().max(2);
    double zMin = db.clipBoundary().min(2);
    double zMinCell;
    double zMaxGround;

    double angle = 40.0;
    double groundErrorPercent = 15.0;

    // std::cout << "z max is " << zMax << std::endl;
    // std::cout << "z min is " << zMin << std::endl;

    EditorQuery queryPoint(&db);
    EditorQuery query(&db);
    query.setGrid();

    double t1 = getRealTime();

    int nGrids = 0;
    int nPoints = 0;
    int nPointsGroundLevel = 0;
    int nPointsGround = 0;

    while (query.nextGrid())
    {
        // Select grid cell.
        query.selectBox(query.gridCell());
        query.exec();

        // Find local minimum.
        zMinCell = zMax;
        while (query.nextPoint())
        {
            if (query.z() < zMinCell)
            {
                zMinCell = query.z();
            }
        }
        zMaxGround = zMinCell + (((zMax - zMin) * 0.01) * groundErrorPercent);

        // Set classification to 'ground' or 'unassigned'.
        query.reset();
        while (query.nextPoint())
        {
            nPoints++;

            if (query.z() < zMaxGround)
            {
                nPointsGroundLevel++;

                queryPoint.setMaximumResults(1);
                queryPoint.selectCone(query.x(),
                                      query.y(),
                                      query.z(),
                                      zMinCell,
                                      angle);
                queryPoint.exec();

                if (queryPoint.nextPoint())
                {
                    // unassigned (has some points below, inside the cone)
                    query.classification() = FileLas::CLASS_UNASSIGNED;
                    nPointsGround++;
                }
                else
                {
                    // ground
                    query.classification() = FileLas::CLASS_GROUND;
                }
            }
            else
            {
                // unassigned (could be a roof)
                query.classification() = FileLas::CLASS_UNASSIGNED;
            }

            query.setModified();
        }

        nGrids++;
        if (nGrids == nGridsMax)
        {
            break;
        }
    }

    double t2 = getRealTime();

    query.write();

    std::cout << (t2 - t1) << " seconds" << std::endl;
    std::cout << "nGrids=" << nGrids << std::endl;
    std::cout << "nPoints=" << nPoints << std::endl;
    std::cout << "nPointsGroundLevel=" << nPointsGroundLevel << std::endl;
    std::cout << "nPointsGround=" << nPointsGround << std::endl;
}

static void getarg(const char **v, int &opt, int argc, char *argv[])
{
    opt++;
    if (opt < argc)
    {
        *v = argv[opt];
    }
}

void getarg(int *v, int &opt, int argc, char *argv[])
{
    opt++;
    if (opt < argc)
    {
        *v = std::stoi(argv[opt]);
    }
}

int main(int argc, char *argv[])
{
    const char *inputPath = nullptr;
    int nGridsMax = 0;

    // Parse command line arguments
    for (int opt = 1; opt < argc; opt++)
    {
        if (strcmp(argv[opt], "-i") == 0)
        {
            getarg(&inputPath, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-m") == 0)
        {
            getarg(&nGridsMax, opt, argc, argv);
        }
    }

    // Execute command
    try
    {
        classify(inputPath, nGridsMax);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
