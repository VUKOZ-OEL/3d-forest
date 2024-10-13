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

/** @file TreeTableExportCsv.cpp */

// Include 3D Forest.
#include <TreeTableExportCsv.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeTableExportCsv"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

TreeTableExportCsv::TreeTableExportCsv()
{
    LOG_DEBUG(<< "Create.");
}

TreeTableExportCsv::~TreeTableExportCsv()
{
    LOG_DEBUG(<< "Destroy.");
}

void TreeTableExportCsv::create(const std::string &path)
{
    // Create/overwrite new text file which is open for writing.
    file_.open(path, "w+t");

    // Write CSV header.
    char text[256] = {};
    text[0] = 0;

    (void)ustrcat(text, "id, label, x, y, z, height, dbh, status");
    (void)ustrcat(text, "\n");

    file_.write(text);
}

void TreeTableExportCsv::write(const Segment &segment)
{
    if (properties().exportValidValuesOnly() &&
        segment.treeAttributes.status != TreeAttributes::Status::VALID)
    {
        return;
    }

    double ppm = properties().pointsPerMeter();

    // Format data into text line.
    char text[4096];

    (void)snprintf(text,
                   sizeof(text),
                   "%d, \"%s\", %f, %f, %f, %f, %f, \"%s\"\n",
                   static_cast<int>(segment.id),
                   segment.label.c_str(),
                   segment.treeAttributes.position[0] / ppm,
                   segment.treeAttributes.position[1] / ppm,
                   segment.treeAttributes.position[2] / ppm,
                   segment.treeAttributes.height / ppm,
                   segment.treeAttributes.dbh / ppm,
                   toString(segment.treeAttributes.status).c_str());

    // Write new tree into file.
    file_.write(text);
}

void TreeTableExportCsv::close()
{
    // Close the file.
    file_.close();
}
