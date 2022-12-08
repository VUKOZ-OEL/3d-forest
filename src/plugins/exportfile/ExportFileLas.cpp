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

/** @file ExportFileLas.cpp */

#include <cstring>

#include <ExportFileLas.hpp>

#define MODULE_NAME "ExportFileLas"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ExportFileLas::ExportFileLas()
{
    LOG_DEBUG_LOCAL("");
}

ExportFileLas::~ExportFileLas()
{
    LOG_DEBUG_LOCAL("");
}

void ExportFileLas::create(const std::string &path,
                           uint64_t nPoints,
                           const Box<double> &region)
{
    LOG_DEBUG_LOCAL("");

    file_.create(path);
    file_.header.set(nPoints, region, {0.001, 0.001, 0.001});
    file_.writeHeader();
}

void ExportFileLas::write(Query &query)
{
    LasFile::Point point;
    std::memset(&point, 0, sizeof(point));

    point.x = static_cast<uint32_t>(query.x());
    point.y = static_cast<uint32_t>(query.y());
    point.z = static_cast<uint32_t>(query.z());

    point.format = 6;
    point.intensity = 65535;

    file_.writePoint(point);
}

void ExportFileLas::close()
{
    LOG_DEBUG_LOCAL("");

    file_.close();
}
