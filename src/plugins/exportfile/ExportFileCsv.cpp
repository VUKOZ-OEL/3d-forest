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

/** @file ExportFileCsv.cpp */

#include <ExportFileCsv.hpp>

ExportFileCsv::ExportFileCsv()
{
}

ExportFileCsv::~ExportFileCsv()
{
}

void ExportFileCsv::create(const std::string &path,
                           uint64_t nPoints,
                           const Box<double> &region)
{
    // Create/overwrite new text file which is open for writing
    file_.open(path, "w+t");

    // Write CSV header
    file_.write("x, y, z\n");
}

void ExportFileCsv::write(Query &query)
{
    // Format point data into text line
    char text[256];

    (void)snprintf(text,
                   sizeof(text),
                   "%11d, %11d, %11d\n",
                   static_cast<int32_t>(query.x()),
                   static_cast<int32_t>(query.y()),
                   static_cast<int32_t>(query.z()));

    // Write new point into file
    file_.write(text);
}

void ExportFileCsv::close()
{
    // Close the file
    file_.close();
}
