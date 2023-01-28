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

/** @file LasFile.cpp */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#include <Box.hpp>
#include <Endian.hpp>
#include <Error.hpp>
#include <LasFile.hpp>

#define LOG_MODULE_NAME "LasFile"
#include <Log.hpp>

#define LAS_FILE_SIGNATURE_0 0x4C
#define LAS_FILE_SIGNATURE_1 0x41
#define LAS_FILE_SIGNATURE_2 0x53
#define LAS_FILE_SIGNATURE_3 0x46
#define LAS_FILE_HEADER_SIZE_V10 227
#define LAS_FILE_HEADER_SIZE_V13 235
#define LAS_FILE_HEADER_SIZE_V14 375
#define LAS_FILE_FORMAT_COUNT 11
#define LAS_FILE_USER_BYTE_COUNT 24U

static const size_t LAS_FILE_FORMAT_BYTE_COUNT[LAS_FILE_FORMAT_COUNT] =
    {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};

static const uint8_t LAS_FILE_FORMAT_GPS_TIME[LAS_FILE_FORMAT_COUNT] =
    {0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};

static const uint8_t LAS_FILE_FORMAT_RGB[LAS_FILE_FORMAT_COUNT] =
    {0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1};

static const uint8_t LAS_FILE_FORMAT_NIR[LAS_FILE_FORMAT_COUNT] =
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1};

static const uint8_t LAS_FILE_FORMAT_WAVE[LAS_FILE_FORMAT_COUNT] =
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1};

static const char *LAS_FILE_GENERATING_SOFTWARE = "3D Forest 2022.12.04";

uint8_t LasFile::Format::las() const
{
    uint8_t result = 6U;

    if (has(LasFile::FORMAT_RGB))
    {
        result = 7U;
    }

    return result;
}

void LasFile::Header::set(uint64_t numberOfPoints,
                          const Box<double> &box,
                          const std::array<double, 3> scale,
                          const std::array<double, 3> offset,
                          uint8_t pointFormat,
                          uint8_t versionMinor)
{
    std::memset(this, 0, sizeof(LasFile::Header));

    file_signature[0] = LAS_FILE_SIGNATURE_0;
    file_signature[1] = LAS_FILE_SIGNATURE_1;
    file_signature[2] = LAS_FILE_SIGNATURE_2;
    file_signature[3] = LAS_FILE_SIGNATURE_3;

    version_major = 1;
    version_minor = versionMinor;
    setGeneratingSoftware();

    if (version_minor > 3)
    {
        header_size = LAS_FILE_HEADER_SIZE_V14;
    }
    else if (version_minor > 2)
    {
        header_size = LAS_FILE_HEADER_SIZE_V13;
    }
    else
    {
        header_size = LAS_FILE_HEADER_SIZE_V10;
    }

    offset_to_point_data = header_size;

    point_data_record_format = pointFormat;
    point_data_record_length =
        static_cast<uint16_t>(pointDataRecordLength3dForest());

    number_of_point_records = numberOfPoints;

    uint32_t maxPoints = std::numeric_limits<uint32_t>::max();
    if (numberOfPoints > maxPoints)
    {
        legacy_number_of_point_records = maxPoints;
    }
    else
    {
        legacy_number_of_point_records = static_cast<uint32_t>(numberOfPoints);
    }

    x_scale_factor = scale[0];
    y_scale_factor = scale[1];
    z_scale_factor = scale[2];

    x_offset = offset[0];
    y_offset = offset[1];
    z_offset = offset[2];

    // Extents of point file data
    max_x = static_cast<double>(box.max(0)) * scale[0] + offset[0];
    min_x = static_cast<double>(box.min(0)) * scale[0] + offset[0];
    max_y = static_cast<double>(box.max(1)) * scale[1] + offset[1];
    min_y = static_cast<double>(box.min(1)) * scale[1] + offset[1];
    max_z = static_cast<double>(box.max(2)) * scale[2] + offset[2];
    min_z = static_cast<double>(box.min(2)) * scale[2] + offset[2];
}

size_t LasFile::Header::versionHeaderSize() const
{
    size_t ret = 0;
    if (version_major == 1)
    {
        if (version_minor < 3)
        {
            ret = LAS_FILE_HEADER_SIZE_V10;
        }
        else if (version_minor == 3)
        {
            ret = LAS_FILE_HEADER_SIZE_V13;
        }
        else if (version_minor > 3)
        {
            ret = LAS_FILE_HEADER_SIZE_V14;
        }
    }
    return ret;
}

size_t LasFile::Header::pointDataRecordLengthFormat() const
{
    return LAS_FILE_FORMAT_BYTE_COUNT[point_data_record_format];
}

size_t LasFile::Header::pointDataRecordLength3dForest() const
{
    return pointDataRecordLengthFormat() + LAS_FILE_USER_BYTE_COUNT;
}

size_t LasFile::Header::pointDataRecordLengthUser() const
{
    return static_cast<size_t>(point_data_record_length) -
           LAS_FILE_FORMAT_BYTE_COUNT[point_data_record_format];
}

uint64_t LasFile::Header::pointDataSize() const
{
    return static_cast<uint64_t>(point_data_record_length) *
           number_of_point_records;
}

bool LasFile::Header::hasRgb() const
{
    return LAS_FILE_FORMAT_RGB[point_data_record_format];
}

std::string LasFile::Header::dateCreated() const
{
    // GMT day
    int day = file_creation_day_of_year;
    if (day < 1)
    {
        day = 1;
    }

    // Not a leap year
    if (!((file_creation_year % 400 == 0) ||
          ((file_creation_year % 4 == 0) && (file_creation_year % 100 != 0))))
    {
        // Has 29th February
        if (day > 31 + 28)
        {
            day++;
        }
    }

    // Find month
    int daysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 0; i < 12; i++)
    {
        if (day <= daysInMonth[i])
        {
            char buffer[32];
            std::snprintf(buffer,
                          sizeof(buffer),
                          "%.4d-%.2d-%.2d 00:00:00",
                          file_creation_year,
                          i + 1,
                          day);
            return buffer;
        }
        day -= daysInMonth[i];
    }

    // Default UNIX Epoch time
    return "1970-01-01 00:00:00";
}

void LasFile::Header::setGeneratingSoftware()
{
    std::memset(generating_software, 0, sizeof(generating_software));
    std::strcpy(static_cast<char *>(generating_software),
                LAS_FILE_GENERATING_SOFTWARE);
}

void LasFile::Header::addOffsetPointData(uint64_t increment)
{
    if (offset_to_point_data != 0)
    {
        offset_to_point_data += static_cast<uint32_t>(increment);
    }
}

void LasFile::Header::addOffsetWdpr(uint64_t increment)
{
    if (offset_to_wdpr != 0)
    {
        offset_to_wdpr += static_cast<uint32_t>(increment);
    }
}

void LasFile::Header::subOffsetWdpr(uint64_t decrement)
{
    if (offset_to_wdpr != 0)
    {
        offset_to_wdpr -= static_cast<uint32_t>(decrement);
    }
}

void LasFile::Header::addOffsetEvlr(uint64_t increment)
{
    if (offset_to_evlr != 0)
    {
        offset_to_evlr += static_cast<uint32_t>(increment);
    }
}

void LasFile::Header::subOffsetEvlr(uint64_t decrement)
{
    if (offset_to_evlr != 0)
    {
        offset_to_evlr -= static_cast<uint32_t>(decrement);
    }
}

LasFile::LasFile()
{
}

LasFile::~LasFile()
{
}

void LasFile::create(const std::string &path,
                     const std::vector<LasFile::Point> &points,
                     const std::array<double, 3> scale,
                     const std::array<double, 3> offset,
                     uint8_t version_minor)
{
    // Point format
    uint8_t point_data_record_format;
    if (points.size() > 0)
    {
        point_data_record_format = points[0].format;
    }
    else
    {
        point_data_record_format = 6;
    }

    // Extents of point file data
    std::vector<int32_t> coords;
    coords.resize(points.size() * 3);
    for (size_t i = 0; i < points.size(); i++)
    {
        coords[i * 3 + 0] = points[i].x;
        coords[i * 3 + 1] = points[i].y;
        coords[i * 3 + 2] = points[i].z;
    }

    Box<int32_t> box;
    box.set(coords);

    // Create file output
    LasFile las;
    las.create(path);
    las.header.set(points.size(),
                   box,
                   scale,
                   offset,
                   point_data_record_format,
                   version_minor);
    las.writeHeader();

    for (size_t i = 0; i < points.size(); i++)
    {
        las.writePoint(points[i]);
    }

    las.close();
}

void LasFile::open(const std::string &path)
{
    std::memset(&header, 0, sizeof(header));
    file_.open(path);
}

void LasFile::create(const std::string &path)
{
    std::memset(&header, 0, sizeof(header));
    file_.create(path);
}

void LasFile::close()
{
    file_.close();
}

void LasFile::seek(uint64_t offset)
{
    file_.seek(offset);
}

void LasFile::seekHeader()
{
    file_.seek(0);
}

void LasFile::seekVlr()
{
    file_.seek(header.header_size);
}

void LasFile::seekPointData()
{
    file_.seek(header.offset_to_point_data);
}

void LasFile::seekExtendedVlr()
{
    file_.seek(header.offset_to_evlr);
}

void LasFile::readHeader()
{
    readHeader(header);
}

void LasFile::readHeader(Header &hdr)
{
    LOG_DEBUG(<< "File name <" << file_.path() << "> size <" << file_.size()
              << "> bytes.");
    uint8_t buffer[256];

    if (file_.size() < LAS_FILE_HEADER_SIZE_V10)
    {
        THROW("LAS '" + file_.path() + "' has invalid size");
    }

    file_.read(buffer, LAS_FILE_HEADER_SIZE_V10);

    // Signature "LASF"
    std::memcpy(hdr.file_signature, buffer, 4);
    if ((hdr.file_signature[0] != LAS_FILE_SIGNATURE_0) ||
        (hdr.file_signature[1] != LAS_FILE_SIGNATURE_1) ||
        (hdr.file_signature[2] != LAS_FILE_SIGNATURE_2) ||
        (hdr.file_signature[3] != LAS_FILE_SIGNATURE_3))
    {
        THROW("LAS '" + file_.path() + "' has invalid signature");
    }

    // File info
    hdr.file_source_id = ltoh16(&buffer[4]);
    hdr.global_encoding = ltoh16(&buffer[6]);
    hdr.project_id_1 = ltoh32(&buffer[8]);
    hdr.project_id_2 = ltoh16(&buffer[12]);
    hdr.project_id_3 = ltoh16(&buffer[14]);
    std::memcpy(hdr.project_id_4, buffer + 16, 8);

    // Version
    hdr.version_major = buffer[24];
    hdr.version_minor = buffer[25];
    LOG_DEBUG(<< "Version major <" << static_cast<int>(hdr.version_major)
              << ">.");
    LOG_DEBUG(<< "Version minor <" << static_cast<int>(hdr.version_minor)
              << ">.");

    if (hdr.version_major != 1)
    {
        THROW("LAS '" + file_.path() + "' has incompatible major version");
    }

    // Software/hardware generated
    std::memcpy(hdr.system_identifier, buffer + 26, 32);
    std::memcpy(hdr.generating_software, buffer + 58, 32);

    // Time
    hdr.file_creation_day_of_year = ltoh16(&buffer[90]);
    hdr.file_creation_year = ltoh16(&buffer[92]);

    // Header
    hdr.header_size = ltoh16(&buffer[94]);
    hdr.offset_to_point_data = ltoh32(&buffer[96]);
    hdr.number_of_vlr = ltoh32(&buffer[100]);

    // Point format
    hdr.point_data_record_format = buffer[104];
    hdr.point_data_record_length = ltoh16(&buffer[105]);
    LOG_DEBUG(<< "Format <" << static_cast<int>(hdr.point_data_record_format)
              << ">.");
    LOG_DEBUG(<< "Record length <" << hdr.point_data_record_length
              << "> bytes.");

    if (hdr.point_data_record_format >= LAS_FILE_FORMAT_COUNT)
    {
        THROW("LAS '" + file_.path() + "' has unknown record format");
    }

    size_t length = static_cast<size_t>(hdr.point_data_record_length);
    if (length < hdr.pointDataRecordLengthFormat())
    {
        THROW("LAS '" + file_.path() +
              "' has invalid record length "
              "per record format");
    }

    // Number of point records
    // Fill both 32-bit (1.0+) and 64-bit (1.4+) values
    hdr.legacy_number_of_point_records = ltoh32(&buffer[107]);
    hdr.number_of_point_records = hdr.legacy_number_of_point_records;
    for (int i = 0; i < 5; i++)
    {
        uint32_t number = ltoh32(&buffer[111 + (i * 4)]);
        hdr.legacy_number_of_points_by_return[i] = number;
        hdr.number_of_points_by_return[i] = number;
    }
    LOG_DEBUG(<< "Number of points <" << hdr.number_of_point_records << ">.");

    // Scale
    hdr.x_scale_factor = ltohd(&buffer[131 + (0 * 8)]);
    hdr.y_scale_factor = ltohd(&buffer[131 + (1 * 8)]);
    hdr.z_scale_factor = ltohd(&buffer[131 + (2 * 8)]);
    hdr.x_offset = ltohd(&buffer[131 + (3 * 8)]);
    hdr.y_offset = ltohd(&buffer[131 + (4 * 8)]);
    hdr.z_offset = ltohd(&buffer[131 + (5 * 8)]);
    hdr.max_x = ltohd(&buffer[131 + (6 * 8)]);
    hdr.min_x = ltohd(&buffer[131 + (7 * 8)]);
    hdr.max_y = ltohd(&buffer[131 + (8 * 8)]);
    hdr.min_y = ltohd(&buffer[131 + (9 * 8)]);
    hdr.max_z = ltohd(&buffer[131 + (10 * 8)]);
    hdr.min_z = ltohd(&buffer[131 + (11 * 8)]);

    LOG_DEBUG(<< "Scale <[" << hdr.x_scale_factor << ", " << hdr.y_scale_factor
              << ", " << hdr.z_scale_factor << "]>.");
    LOG_DEBUG(<< "Offset <[" << hdr.x_offset << ", " << hdr.y_offset << ", "
              << hdr.z_offset << "]>.");
    LOG_DEBUG(<< "Min <[" << hdr.min_x << ", " << hdr.min_y << ", " << hdr.min_z
              << "]>.");
    LOG_DEBUG(<< "Max <[" << hdr.max_x << ", " << hdr.max_y << ", " << hdr.max_z
              << "]>.");

    // Version 1.3
    if (hdr.version_minor > 2)
    {
        if (file_.size() < LAS_FILE_HEADER_SIZE_V13)
        {
            THROW("LAS '" + file_.path() + "' v1.3+ has invalid size");
        }

        file_.read(buffer, 8);
        hdr.offset_to_wdpr = ltoh64(&buffer[0]);
    }
    else
    {
        hdr.offset_to_wdpr = 0;
    }

    // Version 1.4
    if (hdr.version_minor > 3)
    {
        if (file_.size() < LAS_FILE_HEADER_SIZE_V14)
        {
            THROW("LAS '" + file_.path() + "' v1.4+ has invalid size");
        }

        file_.read(buffer, 140);
        hdr.offset_to_evlr = ltoh64(&buffer[0]);
        hdr.number_of_evlr = ltoh32(&buffer[8]);
        hdr.number_of_point_records = ltoh64(&buffer[12]);
        for (int i = 0; i < 15; i++)
        {
            hdr.number_of_points_by_return[i] = ltoh64(&buffer[20 + (i * 8)]);
        }
    }
    else
    {
        hdr.offset_to_evlr = 0;
        hdr.number_of_evlr = 0;
    }
}

void LasFile::writeHeader()
{
    writeHeader(header);
}

void LasFile::writeHeader(const Header &hdr)
{
    LOG_DEBUG(<< "Called.");

    uint8_t buffer[512];
    uint32_t header_size;

    // Signature
    std::memcpy(buffer, hdr.file_signature, 4);

    // File info
    htol16(&buffer[4], hdr.file_source_id);
    htol16(&buffer[6], hdr.global_encoding);
    htol32(&buffer[8], hdr.project_id_1);
    htol16(&buffer[12], hdr.project_id_2);
    htol16(&buffer[14], hdr.project_id_3);
    std::memcpy(buffer + 16, hdr.project_id_4, 8);

    // Version
    buffer[24] = hdr.version_major;
    buffer[25] = hdr.version_minor;

    // Software/hardware generated
    std::memcpy(buffer + 26, hdr.system_identifier, 32);
    std::memcpy(buffer + 58, hdr.generating_software, 32);

    // Time
    htol16(&buffer[90], hdr.file_creation_day_of_year);
    htol16(&buffer[92], hdr.file_creation_year);

    // Header
    htol16(&buffer[94], hdr.header_size);
    htol32(&buffer[96], hdr.offset_to_point_data);
    htol32(&buffer[100], hdr.number_of_vlr);
    buffer[104] = hdr.point_data_record_format;
    htol16(&buffer[105], hdr.point_data_record_length);

    // Number of point records
    htol32(&buffer[107], hdr.legacy_number_of_point_records);
    for (int i = 0; i < 5; i++)
    {
        htol32(&buffer[111 + i * 4], hdr.legacy_number_of_points_by_return[i]);
    }

    // Scale
    htold(&buffer[131 + (0 * 8)], hdr.x_scale_factor);
    htold(&buffer[131 + (1 * 8)], hdr.y_scale_factor);
    htold(&buffer[131 + (2 * 8)], hdr.z_scale_factor);
    htold(&buffer[131 + (3 * 8)], hdr.x_offset);
    htold(&buffer[131 + (4 * 8)], hdr.y_offset);
    htold(&buffer[131 + (5 * 8)], hdr.z_offset);
    htold(&buffer[131 + (6 * 8)], hdr.max_x);
    htold(&buffer[131 + (7 * 8)], hdr.min_x);
    htold(&buffer[131 + (8 * 8)], hdr.max_y);
    htold(&buffer[131 + (9 * 8)], hdr.min_y);
    htold(&buffer[131 + (10 * 8)], hdr.max_z);
    htold(&buffer[131 + (11 * 8)], hdr.min_z);

    header_size = LAS_FILE_HEADER_SIZE_V10;

    // Version 1.3
    if (hdr.version_minor > 2)
    {
        htol64(&buffer[header_size + 0], hdr.offset_to_wdpr);
        header_size = LAS_FILE_HEADER_SIZE_V13;
    }

    // Version 1.4
    if (hdr.version_minor > 3)
    {
        htol64(&buffer[header_size + 0], hdr.offset_to_evlr);
        htol32(&buffer[header_size + 8], hdr.number_of_evlr);
        htol64(&buffer[header_size + 12], hdr.number_of_point_records);
        for (size_t i = 0; i < 15; i++)
        {
            htol64(&buffer[header_size + 20 + (i * 8)],
                   hdr.number_of_points_by_return[i]);
        }
        header_size = LAS_FILE_HEADER_SIZE_V14;
    }

    // Write
    file_.write(buffer, header_size);
}

void LasFile::readPoint(Point &pt)
{
    uint8_t buffer[256];

    readPoint(buffer);
    readPoint(pt, buffer, header.point_data_record_format);
}

void LasFile::readPoint(uint8_t *buffer)
{
    file_.read(buffer, header.point_data_record_length);
}

void LasFile::readPoint(Point &pt, const uint8_t *buffer, uint8_t fmt) const
{
    size_t pos;

    /** @todo Optimization, all points have the same format 'fmt'. */

    pt.format = fmt;

    if (fmt > 5)
    {
        pt.x = static_cast<int32_t>(ltoh32(&buffer[0]));
        pt.y = static_cast<int32_t>(ltoh32(&buffer[4]));
        pt.z = static_cast<int32_t>(ltoh32(&buffer[8]));
        pt.intensity = ltoh16(&buffer[12]);
        uint32_t data14 = static_cast<uint32_t>(buffer[14]);
        pt.return_number = static_cast<uint8_t>(data14 & 15U);
        pt.number_of_returns = static_cast<uint8_t>((data14 >> 4) & 15U);
        uint32_t data15 = static_cast<uint32_t>(buffer[15]);
        pt.classification_flags = static_cast<uint8_t>(data15 & 15U);
        pt.scanner_channel = static_cast<uint8_t>((data15 >> 4) & 3U);
        pt.scan_direction_flag = static_cast<uint8_t>((data15 >> 6) & 1U);
        pt.edge_of_flight_line = static_cast<uint8_t>((data15 >> 7) & 1U);
        pt.classification = buffer[16];
        pt.user_data = buffer[17];
        pt.angle = static_cast<int16_t>(ltoh16(&buffer[18]));
        pt.source_id = ltoh16(&buffer[20]);
        pos = 22;
    }
    else
    {
        pt.x = static_cast<int32_t>(ltoh32(&buffer[0]));
        pt.y = static_cast<int32_t>(ltoh32(&buffer[4]));
        pt.z = static_cast<int32_t>(ltoh32(&buffer[8]));
        pt.intensity = ltoh16(&buffer[12]);
        uint32_t data14 = static_cast<uint32_t>(buffer[14]);
        pt.return_number = static_cast<uint8_t>(data14 & 7U);
        pt.number_of_returns = static_cast<uint8_t>((data14 >> 3) & 7U);
        pt.scan_direction_flag = static_cast<uint8_t>((data14 >> 6) & 1U);
        pt.edge_of_flight_line = static_cast<uint8_t>((data14 >> 7) & 1U);
        pt.classification = buffer[15] & 0x1fU;
        pt.classification_flags =
            static_cast<uint8_t>(static_cast<uint32_t>(buffer[15]) >> 5);

        // Read as -15000 to 15000 from -90 to 90
        int8_t angle = static_cast<int8_t>(buffer[16]);
        double angled = 166.666667 * static_cast<double>(angle);
        pt.angle = static_cast<int16_t>(angled);

        pt.user_data = buffer[17];
        pt.source_id = ltoh16(&buffer[18]);
        pos = 20;
    }

    if (fmt == 1 || fmt > 2)
    {
        pt.gps_time = ltohd(&buffer[pos]);
        pos += 8;
    }

    if (fmt == 2 || fmt == 3 || fmt == 5 || fmt == 7 || fmt == 8 || fmt == 10)
    {
        pt.red = ltoh16(&buffer[pos]);
        pt.green = ltoh16(&buffer[pos + 2]);
        pt.blue = ltoh16(&buffer[pos + 4]);
        pos += 6;
    }

    if (fmt == 8 || fmt == 10)
    {
        pt.nir = ltoh16(&buffer[pos]);
        pos += 2;
    }

    if (fmt == 4 || fmt == 5 || fmt == 9 || fmt == 10)
    {
        pt.wave_index = buffer[pos];
        pt.wave_offset = ltoh64(&buffer[pos + 1]);
        pt.wave_size = ltoh32(&buffer[pos + 9]);
        pt.wave_return = ltohf(&buffer[pos + 13]);
        pt.wave_x = ltohf(&buffer[pos + 17]);
        pt.wave_y = ltohf(&buffer[pos + 21]);
        pt.wave_z = ltohf(&buffer[pos + 25]);
        pos += 29;
    }

    if (header.point_data_record_length >= (pos + LAS_FILE_USER_BYTE_COUNT))
    {
        pt.user_layer = ltoh32(&buffer[pos]);
        pt.user_elevation = ltoh32(&buffer[pos + 4]);
        pt.user_red = buffer[pos + 8];
        pt.user_green = buffer[pos + 9];
        pt.user_blue = buffer[pos + 10];
        pt.user_descriptor = buffer[pos + 11];
        pt.user_density = buffer[pos + 12];
        pt.user_nx = buffer[pos + 13];
        pt.user_ny = buffer[pos + 14];
        pt.user_nz = buffer[pos + 15];
        pt.user_value = ltoh64(&buffer[pos + 16]);
    }
}

void LasFile::writePoint(const Point &pt)
{
    uint8_t buffer[256];

    writePoint(buffer, pt);
    file_.write(buffer, header.point_data_record_length);
}

void LasFile::writePoint(uint8_t *buffer, const Point &pt) const
{
    const uint8_t fmt = pt.format;
    size_t pos;

    /** @todo Optimization, all points have the same format 'fmt'. */

    if (fmt > 5)
    {
        htol32(&buffer[0], static_cast<uint32_t>(pt.x));
        htol32(&buffer[4], static_cast<uint32_t>(pt.y));
        htol32(&buffer[8], static_cast<uint32_t>(pt.z));
        htol16(&buffer[12], pt.intensity);

        // Return Number        4 bits (bits 0 - 3)
        // Number of Returns    4 bits (bits 4 - 7)
        uint32_t data14 =
            (static_cast<uint32_t>(pt.return_number) & 15U) |
            ((static_cast<uint32_t>(pt.number_of_returns) & 15U) << 4);
        buffer[14] = static_cast<uint8_t>(data14);

        // Classification Flags 4 bits (bits 0 - 3)
        // Scanner Channel      2 bits (bits 4 - 5)
        // Scan Direction Flag  1 bit (bit 6)
        // Edge of Flight Line  1 bit (bit 7)
        uint32_t data15 =
            (static_cast<uint32_t>(pt.classification_flags) & 15U) |
            ((static_cast<uint32_t>(pt.scanner_channel) & 3U) << 4) |
            ((static_cast<uint32_t>(pt.scan_direction_flag) & 1U) << 6) |
            ((static_cast<uint32_t>(pt.edge_of_flight_line) & 1U) << 7);
        buffer[15] = static_cast<uint8_t>(data15);

        buffer[16] = pt.classification;
        buffer[17] = pt.user_data;
        htol16(&buffer[18], static_cast<uint16_t>(pt.angle));
        htol16(&buffer[20], pt.source_id);

        pos = 22;
    }
    else
    {
        htol32(&buffer[0], static_cast<uint32_t>(pt.x));
        htol32(&buffer[4], static_cast<uint32_t>(pt.y));
        htol32(&buffer[8], static_cast<uint32_t>(pt.z));
        htol16(&buffer[12], pt.intensity);

        // Return Number        3 bits (bits 0 – 2)
        // Number of Returns    3 bits (bits 3 – 5)
        // Scan Direction Flag  1 bit  (bit 6)
        // Edge of Flight Line  1 bit  (bit 7)
        uint32_t data14 =
            (static_cast<uint32_t>(pt.return_number) & 7U) |
            ((static_cast<uint32_t>(pt.number_of_returns) & 7U) << 3) |
            ((static_cast<uint32_t>(pt.scan_direction_flag) & 1U) << 6) |
            ((static_cast<uint32_t>(pt.edge_of_flight_line) & 1U) << 7);
        buffer[14] = static_cast<uint8_t>(data14);

        // Classification       5 bits (bits 0 - 4)
        // Classification Flags 3 bits (bits 5 - 7)
        uint32_t data15 = (pt.classification & 0x1fU) |
                          (static_cast<uint32_t>(pt.classification_flags) << 5);
        buffer[15] = static_cast<uint8_t>(data15);

        // Write as -90 to 90 from -15000 to 15000
        double angled = static_cast<double>(pt.angle) / 166.666667;
        if (angled < -90.0)
        {
            angled = -90.0;
        }
        else if (angled > 90.0)
        {
            angled = 90.0;
        }
        int8_t angle = static_cast<int8_t>(angled);
        buffer[16] = static_cast<uint8_t>(angle);

        buffer[17] = pt.user_data;
        htol16(&buffer[18], pt.source_id);

        pos = 20;
    }

    if (fmt == 1 || fmt > 2)
    {
        htold(&buffer[pos], pt.gps_time);
        pos += 8;
    }

    if (fmt == 2 || fmt == 3 || fmt == 5 || fmt == 7 || fmt == 8 || fmt == 10)
    {
        htol16(&buffer[pos], pt.red);
        htol16(&buffer[pos + 2], pt.green);
        htol16(&buffer[pos + 4], pt.blue);
        pos += 6;
    }

    if (fmt == 8 || fmt == 10)
    {
        htol16(&buffer[pos], pt.nir);
        pos += 2;
    }

    if (fmt == 4 || fmt == 5 || fmt == 9 || fmt == 10)
    {
        buffer[pos] = pt.wave_index;
        htol64(&buffer[pos + 1], pt.wave_offset);
        htol32(&buffer[pos + 9], pt.wave_size);
        htolf(&buffer[pos + 13], pt.wave_return);
        htolf(&buffer[pos + 17], pt.wave_x);
        htolf(&buffer[pos + 21], pt.wave_y);
        htolf(&buffer[pos + 25], pt.wave_z);
        pos += 29;
    }

    if (header.point_data_record_length >= (pos + LAS_FILE_USER_BYTE_COUNT))
    {
        htol32(&buffer[pos], pt.user_layer);
        htol32(&buffer[pos + 4], pt.user_elevation);
        buffer[pos + 8] = pt.user_red;
        buffer[pos + 9] = pt.user_green;
        buffer[pos + 10] = pt.user_blue;
        buffer[pos + 11] = pt.user_descriptor;
        buffer[pos + 12] = pt.user_density;
        buffer[pos + 13] = pt.user_nx;
        buffer[pos + 14] = pt.user_ny;
        buffer[pos + 15] = pt.user_nz;
        htol64(&buffer[pos + 16], pt.user_value);
    }
}

void LasFile::transform(double &x, double &y, double &z, const Point &pt) const
{
    x = (static_cast<double>(pt.x) * header.x_scale_factor) + header.x_offset;
    y = (static_cast<double>(pt.y) * header.y_scale_factor) + header.y_offset;
    z = (static_cast<double>(pt.z) * header.z_scale_factor) + header.z_offset;
}

void LasFile::transform(double &x,
                        double &y,
                        double &z,
                        const uint8_t *buffer) const
{
    double px = static_cast<double>(ltoh32(&buffer[0]));
    double py = static_cast<double>(ltoh32(&buffer[4]));
    double pz = static_cast<double>(ltoh32(&buffer[8]));
    x = (px * header.x_scale_factor) + header.x_offset;
    y = (py * header.y_scale_factor) + header.y_offset;
    z = (pz * header.z_scale_factor) + header.z_offset;
}

void LasFile::transformInvert(double &x, double &y, double &z) const
{
    x = (x - header.x_offset) / header.x_scale_factor;
    y = (y - header.y_offset) / header.y_scale_factor;
    z = (z - header.z_offset) / header.z_scale_factor;
}

Json &LasFile::Point::write(Json &out) const
{
    out["coordinates"][0] = x;
    out["coordinates"][1] = y;
    out["coordinates"][2] = z;

    out["intensity"] = intensity;

    out["return_number"] = return_number;
    out["number_of_returns"] = number_of_returns;
    out["scan_direction_flag"] = scan_direction_flag;
    out["edge_of_flight_line"] = edge_of_flight_line;
    out["classification_flags"] = classification_flags;

    if (format > 5)
    {
        out["scanner_channel"] = scanner_channel;
    }

    out["angle"] = angle;
    out["source_id"] = source_id;
    out["classification"] = classification;
    out["user_data"] = user_data;

    if (LAS_FILE_FORMAT_GPS_TIME[format])
    {
        out["gps_time"] = gps_time;
    }

    if (LAS_FILE_FORMAT_RGB[format])
    {
        out["rgb"][0] = red;
        out["rgb"][1] = green;
        out["rgb"][2] = blue;
    }

    if (LAS_FILE_FORMAT_NIR[format])
    {
        out["nir"] = nir;
    }

    if (LAS_FILE_FORMAT_WAVE[format])
    {
        out["wave_index"] = wave_index;
        out["wave_size"] = wave_size;
        out["wave_offset"] = wave_offset;
        out["wave_return"] = wave_return;
        out["wave"][0] = wave_x;
        out["wave"][1] = wave_y;
        out["wave"][2] = wave_z;
    }

    return out;
}

Json &LasFile::Header::write(Json &out) const
{
    out["version"][0] = version_major;
    out["version"][1] = version_minor;
    out["generating_software"] = std::string(generating_software);
    out["file_creation"] = dateCreated();

    out["header_size"] = header_size;
    out["offset_to_point_data"] = offset_to_point_data;
    out["offset_to_wdpr"] = offset_to_wdpr;
    out["offset_to_evlr"] = offset_to_evlr;

    out["point_data_record_format"] = point_data_record_format;
    out["point_data_record_length"] = point_data_record_length;
    out["point_data_record_user_length"] = pointDataRecordLengthUser();
    out["number_of_point_records"] = number_of_point_records;

    out["scale"][0] = x_scale_factor;
    out["scale"][1] = y_scale_factor;
    out["scale"][2] = z_scale_factor;
    out["offset"][0] = x_offset;
    out["offset"][1] = y_offset;
    out["offset"][2] = z_offset;
    out["max"][0] = max_x;
    out["max"][1] = max_y;
    out["max"][2] = max_z;
    out["min"][0] = min_x;
    out["min"][1] = min_y;
    out["min"][2] = min_z;

    return out;
}

std::ostream &operator<<(std::ostream &os, const LasFile::Header &obj)
{
    Json json;
    os << obj.write(json).serialize();
    return os;
}

std::ostream &operator<<(std::ostream &os, const LasFile::Point &obj)
{
    Json json;
    os << obj.write(json).serialize();
    return os;
}

/** @class LasFile.cpp::LasFile

Point Data Record Format Table:

@verbatim
v1.0 : formats 0, 1
       16 angle                 8 bits signed (-90 to +90)
       17 file marker           8 bits
       18 user_data            16 bits

v1.1 : formats 0, 1
v1.2 : formats 0, 1, 2, 3
v1.3 : formats 0, 1, 2, 3, 4, 5
       17 user_data             8 bits
       18 source_id            16 bits

v1.4 : formats 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
       14 return_number         4 bits (new 1 bit)
       14 number_of_returns     4 bits (new 1 bit)
       15 classification_flags  4 bits (new 1 bit) *
       15 scanner_channel       2 bits (new)
       16 classification        8 bits class (new 3 bits)
       18 angle                16 bits signed (by 0.006 degrees)

|-------------------------------------------------------------------|
|    0, 20 bytes                  |    6, 30 bytes                  |
|  0 x                    32 bits |  0 x                    32 bits |
|  4 y                    32 bits |  4 y                    32 bits |
|  8 z                    32 bits |  8 z                    32 bits |
| 12 intensity *          16 bits | 12 intensity *          16 bits |
| 14 return_number        3 bits  | 14 return_number        4 bits  |
|    number_of_returns    3 bits  |    number_of_returns    4 bits  |
|                                 | 15 classification_flags 4 bits  |
|                                 |    scanner_channel      2 bits  |
|    scan_direction_flag  1 bit   |    scan_direction_flag  1 bit   |
|    edge_of_flight_line  1 bit   |    edge_of_flight_line  1 bit   |
| 15 classification       5 bits  | 16 classification       8 bits  |
|    classification_flags 3 bits  |                                 |
| 16 angle                8 bits  | 17 user_data *          8 bits  |
| 17 user_data *          8 bits  | 18 angle                16 bits |
| 18 source_id            16 bits | 20 source_id            16 bits |
|                                 | 22 gps_time             64 bits |
|---------------------------------+---------------------------------|
|    1, 28 bytes                  |                                 |
| 20 gps_time             64 bits |                                 |
|---------------------------------+---------------------------------|
|    2, 26 bytes                  |    7, 36 bytes                  |
| 20 red                  16 bits | 30 red                  16 bits |
| 22 green                16 bits | 32 green                16 bits |
| 24 blue                 16 bits | 34 blue                 16 bits |
|---------------------------------+---------------------------------|
|    3, 34 bytes                  |    8, 38 bytes                  |
| 20 gps_time             64 bits | 30 red                  16 bits |
| 28 red                  16 bits | 32 green                16 bits |
| 30 green                16 bits | 34 blue                 16 bits |
| 32 blue                 16 bits | 36 NIR                  16 bits |
|---------------------------------+---------------------------------|
|    4, 57 bytes                  |                                 |
| 20 gps_time             64 bits |    9, 59 bytes                  |
| 28 wave_index            8 bits | 30 wave_index            8 bits |
| 29 wave_offset          64 bits | 31 wave_offset          64 bits |
| 37 wave_size            32 bits | 39 wave_size            32 bits |
| 41 wave_return          32 bits | 43 wave_return          32 bits |
| 45 wave_x               32 bits | 47 wave_x               32 bits |
| 49 wave_y               32 bits | 51 wave_y               32 bits |
| 53 wave_z               32 bits | 55 wave_z               32 bits |
|---------------------------------+---------------------------------|
|    5, 63 bytes                  |                                 |
| 20 gps_time             64 bits |    10, 67 bytes                 |
| 28 red                  16 bits | 30 red                  16 bits |
| 30 green                16 bits | 32 green                16 bits |
| 32 blue                 16 bits | 34 blue                 16 bits |
|                                 | 36 NIR                  16 bits |
| 34 wave_index            8 bits | 38 wave_index            8 bits |
| 35 wave_offset          64 bits | 39 wave_offset          64 bits |
| 43 wave_size            32 bits | 47 wave_size            32 bits |
| 47 wave_return          32 bits | 51 wave_return          32 bits |
| 51 wave_x               32 bits | 55 wave_x               32 bits |
| 55 wave_y               32 bits | 59 wave_y               32 bits |
| 59 wave_z               32 bits | 63 wave_z               32 bits |
|-------------------------------------------------------------------|
@endverbatim
*/
