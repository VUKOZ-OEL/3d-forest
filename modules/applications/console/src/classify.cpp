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

static void classify(const char *inputPath)
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
            if (query.z() < zMaxGround)
            {
                queryPoint.selectCone(query.x(),
                                      query.y(),
                                      query.z(),
                                      zMinCell,
                                      angle);
                queryPoint.exec();

                if (queryPoint.nextPoint())
                {
                    // unassigned (has some points below inside the cone)
                    query.classification() = 1;
                }
                else
                {
                    // ground
                    query.classification() = 2;
                }
            }
            else
            {
                // unassigned (could be a roof)
                query.classification() = 1;
            }

            query.setModified();
        }
    }

    query.write();
}

static void getarg(const char **v, int &opt, int argc, char *argv[])
{
    opt++;
    if (opt < argc)
    {
        *v = argv[opt];
    }
}

int main(int argc, char *argv[])
{
    const char *inputPath = nullptr;

    // Parse command line arguments
    for (int opt = 1; opt < argc; opt++)
    {
        if (strcmp(argv[opt], "-i") == 0)
        {
            getarg(&inputPath, opt, argc, argv);
        }
    }

    // Execute command
    try
    {
        classify(inputPath);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
