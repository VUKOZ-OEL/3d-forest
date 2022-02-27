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

// Ignore compiler warnings from Eigen 3rd party library.
#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) ||               \
     defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <EditorDatabase.hpp>
#include <Error.hpp>
#include <FileIndexBuilder.hpp>
#include <Time.hpp>
#include <cstring>
#include <delaunator.hpp>
#include <igl/writeOBJ.h>

static void writeGroundMesh(const char *inputPath)
{
    if (!inputPath)
    {
        THROW("Missing input file path argument");
    }

    // Open database.
    EditorDatabase db;
    db.open(inputPath);

    // Obtain the number of ground points.
    EditorQuery queryGround(&db);
    queryGround.selectClassifications({FileLas::CLASS_GROUND});
    queryGround.selectBox(db.clipBoundary());
    size_t nPointsGround = 0;
    queryGround.exec();
    while (queryGround.nextPoint())
    {
        nPointsGround++;
    }

    std::cout << nPointsGround << " ground points" << std::endl;

    if (nPointsGround == 0)
    {
        // std::cout << "ground points not found\n";
        return;
    }

    // Collect 2D and 3D point coordinates.
    Eigen::MatrixXd V;
    V.resize(nPointsGround, 3);

    std::vector<double> XY;
    XY.resize(nPointsGround * 2);

    nPointsGround = 0;
    queryGround.reset();
    while (queryGround.nextPoint())
    {
        XY[2 * nPointsGround] = queryGround.x();
        XY[2 * nPointsGround + 1] = queryGround.y();

        V(nPointsGround, 0) = queryGround.x();
        V(nPointsGround, 1) = queryGround.y();
        V(nPointsGround, 2) = queryGround.z();

        nPointsGround++;
    }

    // Create triangle mesh.
    delaunator::Delaunator delaunay(XY);

    // Convert to igl triangles.
    size_t nTriangles = delaunay.triangles.size() / 3;

    Eigen::MatrixXi F;
    F.resize(nTriangles, 3);

    for (size_t i = 0; i < nTriangles; i++)
    {
        F(i, 0) = delaunay.triangles[i * 3];
        F(i, 1) = delaunay.triangles[i * 3 + 1];
        F(i, 2) = delaunay.triangles[i * 3 + 2];
    }

    // Write output file.
    igl::writeOBJ("ground.obj", V, F);
}

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

    int nGrids = 0;
    size_t nPoints = 0;
    size_t nPointsGroundLevel = 0;
    size_t nPointsGround = 0;
    size_t nPointsGroundGrid;
    std::vector<double> groundXY;

    EditorQuery queryPoint(&db);
    EditorQuery query(&db);
    query.setGrid();

    double t1 = getRealTime();

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
        nPointsGroundGrid = 0;
        query.reset();
        while (query.nextPoint())
        {
            nPoints++;

            if (query.z() > zMaxGround)
            {
                // unassigned (could be a roof)
                query.classification() = FileLas::CLASS_UNASSIGNED;
            }
            else
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
                }
                else
                {
                    // ground
                    query.classification() = FileLas::CLASS_GROUND;

                    nPointsGround++;
                    nPointsGroundGrid++;
                }
            }

            query.setModified();
        }

        // Ground surface.
        /*
        if (nPointsGroundGrid > 0)
        {
            groundXY.resize(nPointsGroundGrid * 2);
            nPointsGroundGrid = 0;
            query.reset();
            while (query.nextPoint())
            {
                if (query.classification() == FileLas::CLASS_GROUND)
                {
                    groundXY[2 * nPointsGroundGrid] = query.x();
                    groundXY[2 * nPointsGroundGrid + 1] = query.y();
                    nPointsGroundGrid++;
                }
            }

            delaunator::Delaunator d(groundXY);

            std::cout << d.triangles.size() << " ground triangles\n";
        }
        */

        // Limit grid processing.
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

enum Command
{
    COMMAND_NONE,
    COMMAND_CLASSIFY,
    COMMAND_EXPORT_GROUND_MESH
};

int main(int argc, char *argv[])
{
    const char *inputPath = nullptr;
    int command = COMMAND_NONE;
    int nGridsMax = 0;

    // Parse command line arguments
    for (int opt = 1; opt < argc; opt++)
    {
        if (strcmp(argv[opt], "-c") == 0)
        {
            command = COMMAND_CLASSIFY;
        }
        else if (strcmp(argv[opt], "-g") == 0)
        {
            command = COMMAND_EXPORT_GROUND_MESH;
        }
        else if (strcmp(argv[opt], "-i") == 0)
        {
            getarg(&inputPath, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "--max-grids") == 0)
        {
            getarg(&nGridsMax, opt, argc, argv);
        }
    }

    // Execute command
    try
    {
        switch (command)
        {
            case COMMAND_CLASSIFY:
                classify(inputPath, nGridsMax);
                break;
            case COMMAND_EXPORT_GROUND_MESH:
                writeGroundMesh(inputPath);
                break;
            case COMMAND_NONE:
            default:
                THROW("Unknown command");
                break;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
