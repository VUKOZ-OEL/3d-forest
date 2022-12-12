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
#include <cstring>

ExportFileCsv::ExportFileCsv()
{
}

ExportFileCsv::~ExportFileCsv()
{
}

void ExportFileCsv::create(const std::string &path)
{
    // Create/overwrite new text file which is open for writing
    file_.open(path, "w+t");

    // Write CSV header
    char text[256];
    text[0] = 0;
    (void)strcat(text, "x, y, z");
    if (properties().format().has(LasFile::FORMAT_INTENSITY))
    {
        (void)strcat(text, ", intensity");
    }
    if (properties().format().has(LasFile::FORMAT_CLASSIFICATION))
    {
        (void)strcat(text, ", classification");
    }
    if (properties().format().has(LasFile::FORMAT_RGB))
    {
        (void)strcat(text, ", red, green, blue");
    }
    if (properties().format().has(LasFile::FORMAT_LAYER))
    {
        (void)strcat(text, ", layer");
    }
    (void)strcat(text, "\n");

    file_.write(text);
}

void ExportFileCsv::write(Query &query)
{
    // Format point data into text line
    char text[512];
    char buffer[32];

    // Format point XYZ coordinates
    const Vector3<double> &scale = properties().scale();
    if (scale[0] < 1.0 || scale[1] < 1.0 || scale[2] < 1.0)
    {
        (void)snprintf(text,
                       sizeof(text),
                       "%f, %f, %f",
                       static_cast<double>(query.x() * scale[0]),
                       static_cast<double>(query.y() * scale[1]),
                       static_cast<double>(query.z() * scale[2]));
    }
    else
    {
        (void)snprintf(text,
                       sizeof(text),
                       "%d, %d, %d",
                       static_cast<int>(query.x()),
                       static_cast<int>(query.y()),
                       static_cast<int>(query.z()));
    }

    // Format point intensity
    if (properties().format().has(LasFile::FORMAT_INTENSITY))
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       ", %d",
                       static_cast<int>(query.intensity() * 65535.0));

        (void)strcat(text, buffer);
    }

    // Format point classification
    if (properties().format().has(LasFile::FORMAT_CLASSIFICATION))
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       ", %d",
                       static_cast<int>(query.classification()));

        (void)strcat(text, buffer);
    }

    // Color
    if (properties().format().has(LasFile::FORMAT_RGB))
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       ", %d, %d, %d",
                       static_cast<int>(query.red() * 65535.0),
                       static_cast<int>(query.green() * 65535.0),
                       static_cast<int>(query.blue() * 65535.0));

        (void)strcat(text, buffer);
    }

    // Layer
    if (properties().format().has(LasFile::FORMAT_LAYER))
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       ", %u",
                       static_cast<unsigned int>(query.layer()));

        (void)strcat(text, buffer);
    }

    // End line
    (void)strcat(text, "\n");

    // Write new point into file
    file_.write(text);
}

void ExportFileCsv::close()
{
    // Close the file
    file_.close();
}
