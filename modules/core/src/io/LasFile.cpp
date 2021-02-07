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
    @file LasFile.cpp
*/

#include <Endian.hpp>
#include <Error.hpp>
#include <LasFile.hpp>
#include <cstring>
#include <iostream>
#include <sstream>

#define LAS_FILE_SIGNATURE_0 0x4C
#define LAS_FILE_SIGNATURE_1 0x41
#define LAS_FILE_SIGNATURE_2 0x53
#define LAS_FILE_SIGNATURE_3 0x46
#define LAS_FILE_HEADER_SIZE_V10 227
#define LAS_FILE_HEADER_SIZE_V13 235
#define LAS_FILE_HEADER_SIZE_V14 375
#define LAS_FILE_FORMAT_COUNT 11
static const int LAS_FILE_FORMAT_BYTE_COUNT[LAS_FILE_FORMAT_COUNT] =
    {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};

LasFile::LasFile()
{
}

LasFile::~LasFile()
{
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

void LasFile::readHeader()
{
    readHeader(header);
}

void LasFile::readHeader(Header &hdr)
{
    uint8_t buffer[256];

    if (file_.size() < LAS_FILE_HEADER_SIZE_V10)
    {
        THROW("LAS '" + file_.path() + "' has invalid size");
    }

    file_.read(buffer, LAS_FILE_HEADER_SIZE_V10);

    // signature "LASF"
    std::memcpy(hdr.file_signature, buffer, 4);
    if ((hdr.file_signature[0] != LAS_FILE_SIGNATURE_0) ||
        (hdr.file_signature[1] != LAS_FILE_SIGNATURE_1) ||
        (hdr.file_signature[2] != LAS_FILE_SIGNATURE_2) ||
        (hdr.file_signature[3] != LAS_FILE_SIGNATURE_3))
    {
        THROW("LAS '" + file_.path() + "' has invalid signature");
    }

    // file info
    hdr.file_source_id = ltoh16(&buffer[4]);
    hdr.global_encoding = ltoh16(&buffer[6]);
    hdr.project_id_1 = ltoh32(&buffer[8]);
    hdr.project_id_2 = ltoh16(&buffer[12]);
    hdr.project_id_3 = ltoh16(&buffer[14]);
    std::memcpy(hdr.project_id_4, buffer + 16, 8);

    // version
    hdr.version_major = buffer[24];
    hdr.version_minor = buffer[25];

    if (hdr.version_major != 1)
    {
        THROW("LAS '" + file_.path() + "' has incompatible major version");
    }

    // software/hardware generated
    std::memcpy(hdr.system_identifier, buffer + 26, 32);
    std::memcpy(hdr.generating_software, buffer + 58, 32);

    // time
    hdr.file_creation_day_of_year = ltoh16(&buffer[90]);
    hdr.file_creation_year = ltoh16(&buffer[92]);

    // header
    hdr.header_size = ltoh16(&buffer[94]);
    hdr.offset_to_point_data = ltoh32(&buffer[96]);
    hdr.number_of_vlr = ltoh32(&buffer[100]);
    hdr.point_data_record_format = buffer[104];
    hdr.point_data_record_length = ltoh16(&buffer[105]);

    if (hdr.point_data_record_format >= LAS_FILE_FORMAT_COUNT)
    {
        THROW("LAS '" + file_.path() + "' has unknown record format");
    }

    // number of point records,
    // fill both 32-bit (1.0+) and 64-bit (1.4+) values
    hdr.legacy_number_of_point_records = ltoh32(&buffer[107]);
    hdr.number_of_point_records = hdr.legacy_number_of_point_records;
    for (int i = 0; i < 5; i++)
    {
        uint32_t number = ltoh32(&buffer[111 + (i * 4)]);
        hdr.legacy_number_of_points_by_return[i] = number;
        hdr.number_of_points_by_return[i] = number;
    }

    // scale
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

    // version 1.3
    if (hdr.version_minor > 2)
    {
        if (file_.size() < LAS_FILE_HEADER_SIZE_V13)
        {
            THROW("LAS '" + file_.path() + "' v1.3+ has invalid size");
        }

        file_.read(buffer, 8);
        hdr.offset_to_wdpr = ltoh64(&buffer[0]);
    }

    // version 1.4
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
}

void LasFile::writeHeader()
{
    writeHeader(header);
}

void LasFile::writeHeader(const Header &hdr)
{
    uint8_t buffer[512];
    uint32_t header_size;

    // signature
    std::memcpy(buffer, hdr.file_signature, 4);

    // file info
    htol16(&buffer[4], hdr.file_source_id);
    htol16(&buffer[6], hdr.global_encoding);
    htol32(&buffer[8], hdr.project_id_1);
    htol16(&buffer[12], hdr.project_id_2);
    htol16(&buffer[14], hdr.project_id_3);
    std::memcpy(buffer + 16, hdr.project_id_4, 8);

    // version
    buffer[24] = hdr.version_major;
    buffer[25] = hdr.version_minor;

    // software/hardware generated
    std::memcpy(buffer + 26, hdr.system_identifier, 32);
    std::memcpy(buffer + 58, hdr.generating_software, 32);

    // time
    htol16(&buffer[90], hdr.file_creation_day_of_year);
    htol16(&buffer[92], hdr.file_creation_year);

    // header
    htol16(&buffer[94], hdr.header_size);
    htol32(&buffer[96], hdr.offset_to_point_data);
    htol32(&buffer[100], hdr.number_of_vlr);
    buffer[104] = hdr.point_data_record_format;
    htol16(&buffer[105], hdr.point_data_record_length);

    // number of point records
    htol32(&buffer[107], hdr.legacy_number_of_point_records);
    for (int i = 0; i < 5; i++)
    {
        htol32(&buffer[111 + i * 4], hdr.legacy_number_of_points_by_return[i]);
    }

    // scale
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

    // version 1.3
    if (hdr.version_minor > 2)
    {
        htol64(&buffer[0], hdr.offset_to_wdpr);
        header_size = LAS_FILE_HEADER_SIZE_V13;
    }

    // version 1.4
    if (hdr.version_minor > 3)
    {
        htol64(&buffer[0], hdr.offset_to_evlr);
        htol32(&buffer[8], hdr.number_of_evlr);
        htol64(&buffer[12], hdr.number_of_point_records);
        for (int i = 0; i < 15; i++)
        {
            htol64(&buffer[20 + (i * 8)], hdr.number_of_points_by_return[i]);
        }
        header_size = LAS_FILE_HEADER_SIZE_V14;
    }

    // write
    file_.write(buffer, header_size);
}

void LasFile::read(uint8_t *buffer)
{
    file_.read(buffer, header.point_data_record_length);
}

void LasFile::read(Point &pt)
{
    uint8_t buffer[256];

    read(buffer);
    read(pt, buffer, header.point_data_record_format);
}

void LasFile::read(Point &pt, const uint8_t *buffer, uint8_t fmt) const
{
    size_t pos;

    // TBD this is without optimization, all points have the same format 'fmt'

    pt.format = fmt;

    if (fmt > 5)
    {
        pt.x = ltoh32(&buffer[0]);
        pt.y = ltoh32(&buffer[4]);
        pt.z = ltoh32(&buffer[8]);
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
        pt.x = ltoh32(&buffer[0]);
        pt.y = ltoh32(&buffer[4]);
        pt.z = ltoh32(&buffer[8]);
        pt.intensity = ltoh16(&buffer[12]);
        uint32_t data14 = static_cast<uint32_t>(buffer[14]);
        pt.return_number = static_cast<uint8_t>(data14 & 7U);
        pt.number_of_returns = static_cast<uint8_t>((data14 >> 3) & 7U);
        pt.scan_direction_flag = static_cast<uint8_t>((data14 >> 6) & 1U);
        pt.edge_of_flight_line = static_cast<uint8_t>((data14 >> 7) & 1U);
        pt.classification = buffer[15];
        pt.angle = static_cast<int16_t>(buffer[16]);
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

size_t LasFile::getVersionHeaderSize() const
{
    size_t ret = 0;
    if (header.version_major == 1)
    {
        if (header.version_minor < 3)
        {
            ret = LAS_FILE_HEADER_SIZE_V10;
        }
        else if (header.version_minor == 3)
        {
            ret = LAS_FILE_HEADER_SIZE_V13;
        }
        else if (header.version_minor > 3)
        {
            ret = LAS_FILE_HEADER_SIZE_V14;
        }
    }
    return ret;
}

size_t LasFile::getPointDataRecordUserLength() const
{
    int ret = static_cast<int>(header.point_data_record_length) -
              LAS_FILE_FORMAT_BYTE_COUNT[header.point_data_record_format];

    if (ret < 0)
    {
        THROW("LAS '" + file_.path() +
              "' has invalid record length "
              "per record format");
    }

    return static_cast<size_t>(ret);
}

bool LasFile::hasRgb() const
{
    switch (header.point_data_record_format)
    {
        case 2:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
            return true;
        default:
            return false;
    }
}

std::string LasFile::dateCreated() const
{
    // TBD
    return "";
}

Json &LasFile::Point::write(Json &out) const
{
    out["coordinates"][0] = x;
    out["coordinates"][1] = y;
    out["coordinates"][2] = z;
    out["return_number"] = return_number;
    out["number_of_returns"] = number_of_returns;
    out["classification"] = classification;

    return out;
}

Json &LasFile::Header::write(Json &out) const
{
    out["version"][0] = version_major;
    out["version"][1] = version_minor;
    out["header_size"] = header_size;
    out["offset_to_point_data"] = offset_to_point_data;
    out["point_data_record_format"] = point_data_record_format;
    out["point_data_record_length"] = point_data_record_length;
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

void LasFile::randomize(const std::string &outputPath,
                        const std::string &inputPath)
{
    uint64_t n;
    uint64_t start;
    uint64_t pointSize;
    uint64_t r;
    uint64_t rstate = 10;
    uint8_t buffer_1[256];
    uint8_t buffer_2[256];

    // Header
    LasFile inputLas;
    inputLas.open(inputPath);
    inputLas.readHeader();

    LasFile outputLas;
    std::string writePath = File::tmpname(outputPath, inputPath);
    outputLas.create(writePath);
    outputLas.header = inputLas.header;
    outputLas.writeHeader();

    // Copy
    n = inputLas.file_.size() - inputLas.file_.offset();
    outputLas.file_.write(inputLas.file_, n);

    // Randomize (very slow)
    n = outputLas.header.number_of_point_records;
    pointSize = outputLas.header.point_data_record_length;
    start = outputLas.header.offset_to_point_data;
    if (n > 0)
    {
        n--;

        for (uint64_t i = 0; i < n; i++)
        {
            r = i + 1 + (rstate % (n - i));

            // Linear congruent pseudo-random generator
            rstate = rstate * 69069UL + 1;

            // Read A
            outputLas.seek(start + i * pointSize);
            outputLas.file_.read(buffer_1, pointSize);

            // Read B, Overwrite B with A
            outputLas.seek(start + r * pointSize);
            outputLas.file_.read(buffer_2, pointSize);
            outputLas.seek(start + r * pointSize);
            outputLas.file_.write(buffer_1, pointSize);

            // Overwrite A with B
            outputLas.seek(start + i * pointSize);
            outputLas.file_.write(buffer_2, pointSize);
        }
    }

    // Close
    inputLas.close();
    outputLas.close();

    File::move(outputPath, writePath);
}

void LasFile::format(const std::string &outputPath,
                     const std::string &inputPath)
{
    (void)outputPath;
    (void)inputPath;
}

void LasFile::index(const std::string &outputPath,
                    const std::string &inputPath,
                    size_t maxLeafSize1,
                    size_t maxLeafSize2)
{
    (void)outputPath;
    (void)inputPath;
    (void)maxLeafSize1;
    (void)maxLeafSize2;
}

void LasFile::print(const std::string &inputPath)
{
    LasFile las;
    las.open(inputPath);
    las.readHeader();

    Json obj;
    std::cout << las.header.write(obj).serialize() << std::endl;
}
