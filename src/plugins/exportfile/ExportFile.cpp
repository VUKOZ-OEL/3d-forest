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

#include <cstring>

#include <Editor.hpp>
#include <ExportFile.hpp>
#include <Time.hpp>

#define MODULE_NAME "ExportFile"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ExportFile::ExportFile(Editor *editor)
    : ProgressActionInterface(),
      editor_(editor),
      query_(editor)
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
    file_.close();

    nPointsTotal_ = 0;
    region_.clear();

    query_.selectBox(editor_->clipBoundary());
    query_.exec();

    if (query_.next())
    {
        regionMin_.set(query_.x(), query_.y(), query_.z());
        regionMax_ = regionMin_;

        nPointsTotal_++;
    }

    ProgressActionInterface::initialize(ProgressActionInterface::npos, 1000UL);
}

void ExportFile::clear()
{
    LOG_DEBUG_LOCAL("");
    query_.clear();
}

void ExportFile::step()
{
    if (initializing())
    {
        determineMaximum();
        return;
    }

    uint64_t n = process();
    uint64_t i = 0;

    startTimer();

    createFile(editor_, path_, file_);

    while (i < n)
    {
        writePoint();

        i++;

        if (timedOut())
        {
            break;
        }
    }

    increment(i);

    if (end())
    {
        file_.close();
    }
}

void ExportFile::determineMaximum()
{
    startTimer();

    while (query_.next())
    {
        regionMin_.updateLess(query_.x(), query_.y(), query_.z());
        regionMax_.updateGreater(query_.x(), query_.y(), query_.z());

        nPointsTotal_++;

        if (timedOut())
        {
            return;
        }
    }

    query_.reset();

    region_.set(regionMin_, regionMax_);

    ProgressActionInterface::initialize(nPointsTotal_, 1000UL);
}

void ExportFile::createFile(Editor *editor,
                            const std::string path,
                            LasFile &file)
{
    if (file.file().isOpen())
    {
        return;
    }

    file.create(path);

    file.header.set(nPointsTotal_, region_, {0.001, 0.001, 0.001});

    file.writeHeader();
}

void ExportFile::writePoint()
{
    if (!query_.next())
    {
        return;
    }

    LasFile::Point point;
    std::memset(&point, 0, sizeof(point));

    point.x = static_cast<uint32_t>(query_.x());
    point.y = static_cast<uint32_t>(query_.y());
    point.z = static_cast<uint32_t>(query_.z());

    point.format = 6;
    point.intensity = 65535;

    file_.writePoint(point);
}
