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

/** @file ClassificationAction.cpp */

#include <ClassificationAction.hpp>
#include <Editor.hpp>

#define LOG_MODULE_NAME "ClassificationAction"
#include <Log.hpp>

ClassificationAction::ClassificationAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Called.");
}

ClassificationAction::~ClassificationAction()
{
    LOG_DEBUG(<< "Called.");
}

void ClassificationAction::clear()
{
    LOG_DEBUG(<< "Called.");

    query_.clear();
    queryPoint_.clear();
}

void ClassificationAction::initialize(size_t pointsPerCell,
                                      double cellLengthMinPercent,
                                      double groundErrorPercent,
                                      double angleDeg)
{
    LOG_DEBUG(<< "Called with parameter pointsPerCell <" << pointsPerCell
              << "> "
              << "cellLengthMinPercent <" << cellLengthMinPercent << "> "
              << "groundErrorPercent <" << groundErrorPercent << "> "
              << "angleDeg <" << angleDeg << ">.");

    groundErrorPercent_ = groundErrorPercent;

    // Ground plane angle to inverted angle for selection.
    angleDeg_ = 90.0 - angleDeg;

    query_.setGrid(pointsPerCell, cellLengthMinPercent);

    size_t numberOfSteps = query_.gridSize();
    LOG_DEBUG(<< "Initialize numberOfSteps <" << numberOfSteps << ">.");

    ProgressActionInterface::initialize(numberOfSteps, 1UL);
}

void ClassificationAction::next()
{
    if (query_.nextGrid())
    {
        stepGrid();
    }

    increment(1);

    if (end())
    {
        LOG_DEBUG(<< "Flush modifications.");
        query_.flush();
    }
}

void ClassificationAction::stepGrid()
{
    double zMax = editor_->clipBoundary().max(2);
    double zMin = editor_->clipBoundary().min(2);
    double zMinCell;
    double zMaxGround;

    size_t nPointsGroundGrid;
    size_t nPointsAboveGrid;

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

    LOG_DEBUG(<< "Number of points as ground <" << nPointsGroundGrid
              << "> above ground <" << nPointsAboveGrid << ">.");
}
