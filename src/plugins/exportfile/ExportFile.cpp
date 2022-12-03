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

/** @file ExportFile.cpp */

#include <Editor.hpp>
#include <ExportFile.hpp>

#define MODULE_NAME "ExportFile"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ExportFile::ExportFile(Editor *editor) : editor_(editor), queryPoints_(editor)
{
    LOG_DEBUG_LOCAL("");
}

ExportFile::~ExportFile()
{
    LOG_DEBUG_LOCAL("");
}

void ExportFile::clear()
{
    LOG_DEBUG_LOCAL("");

    queryPoints_.clear();

    currentStep_ = 0;
    numberOfSteps_ = 0;

    nPointsTotal_ = 0;
    nPointsPerStep_ = 0;
    nPointsProcessed_ = 0;
}

int ExportFile::start()
{
    LOG_DEBUG_LOCAL("");

    nPointsTotal_ = 0;
    queryPoints_.selectBox(editor_->clipBoundary());
    queryPoints_.exec();
    while (queryPoints_.next())
    {
        nPointsTotal_++;
    }

    nPointsPerStep_ = 1000;
    nPointsProcessed_ = 0;

    uint64_t nSteps = nPointsTotal_ / nPointsPerStep_;
    if (nPointsTotal_ % nPointsPerStep_ > 0)
    {
        nSteps++;
    }

    currentStep_ = 0;
    numberOfSteps_ = static_cast<int>(nSteps * 2);

    LOG_DEBUG_LOCAL("numberOfSteps <" << numberOfSteps_ << ">");

    return numberOfSteps_;
}

void ExportFile::step()
{
    LOG_DEBUG_LOCAL("step <" << (currentStep_ + 1) << "> from <"
                             << numberOfSteps_ << ">");

    currentStep_++;
}
