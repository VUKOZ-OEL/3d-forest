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

/** @file FileLas.hpp */

#ifndef FILE_LAS_HPP
#define FILE_LAS_HPP

#include <File.hpp>
#include <Json.hpp>
#include <string>
#include <vector>

/** LAS (LASer) File Format. */
class FileLas
{
public:
    /** LAS Header. */
    struct Header
    {
        char file_signature[4];

        // Offset 4, size 20
        uint16_t file_source_id;  // 1.1
        uint16_t global_encoding; // 1.2
        uint32_t project_id_1;    // Optional
        uint16_t project_id_2;    // Optional
        uint16_t project_id_3;    // Optional
        uint8_t project_id_4[8];  // Optional

        // Offset 24, size 70
        uint8_t version_major;
        uint8_t version_minor;
        char system_identifier[32];
        char generating_software[32];
        uint16_t file_creation_day_of_year; // Optional, 1.3 required
        uint16_t file_creation_year;        // Optional 4 digits, 1.3 required

        // Offset 94, size 37
        uint16_t header_size;
        uint32_t offset_to_point_data;
        uint32_t number_of_vlr;
        uint8_t point_data_record_format;
        uint16_t point_data_record_length; // Bytes per point (extra data)
        uint32_t legacy_number_of_point_records;
        uint32_t legacy_number_of_points_by_return[5];

        // Offset 131, size 96
        double x_scale_factor;
        double y_scale_factor;
        double z_scale_factor;
        double x_offset;
        double y_offset;
        double z_offset;
        double max_x;
        double min_x;
        double max_y;
        double min_y;
        double max_z;
        double min_z;
        // End of v 1.0, 1.1, 1.2 (227 bytes)

        // Offset 227, size 8
        uint64_t offset_to_wdpr;
        // End of 1.3 (235 bytes)

        // Offset 235, size 140
        uint64_t offset_to_evlr;
        uint32_t number_of_evlr;
        uint64_t number_of_point_records;
        uint64_t number_of_points_by_return[15];
        // End of 1.4 (375 bytes)

        size_t versionHeaderSize() const;
        size_t pointDataRecordLengthFormat() const;
        size_t pointDataRecordLength3dForest() const;
        size_t pointDataRecordLengthUser() const;
        uint64_t pointDataSize() const;
        std::string dateCreated() const;
        bool hasRgb() const;

        void setGeneratingSoftware();
        void addOffsetPointData(uint64_t increment);
        void addOffsetWdpr(uint64_t increment);
        void subOffsetWdpr(uint64_t decrement);
        void addOffsetEvlr(uint64_t increment);
        void subOffsetEvlr(uint64_t decrement);
        Json &write(Json &out) const;
    };

    /** LAS Point. */
    struct Point
    {
        // Format 0 to 10
        uint32_t x;
        uint32_t y; // 1*8
        uint32_t z;
        uint16_t intensity; // Optional

        // Format 0 to 10
        uint8_t return_number;        // 0 to 7 or 0 to 15
        uint8_t number_of_returns;    // 0 to 7 or 0 to 15, 2 * 8
        uint8_t scan_direction_flag;  // 0 or 1
        uint8_t edge_of_flight_line;  // 0 or 1
        uint8_t classification_flags; // 3 or 4 bit bitmask

        // Format 6 to 10
        uint8_t scanner_channel; // 0 to 3

        // Format 0 to 10
        int16_t angle;
        uint16_t source_id;     // v1.0 user_data, 3 * 8
        uint8_t classification; // 0 to 31 or 0 to 255, Sometimes optional
        uint8_t user_data;      // Optional, v1.0 file_marker
        uint8_t format;         // Copy of Header::point_data_record_format

        // Format 4, 5, 9, 10
        uint8_t wave_index;
        uint32_t wave_size; // 4 * 8

        // Format 1, 3, 4, 5, 6, 7, 8, 9, 10
        double gps_time; // 5 * 8

        // Format 2, 3, 5, 7, 8, 10
        uint16_t red;
        uint16_t green;
        uint16_t blue;

        // Format 8, 10
        uint16_t nir; // 6 * 8

        // Format 4, 5, 9, 10
        uint64_t wave_offset; // 7 * 8
        float wave_return;
        float wave_x; // 8 * 8
        float wave_y;
        float wave_z; // 9 * 8

        // User-specific extra bytes
        uint32_t user_layer;
        uint16_t user_red;
        uint16_t user_green; // 10 * 8
        uint16_t user_blue;
        uint16_t user_intensity;

        uint32_t reserved1; // 11 * 8

        Json &write(Json &out) const;
    };

    /** LAS Classification. */
    enum Classification
    {
        CLASS_NEVER_CLASSIFIED,
        CLASS_UNASSIGNED,
        CLASS_GROUND,
        CLASS_LOW_VEGETATION,
        CLASS_MEDIUM_VEGETATION,
        CLASS_HIGH_VEGETATION,
        CLASS_BUILDING
    };

    Header header;

    FileLas();
    ~FileLas();

    static void create(const std::string &path,
                       const std::vector<FileLas::Point> &points,
                       const std::array<double, 3> &scale = {1, 1, 1},
                       const std::array<double, 3> &offset = {0, 0, 0},
                       uint8_t version_minor = 4);

    void open(const std::string &path);
    void create(const std::string &path);
    void close();

    void seek(uint64_t offset);
    void seekHeader();
    void seekVlr();
    void seekPointData();
    void seekExtendedVlr();

    void readHeader();
    void writeHeader();

    void readPoint(Point &pt);
    void readPoint(Point &pt, const uint8_t *buffer, uint8_t fmt) const;
    void writePoint(const Point &pt);
    void writePoint(uint8_t *buffer, const Point &pt) const;

    void transform(double &x, double &y, double &z, const Point &pt) const;
    void transform(double &x,
                   double &y,
                   double &z,
                   const uint8_t *buffer) const;
    void transformInvert(double &x, double &y, double &z) const;

    File &file() { return file_; }

protected:
    File file_;

    void readHeader(Header &hdr);
    void writeHeader(const Header &hdr);
    void readPoint(uint8_t *buffer);
};

std::ostream &operator<<(std::ostream &os, const FileLas::Header &obj);
std::ostream &operator<<(std::ostream &os, const FileLas::Point &obj);

#endif /* FILE_LAS_HPP */
