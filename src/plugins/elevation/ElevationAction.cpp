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

/** @file ElevationAction.cpp */

#include <delaunator.hpp>
#include <igl/point_mesh_squared_distance.h>
#include <igl/writeOBJ.h>

#include <Editor.hpp>
#include <ElevationAction.hpp>

#define LOG_MODULE_NAME "ElevationAction"
#include <Log.hpp>

ElevationAction::ElevationAction(Editor *editor)
    : editor_(editor),
      query_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ElevationAction::~ElevationAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ElevationAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();

    elevationPointsCount_ = 0;

    elevationMinimum_ = 0;
    elevationMaximum_ = 0;

    XY.clear();

    P.reserve(0, 0);
    V.reserve(0, 0);
    F.reserve(0, 0);
    D.reserve(0, 0);
    I.reserve(0, 0);
    C.reserve(0, 0);
}

void ElevationAction::initialize(size_t pointsPerCell,
                                 double cellLengthMinPercent)
{
    LOG_DEBUG(<< "Initialize with parameters pointsPerCell <" << pointsPerCell
              << "> "
              << "cellLengthMinPercent <" << cellLengthMinPercent << ">.");

    elevationPointsCount_ = 0;

    elevationMinimum_ = 0;
    elevationMaximum_ = 0;

    query_.setGrid(pointsPerCell, cellLengthMinPercent);

    size_t numberOfSteps = query_.gridSize();
    LOG_DEBUG(<< "Initialize numberOfSteps <" << numberOfSteps << ">.");

    progress_.setMaximumStep(numberOfSteps, 1UL);
}

void ElevationAction::next()
{
    if (query_.nextGrid())
    {
        stepGrid();
    }

    progress_.addValueStep(1);

    if (end())
    {
        LOG_DEBUG(<< "Flush modifications.");
        query_.flush();

        if (elevationPointsCount_ > 0)
        {
            Range<double> range;
            range.setMinimum(elevationMinimum_);
            range.setMinimumValue(elevationMinimum_);
            range.setMaximum(elevationMaximum_);
            range.setMaximumValue(elevationMaximum_);

            editor_->setElevationFilter(range);
        }
    }
}

void ElevationAction::stepGrid()
{
    size_t nPointsGroundGrid;
    size_t nPointsAboveGrid;

    // Select grid cell.
    query_.where().setBox(query_.gridCell());
    query_.exec();

    // Get number of ground and non-ground points. Reset elevation to zero.
    nPointsGroundGrid = 0;
    nPointsAboveGrid = 0;
    query_.reset();
    while (query_.next())
    {
        if (query_.classification() == LasFile::CLASS_GROUND)
        {
            // ground
            nPointsGroundGrid++;
        }
        else
        {
            // above ground
            nPointsAboveGrid++;
        }

        query_.elevation() = 0;
        query_.setModified();
    }

    // Compute elevation
    if (nPointsGroundGrid > 2)
    {
        LOG_DEBUG(<< "Number of points as ground <" << nPointsGroundGrid
                  << "> above ground <" << nPointsAboveGrid << ">.");

        // Collect points
        P.resize(static_cast<Eigen::Index>(nPointsAboveGrid), 3);
        V.resize(static_cast<Eigen::Index>(nPointsGroundGrid), 3);
        XY.resize(nPointsGroundGrid * 2);

        nPointsGroundGrid = 0;
        nPointsAboveGrid = 0;
        query_.reset();
        while (query_.next())
        {
            if (query_.classification() == LasFile::CLASS_GROUND)
            {
                V(static_cast<Eigen::Index>(nPointsGroundGrid), 0) = query_.x();
                V(static_cast<Eigen::Index>(nPointsGroundGrid), 1) = query_.y();
                V(static_cast<Eigen::Index>(nPointsGroundGrid), 2) = query_.z();

                XY[2 * nPointsGroundGrid] = query_.x();
                XY[2 * nPointsGroundGrid + 1] = query_.y();

                nPointsGroundGrid++;
            }
            else
            {
                P(static_cast<Eigen::Index>(nPointsAboveGrid), 0) = query_.x();
                P(static_cast<Eigen::Index>(nPointsAboveGrid), 1) = query_.y();
                P(static_cast<Eigen::Index>(nPointsAboveGrid), 2) = query_.z();

                nPointsAboveGrid++;
            }
        }

        // Compute ground surface
        delaunator::Delaunator delaunay(XY);

        // Convert to igl triangles
        size_t nTriangles = delaunay.triangles.size() / 3;

        F.resize(static_cast<Eigen::Index>(nTriangles), 3);

        for (size_t iTriangle = 0; iTriangle < nTriangles; iTriangle++)
        {
            // Swap the order of the vertices in triangle to 0, 2, 1.
            // This will affect the direction of the normal to face up
            // along z.
            F(static_cast<Eigen::Index>(iTriangle), 0) =
                static_cast<int>(delaunay.triangles[iTriangle * 3]);
            F(static_cast<Eigen::Index>(iTriangle), 1) =
                static_cast<int>(delaunay.triangles[iTriangle * 3 + 2]);
            F(static_cast<Eigen::Index>(iTriangle), 2) =
                static_cast<int>(delaunay.triangles[iTriangle * 3 + 1]);
        }

        // Compute distances from a set of points P to a triangle mesh (V,F)
        igl::point_mesh_squared_distance(P, V, F, D, I, C);

        // Set elevation
        Eigen::Index idx = 0;
        Eigen::Index idxElevation = 0;
        query_.reset();
        while (query_.next())
        {
            if (query_.classification() != LasFile::CLASS_GROUND)
            {
                if (idx < D.rows() && D(idx) > 0.)
                {
                    double elevation = ::sqrt(D(idx));

                    query_.elevation() = elevation;
                    query_.setModified();

                    if (elevationPointsCount_ > 0)
                    {
                        if (elevation < elevationMinimum_)
                        {
                            elevationMinimum_ = elevation;
                        }

                        if (elevation > elevationMaximum_)
                        {
                            elevationMaximum_ = elevation;
                        }
                    }
                    else
                    {
                        elevationMinimum_ = elevation;
                        elevationMaximum_ = elevation;
                    }

                    idxElevation++;
                    elevationPointsCount_++;
                }

                idx++;
            }
        }

        LOG_DEBUG(<< "Points with elevation <" << idxElevation << ">.");
    }
}

void ElevationAction::exportGroundMesh(const std::string &path)
{
    std::string fullPath;
    fullPath = path + std::to_string(progress_.valueStep()) + ".obj";
    LOG_DEBUG(<< "Full path <" << fullPath << ">.");

    igl::writeOBJ(fullPath, V, F);
}
