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

/** @file Classification.cpp */

#include <Classification.hpp>
#include <Editor.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("Classification", msg)

Classification::Classification(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG_LOCAL("");
}

Classification::~Classification()
{
    LOG_DEBUG_LOCAL("");
}

int Classification::start(size_t pointsPerCell,
                          double cellLengthMinPercent,
                          double groundErrorPercent,
                          double angleDeg)
{
    // clang-format off
    LOG_DEBUG_LOCAL("pointsPerCell <" << pointsPerCell << "> " <<
                    "cellLengthMinPercent <" << cellLengthMinPercent << "> " <<
                    "groundErrorPercent <" << groundErrorPercent << "> " <<
                    "angleDeg <" << angleDeg << ">");
    // clang-format on

    groundErrorPercent_ = groundErrorPercent;

    // Ground plane angle to inverted angle for selection.
    angleDeg_ = 90.0 - angleDeg;

    query_.setGrid(pointsPerCell, cellLengthMinPercent);

    currentStep_ = 0;
    numberOfSteps_ = static_cast<int>(query_.gridSize());

    LOG_DEBUG_LOCAL("numberOfSteps <" << numberOfSteps_ << ">");

    return numberOfSteps_;
}

void Classification::step()
{
    LOG_DEBUG_LOCAL("step <" << (currentStep_ + 1) << "> from <"
                             << numberOfSteps_ << ">");

    double zMax = editor_->clipBoundary().max(2);
    double zMin = editor_->clipBoundary().min(2);
    double zMinCell;
    double zMaxGround;

    size_t nPointsGroundGrid;
    size_t nPointsAboveGrid;

    if (!query_.nextGrid())
    {
        LOG_DEBUG_LOCAL("expected nextGrid");
        return;
    }

    // Select grid cell.
    query_.where().setBox(query_.gridCell());
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
            nPointsAboveGrid++;
        }
        else
        {
            queryPoint_.setMaximumResults(1);

            queryPoint_.where().setCone(query_.x(),
                                        query_.y(),
                                        query_.z(),
                                        zMinCell,
                                        angleDeg_);

            queryPoint_.exec();

            if (queryPoint_.next())
            {
                // unassigned (has some points below, inside the cone)
                query_.classification() = LasFile::CLASS_UNASSIGNED;
                nPointsAboveGrid++;
            }
            else
            {
                // ground
                query_.classification() = LasFile::CLASS_GROUND;
                nPointsGroundGrid++;
            }
        }

        // New ground level may brake elevation.
        // TBD query_.elevation() = 0;

        query_.setModified();
    }

    LOG_DEBUG_LOCAL("number of points as ground <" << nPointsGroundGrid
                                                   << "> above ground <"
                                                   << nPointsAboveGrid << ">");

    currentStep_++;

    if (currentStep_ == numberOfSteps_)
    {
        LOG_DEBUG_LOCAL("flush");
        query_.flush();
    }
}

void Classification::clear()
{
    LOG_DEBUG_LOCAL("");

    query_.clear();
    queryPoint_.clear();

    currentStep_ = 0;
    numberOfSteps_ = 0;
}
