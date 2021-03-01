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
    maxCache_ = 8;
    loaded_ = 0;
    maximum_ = 0;
}

Database::~Database()
{
}

void Database::openDataSet(uint64_t id, const std::string &path)
{
    std::shared_ptr<DatabaseDataSet> dataSet;
    dataSet = std::make_shared<DatabaseDataSet>();
    dataSet->read(id, path);
    maximum_ = dataSet->index_.size();
    dataSets_.push_back(dataSet);
}

void Database::clear()
{
    loaded_ = 0;
    dataSets_.clear();
    cells_.clear();
}

void Database::updateView()
{
    if (dataSets_.size() < 1)
    {
        return;
    }

    if (loaded_ >= maximum_)
    {
        return;
    }

    DatabaseDataSet *dataSet = dataSets_[0].get();
    const OctreeIndex::Node *node = dataSet->index_.at(loaded_);

    std::shared_ptr<DatabaseCell> cell = std::make_shared<DatabaseCell>();
    LasFile las;
    las.open(dataSet->path_);
    las.readHeader();

    size_t n = static_cast<size_t>(node->size);
    bool rgbFlag = las.header.hasRgb();
    std::vector<double> &xyz = cell->xyz;
    std::vector<float> &rgb = cell->rgb;
    xyz.resize(n * 3);
    if (rgbFlag)
    {
        rgb.resize(n * 3);
    }

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    std::vector<uint8_t> buffer;
    size_t bufferSize = pointSize * n;
    uint8_t fmt = las.header.point_data_record_format;
    buffer.resize(bufferSize);
    las.file().read(buffer.data(), bufferSize);

    uint8_t *ptr = buffer.data();
    LasFile::Point point;
    double x;
    double y;
    double z;
    constexpr float scaleU16 =
        1.F / static_cast<float>(std::numeric_limits<uint16_t>::max());
    for (size_t i = 0; i < n; i++)
    {
        las.readPoint(point, ptr + (pointSize * i), fmt);

        las.transform(x, y, z, point);

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

    loaded_++;
}

// size_t Database::map(uint64_t index)
//{
//    return 0;
//}
