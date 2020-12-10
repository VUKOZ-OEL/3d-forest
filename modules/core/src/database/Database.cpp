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

/**
    @file Database.cpp
*/

#include <Database.hpp>
#include <limits>

Database::Database()
{
}

Database::~Database()
{
}

void Database::open(const std::string &path)
{
    LasFile las_;
    las_.open(path);

    aabb.set(las_.header.min_x,
             las_.header.min_y,
             las_.header.min_z,
             las_.header.max_x,
             las_.header.max_y,
             las_.header.max_z);

    std::shared_ptr<DatabaseCell> cell = std::make_shared<DatabaseCell>();

    uint64_t npoints = las_.header.number_of_point_records;
    LasFile::Point point;
    double x;
    double y;
    double z;
    bool rgbFlag = las_.header.hasRgb();
    constexpr float scaleU16 =
        1.F / static_cast<float>(std::numeric_limits<uint16_t>::max());

    // Data
    std::vector<double> &xyz = cell->xyz;
    std::vector<float> &rgb = cell->rgb;
    xyz.resize(npoints * 3);
    if (rgbFlag)
    {
        rgb.resize(npoints * 3);
    }

    // Read data
    for (uint64_t i = 0; i < npoints; i++)
    {
        las_.read(point);
        las_.transform(x, y, z, point);

        xyz[3 * i + 0] = x;
        xyz[3 * i + 1] = y;
        xyz[3 * i + 2] = z;

        if (rgbFlag)
        {
            rgb[3 * i + 0] = point.red * scaleU16;
            rgb[3 * i + 1] = point.green * scaleU16;
            rgb[3 * i + 2] = point.blue * scaleU16;
        }
    }

    cells_.push_back(cell);
}

void Database::close()
{
    cells_.clear();
    // las_.close();
}

// size_t Database::map(uint64_t index)
//{
//    return 0;
//}
