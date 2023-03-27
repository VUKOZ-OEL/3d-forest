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

/** @file ExportFileFormatLas.cpp */

#include <cstring>

#include <ExportFileFormatLas.hpp>
#include <IndexFileBuilder.hpp>

#define LOG_MODULE_NAME "ExportFileFormatLas"
#include <Log.hpp>

ExportFileFormatLas::ExportFileFormatLas()
{
    LOG_DEBUG(<< "Called.");
}

ExportFileFormatLas::~ExportFileFormatLas()
{
    LOG_DEBUG(<< "Called.");
}

void ExportFileFormatLas::create(const std::string &path)
{
    LOG_DEBUG(<< "Called with parameter path <" << path << "> nPoints <"
              << properties().numberOfPoints() << "> region <"
              << properties().region() << ">.");

    // Create new file which is open for writing
    file_.create(path);

    // Fill LAS header
    std::memset(&file_.header, 0, sizeof(file_.header));
    file_.header.set(properties().numberOfPoints(),
                     properties().region(),
                     properties().scale(),
                     properties().offset(),
                     properties().format().las());

    // Write LAS header
    file_.writeHeader();

    // Remove index file
    std::string pathIndex = IndexFileBuilder::extension(path);
    if (File::exists(pathIndex))
    {
        File::remove(pathIndex);
    }
}

void ExportFileFormatLas::write(Query &query)
{
    const double f16 = 65535.0;

    LasFile::Point point;

    // Set point data to zeroes
    std::memset(&point, 0, sizeof(point));

    // Set point data format
    point.format = properties().format().las();

    // Set point data
    point.x = static_cast<int32_t>(query.x());
    point.y = static_cast<int32_t>(query.y());
    point.z = static_cast<int32_t>(query.z());

    point.intensity = static_cast<uint16_t>(query.intensity() * f16);

    point.return_number = query.returnNumber();
    point.number_of_returns = query.numberOfReturns();
    point.classification = query.classification();
    point.user_data = query.userData();

    point.gps_time = query.gpsTime();

    point.red = static_cast<uint16_t>(query.red() * f16);
    point.green = static_cast<uint16_t>(query.green() * f16);
    point.blue = static_cast<uint16_t>(query.blue() * f16);

    point.user_layer = static_cast<uint32_t>(query.layer());
    point.user_elevation = static_cast<uint32_t>(query.elevation());

    point.user_red = static_cast<uint16_t>(query.customRed() * f16);
    point.user_green = static_cast<uint16_t>(query.customGreen() * f16);
    point.user_blue = static_cast<uint16_t>(query.customBlue() * f16);

    point.user_descriptor = query.descriptor();

    // Write new point to file
    file_.writePoint(point);
}

void ExportFileFormatLas::close()
{
    LOG_DEBUG(<< "Called.");

    // Close the file
    file_.close();
}
