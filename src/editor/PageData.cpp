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

/** @file PageData.cpp */

#include <Editor.hpp>
#include <Endian.hpp>
#include <File.hpp>
#include <LasFile.hpp>
#include <PageData.hpp>

#define LOG_MODULE_NAME "PageData"
#include <Log.hpp>

PageData::PageData(uint32_t datasetId, uint32_t pageId)
    : datasetId_(datasetId),
      pageId_(pageId),
      modified_(false)
{
    LOG_DEBUG(<< "Create page <" << pageId_ << "> dataset <" << datasetId_
              << ">.");
}

PageData::~PageData()
{
    LOG_DEBUG(<< "Destroy page <" << pageId_ << "> dataset <" << datasetId_
              << ">.");
}

void PageData::resize(size_t n)
{
    position.resize(n * 3);
    intensity.resize(n);
    returnNumber.resize(n);
    numberOfReturns.resize(n);
    classification.resize(n);
    userData.resize(n);
    gpsTime.resize(n);
    color.resize(n * 3);

    layer.resize(n);
    elevation.resize(n);
    customColor.resize(n * 3);
    descriptor.resize(n);
    value.resize(n);

    renderPosition.resize(n * 3);
    positionBase_.resize(n * 3);
}

void PageData::read(Editor *editor)
{
    LOG_DEBUG(<< "Read page <" << pageId_ << "> dataset <" << datasetId_
              << ">.");

    const Dataset &dataset = editor->datasets().key(datasetId_);
    const IndexFile::Node *node = dataset.index().at(pageId_);

    // Read page buffer from LAS file
    LasFile las;
    las.open(dataset.path());
    las.readHeader();

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    size_t nPagePoints = static_cast<size_t>(node->size);
    size_t bufferSize = pointSize * nPagePoints;
    uint8_t fmt = las.header.point_data_record_format;
    buffer_.resize(bufferSize);
    las.file().read(buffer_.data(), bufferSize);

    // Create point data
    resize(nPagePoints);

    // Covert buffer to point data
    uint8_t *ptr = buffer_.data();
    LasFile::Point point;
    const double s16 = 1.0 / 65535.0;
    bool rgbFlag = las.header.hasRgb();

    for (size_t i = 0; i < nPagePoints; i++)
    {
        las.readPoint(point, ptr + (pointSize * i), fmt);

        // xyz
        positionBase_[3 * i + 0] = static_cast<double>(point.x);
        positionBase_[3 * i + 1] = static_cast<double>(point.y);
        positionBase_[3 * i + 2] = static_cast<double>(point.z);

        position[3 * i + 0] = positionBase_[3 * i + 0];
        position[3 * i + 1] = positionBase_[3 * i + 1];
        position[3 * i + 2] = positionBase_[3 * i + 2];

        // intensity and color
        intensity[i] = static_cast<double>(point.intensity) * s16;

        if (rgbFlag)
        {
            color[3 * i + 0] = point.red * s16;
            color[3 * i + 1] = point.green * s16;
            color[3 * i + 2] = point.blue * s16;
        }
        else
        {
            color[3 * i + 0] = 1.0;
            color[3 * i + 1] = 1.0;
            color[3 * i + 2] = 1.0;
        }

        // attributes
        returnNumber[i] = point.return_number;
        numberOfReturns[i] = point.number_of_returns;
        classification[i] = point.classification;
        userData[i] = point.user_data;

        // gps
        gpsTime[i] = point.gps_time;

        // User extra
        layer[i] = point.user_layer;
        elevation[i] = static_cast<double>(point.user_elevation);
        customColor[3 * i + 0] = static_cast<double>(point.user_red) * s16;
        customColor[3 * i + 1] = static_cast<double>(point.user_green) * s16;
        customColor[3 * i + 2] = static_cast<double>(point.user_blue) * s16;
        descriptor[i] = point.user_descriptor;
        value[i] = static_cast<size_t>(point.user_value);
    }

    // Index
    std::string pathIndex;
    pathIndex = IndexFileBuilder::extension(dataset.path());
    octree.read(pathIndex, node->offset);
    octree.translate(dataset.translation());

    // Loaded
    modified_ = false;

    // Apply
    transform(editor);
}

#define PAGE_FORMAT_COUNT 11

static const size_t PAGE_FORMAT_USER[PAGE_FORMAT_COUNT] =
    {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};

void PageData::toPoint(uint8_t *ptr, size_t i, uint8_t fmt)
{
    const double s16 = 65535.0;
    size_t pos = PAGE_FORMAT_USER[fmt];

    // Do not overwrite the other values for now
    // - return number
    // - gps time
    // - etc.

    // Classification
    if (fmt > 5)
    {
        ptr[16] = classification[i];
    }

    // Layer
    htol32(ptr + pos, static_cast<uint32_t>(layer[i]));

    // Elevation
    htol32(ptr + pos + 4, static_cast<uint32_t>(elevation[i]));

    // Custom color
    htol16(ptr + pos + 8, static_cast<uint16_t>(customColor[3 * i + 0] * s16));
    htol16(ptr + pos + 10, static_cast<uint16_t>(customColor[3 * i + 1] * s16));
    htol16(ptr + pos + 12, static_cast<uint16_t>(customColor[3 * i + 2] * s16));

    // Descriptor
    htold(ptr + pos + 16, descriptor[i]);

    // Value
    htol64(ptr + pos + 24, static_cast<uint64_t>(value[i]));
}

void PageData::write(Editor *editor)
{
    LOG_DEBUG(<< "Write page <" << pageId_ << "> dataset <" << datasetId_
              << ">.");

    const Dataset &dataset = editor->datasets().key(datasetId_);
    const IndexFile::Node *node = dataset.index().at(pageId_);

    LasFile las;
    las.open(dataset.path());
    las.readHeader();

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    size_t n = static_cast<size_t>(node->size);
    uint8_t fmt = las.header.point_data_record_format;

    uint8_t *ptr = buffer_.data();

    for (size_t i = 0; i < n; i++)
    {
        toPoint(ptr + (pointSize * i), i, fmt);
    }

    las.file().write(buffer_.data(), buffer_.size());

    modified_ = false;
}

void PageData::transform(Editor *editor)
{
    const Dataset &dataset = editor->datasets().key(datasetId_);
    size_t n = positionBase_.size() / 3;
    double x;
    double y;
    double z;
    double tx = dataset.translation()[0];
    double ty = dataset.translation()[1];
    double tz = dataset.translation()[2];

    for (size_t i = 0; i < n; i++)
    {
        x = positionBase_[3 * i + 0] + tx;
        y = positionBase_[3 * i + 1] + ty;
        z = positionBase_[3 * i + 2] + tz;

        renderPosition[3 * i + 0] = static_cast<float>(x);
        renderPosition[3 * i + 1] = static_cast<float>(y);
        renderPosition[3 * i + 2] = static_cast<float>(z);

        position[3 * i + 0] = x;
        position[3 * i + 1] = y;
        position[3 * i + 2] = z;
    }

    box.set(position);
}
