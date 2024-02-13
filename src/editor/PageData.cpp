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

// Include 3D Forest.
#include <Editor.hpp>
#include <Endian.hpp>
#include <File.hpp>
#include <IndexFileBuilder.hpp>
#include <LasFile.hpp>
#include <PageData.hpp>

// Include local.
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

    segment.resize(n);
    elevation.resize(n);
    descriptor.resize(n);
    voxel.resize(n);

    renderPosition.resize(n * 3);
    positionBase_.resize(n * 3);
}

void PageData::readPage(Editor *editor)
{
    LOG_DEBUG(<< "Read page <" << pageId_ << "> dataset <" << datasetId_
              << ">.");

    Dataset &dataset = editor->datasets().key(datasetId_);
    const IndexFile::Node *node = dataset.index().at(pageId_);
    LasFile &las = dataset.las();

    // Read page buffer from LAS file.
    las.seekPoint(node->from);

    size_t numberOfPointsInPage = static_cast<size_t>(node->size);
    size_t pointSize = las.header.point_data_record_length;
    size_t bufferLasPageSize = pointSize * numberOfPointsInPage;
    // uint8_t fmt = las.header.point_data_record_format;
    pointDataBuffer_.resize(bufferLasPageSize);
    las.readBuffer(pointDataBuffer_.data(), bufferLasPageSize);

    // Create point data.
    resize(numberOfPointsInPage);

    // Covert buffer to point data.
    uint8_t *ptrPointData = pointDataBuffer_.data();

    LasFile::Point point;

    const double s16 = 1.0 / 65535.0;
    bool rgbFlag = las.header.hasRgb();

    for (size_t i = 0; i < numberOfPointsInPage; i++)
    {
        las.formatBytesToPoint(point, ptrPointData + (pointSize * i));

        // XYZ coordinates.
        positionBase_[3 * i + 0] = static_cast<double>(point.x);
        positionBase_[3 * i + 1] = static_cast<double>(point.y);
        positionBase_[3 * i + 2] = static_cast<double>(point.z);

        position[3 * i + 0] = positionBase_[3 * i + 0];
        position[3 * i + 1] = positionBase_[3 * i + 1];
        position[3 * i + 2] = positionBase_[3 * i + 2];

        // Intensity and color.
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

        // Attributes.
        returnNumber[i] = point.return_number;
        numberOfReturns[i] = point.number_of_returns;
        classification[i] = point.classification;
        userData[i] = point.user_data;

        // GPS.
        gpsTime[i] = point.gps_time;
    }

    // 3D Forest attributes.
    LasFile::AttributesBuffer attributes;
    las.createAttributesBuffer(attributes, numberOfPointsInPage);
    las.readAttributesBuffer(attributes, numberOfPointsInPage);
    attributes.attributes[0].read(segment);
    attributes.attributes[1].read(elevation);
    attributes.attributes[2].read(descriptor);
    attributes.attributes[3].read(voxel);

    // Read page index.
    std::string pathIndex;
    pathIndex = IndexFileBuilder::extension(dataset.path());
    octree.read(pathIndex, node->offset);
    octree.translate(dataset.translation());

    // Loaded.
    modified_ = false;

    // Apply transformation.
    transform(editor);
}

void PageData::updatePoint(uint8_t *ptr, size_t i, uint8_t fmt)
{
    // Do not overwrite the other values for now:
    // - return number
    // - gps time
    // - etc.

    // Update classification.
    if (fmt > 5)
    {
        ptr[16] = classification[i];
    }
    else
    {
        ptr[15] = static_cast<uint8_t>(
            static_cast<unsigned int>(classification[i]) & 0x1fU);
    }
}

void PageData::writePage(Editor *editor)
{
    LOG_DEBUG(<< "Write page <" << pageId_ << "> dataset <" << datasetId_
              << ">.");

    Dataset &dataset = editor->datasets().key(datasetId_);
    const IndexFile::Node *node = dataset.index().at(pageId_);
    LasFile &las = dataset.las();

    size_t pointSize = las.header.point_data_record_length;
    uint8_t fmt = las.header.point_data_record_format;

    size_t numberOfPointsInPage = static_cast<size_t>(node->size);
    uint8_t *ptrPointData = pointDataBuffer_.data();

    for (size_t i = 0; i < numberOfPointsInPage; i++)
    {
        updatePoint(ptrPointData + (pointSize * i), i, fmt);
    }

    las.seekPoint(node->from);
    las.writeBuffer(pointDataBuffer_.data(), pointDataBuffer_.size());

    // Attributes.
    LasFile::AttributesBuffer attributes;
    las.createAttributesBuffer(attributes, numberOfPointsInPage);
    attributes.attributes[0].write(segment);
    attributes.attributes[1].write(elevation);
    attributes.attributes[2].write(descriptor);
    attributes.attributes[3].write(voxel);
    las.writeAttributesBuffer(attributes, numberOfPointsInPage);

    // Clear 'modified' flag.
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
