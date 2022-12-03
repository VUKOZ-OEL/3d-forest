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
#include <Time.hpp>

#define MODULE_NAME "ExportFile"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ExportFile::ExportFile(Editor *editor)
    : ProgressActionInterface(),
      editor_(editor),
      queryPoints_(editor)
{
    LOG_DEBUG_LOCAL("");
}

ExportFile::~ExportFile()
{
    LOG_DEBUG_LOCAL("");
}

void ExportFile::initialize(const std::string &path)
{
    LOG_DEBUG_LOCAL("");

    path_ = path;

    uint64_t nPointsTotal = 0;
    queryPoints_.selectBox(editor_->clipBoundary());
    queryPoints_.exec();
    while (queryPoints_.next())
    {
        nPointsTotal++;
    }

    return ProgressActionInterface::initialize(nPointsTotal, 1000);
}

void ExportFile::clear()
{
    LOG_DEBUG_LOCAL("");
    queryPoints_.clear();
}

void ExportFile::step()
{
    uint64_t n = process();
    uint64_t i = 0;

    startTimer();

    if (processed() == 0)
    {
        createFile(editor_, path_, file_);
    }

    while (i < n)
    {
        // TBD

        i++;

        if (timedOut())
        {
            break;
        }
    }

    increment(i);
}

void ExportFile::createFile(Editor *editor,
                            const std::string path,
                            LasFile &file)
{
    file.create(path);
}
