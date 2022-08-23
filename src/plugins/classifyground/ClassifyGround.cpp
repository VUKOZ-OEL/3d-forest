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

/** @file ClassifyGround.cpp */

#include <delaunator.hpp>
#include <igl/point_mesh_squared_distance.h>
#include <igl/writeOBJ.h>

#include <ClassifyGround.hpp>
#include <Editor.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("ClassifyGround", msg)

#define CLASSIFY_GROUND_BUFFER_SIZE 8192

ClassifyGround::ClassifyGround(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG_LOCAL("");
}

ClassifyGround::~ClassifyGround()
{
    LOG_DEBUG_LOCAL("");
}

int ClassifyGround::start(size_t pointsPerCell,
                          double cellLengthMinPercent,
                          double groundErrorPercent,
                          double angleDeg)
{
    // clang-format off
    LOG_DEBUG_LOCAL("pointsPerCell <" << pointsPerCell << "> " <<
                    "cellLengthMinPercent <" << cellLengthMinPercent << "> " <<
                    "groundErrorPercent <" << groundErrorPercent << "> " <<
                    "angleDeg <" << angleDeg << "> ");
    // clang-format on

    groundErrorPercent_ = groundErrorPercent;

    // Ground plane angle to inverted angle for selection.
    angleDeg_ = 90.0 - angleDeg;

    P.resize(CLASSIFY_GROUND_BUFFER_SIZE, 3);
    V.resize(CLASSIFY_GROUND_BUFFER_SIZE, 3);
    XY.resize(CLASSIFY_GROUND_BUFFER_SIZE * 2);

    query_.setGrid(pointsPerCell, cellLengthMinPercent);

    currentStep_ = 0;
    numberOfSteps_ = static_cast<int>(query_.gridSize());

    LOG_DEBUG_LOCAL("numberOfSteps <" << numberOfSteps_ << "> ");

    return numberOfSteps_;
}

void ClassifyGround::step()
{
    LOG_DEBUG_LOCAL("step <" << currentStep_ << "/" << numberOfSteps_ << "> ");

    double zMax = editor_->clipBoundary().max(2);
    double zMin = editor_->clipBoundary().min(2);
    double zMinCell;
    double zMaxGround;

    size_t nPointsGroundGrid;
    size_t nPointsAboveGrid;

    if (!query_.nextGrid())
    {
        // TBD Error.
        return;
    }

    // Select grid cell.
    query_.selectBox(query_.gridCell());
    query_.exec();

    // Find local minimum.
    zMinCell = zMax;
    while (query_.next())
    {
        if (query_.z() < zMinCell)
        {
            zMinCell = query_.z();
        }
    }
    zMaxGround = zMinCell + (((zMax - zMin) * 0.01) * groundErrorPercent_);

    // Set classification to 'ground' or 'unassigned'.
    nPointsGroundGrid = 0;
    nPointsAboveGrid = 0;
    query_.reset();
    while (query_.next())
    {
        if (query_.z() > zMaxGround)
        {
            // unassigned (could be a roof)
            query_.classification() = LasFile::CLASS_UNASSIGNED;
        }
        else
        {
            queryPoint_.setMaximumResults(1);

            queryPoint_.selectCone(query_.x(),
                                   query_.y(),
                                   query_.z(),
                                   zMinCell,
                                   angleDeg_);

            queryPoint_.exec();

            if (queryPoint_.next())
            {
                // unassigned (has some points below, inside the cone)
                query_.classification() = LasFile::CLASS_UNASSIGNED;
            }
            else
            {
                // ground
                query_.classification() = LasFile::CLASS_GROUND;
            }
        }

        if (query_.classification() == LasFile::CLASS_GROUND)
        {
            if (static_cast<Eigen::Index>(nPointsGroundGrid) == V.rows())
            {
                V.resize(static_cast<Eigen::Index>(nPointsGroundGrid) * 2, 3);
                XY.resize(nPointsGroundGrid * 4);
            }

            V(static_cast<Eigen::Index>(nPointsGroundGrid), 0) = query_.x();
            V(static_cast<Eigen::Index>(nPointsGroundGrid), 1) = query_.y();
            V(static_cast<Eigen::Index>(nPointsGroundGrid), 2) = query_.z();

            XY[2 * nPointsGroundGrid] = query_.x();
            XY[2 * nPointsGroundGrid + 1] = query_.y();

            nPointsGroundGrid++;
        }
        else
        {
            if (static_cast<Eigen::Index>(nPointsAboveGrid) == P.rows())
            {
                P.resize(static_cast<Eigen::Index>(nPointsAboveGrid) * 2, 3);
            }

            P(static_cast<Eigen::Index>(nPointsAboveGrid), 0) = query_.x();
            P(static_cast<Eigen::Index>(nPointsAboveGrid), 1) = query_.y();
            P(static_cast<Eigen::Index>(nPointsAboveGrid), 2) = query_.z();

            nPointsAboveGrid++;
        }

        query_.elevation() = 0;

        query_.setModified();
    }

    // Ground surface
    if (nPointsGroundGrid > 0)
    {
        XY.resize(nPointsGroundGrid * 2);

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
        query_.reset();
        while (query_.next())
        {
            if (query_.classification() != LasFile::CLASS_GROUND)
            {
                if (idx < D.rows() && D(idx) > 0.)
                {
                    query_.elevation() = ::sqrt(D(idx));
                }

                idx++;

                query_.setModified();
            }
        }
    } // Ground surface

    currentStep_++;

    if (currentStep_ == numberOfSteps_)
    {
        LOG_DEBUG_LOCAL("flush");
        query_.flush();
    }
}

void ClassifyGround::exportGroundMesh(const char *path)
{
    std::string fullPath;
    fullPath = path + std::to_string(currentStep_) + ".obj";
    LOG_DEBUG_LOCAL("path <" << fullPath << ">");

    igl::writeOBJ(fullPath, V, F);
}

void ClassifyGround::clear()
{
    LOG_DEBUG_LOCAL("");

    query_.clear();
    queryPoint_.clear();

    currentStep_ = 0;
    numberOfSteps_ = 0;

    XY.clear();

    P.reserve(0, 0);
    V.reserve(0, 0);
    F.reserve(0, 0);
    D.reserve(0, 0);
    I.reserve(0, 0);
    C.reserve(0, 0);
}
