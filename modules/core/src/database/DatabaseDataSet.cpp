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
    @file DatabaseDataSet.cpp
*/

#include <DatabaseBuilder.hpp>
#include <DatabaseDataSet.hpp>

DatabaseDataSet::DatabaseDataSet()
{
}

DatabaseDataSet::~DatabaseDataSet()
{
}

void DatabaseDataSet::read(size_t id, const std::string &path, bool enabled)
{
    id_ = id;
    path_ = path;
    enabled_ = enabled;

    const std::string pathL1 = DatabaseBuilder::extensionL1(path_);
    index_.read(pathL1);

    LasFile las;
    las.open(path_);
    las.readHeader();

    double x1 = las.header.min_x;
    double y1 = las.header.min_y;
    double z1 = las.header.min_z;
    double x2 = las.header.max_x;
    double y2 = las.header.max_y;
    double z2 = las.header.max_z;

    boundary_.set(x1, y1, z1, x2, y2, z2);

    las.transformInvert(x1, y1, z1);
    las.transformInvert(x2, y2, z2);
    x1 += las.header.x_offset;
    y1 += las.header.y_offset;
    z1 += las.header.z_offset;
    x2 += las.header.x_offset;
    y2 += las.header.y_offset;
    z2 += las.header.z_offset;
    boundaryView_.set(x1, y1, z1, x2, y2, z2);
}
