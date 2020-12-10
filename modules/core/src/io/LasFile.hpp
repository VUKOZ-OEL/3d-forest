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
    @file LasFile.hpp
*/

#ifndef LAS_FILE_HPP
#define LAS_FILE_HPP

#include <File.hpp>
#include <Json.hpp>
#include <vector>

/** LAS (LASer) file format. */
class LasFile
{
public:
    /** LAS header. */
    struct Header
    {
        char file_signature[4];

        // offset 4, size 20
        uint16_t file_source_id;  // 1.1
        uint16_t global_encoding; // 1.2
        uint32_t project_id_1;    // optional
        uint16_t project_id_2;    // optional
        uint16_t project_id_3;    // optional
        uint8_t project_id_4[8];  // optional

        // offset 24, size 70
        uint8_t version_major;
        uint8_t version_minor;
        char system_identifier[32];
        char generating_software[32];
        uint16_t file_creation_day_of_year; // optional, 1.3 required
        uint16_t file_creation_year;        // optional 4 digits, 1.3 required

        // offset 94, size 37
        uint16_t header_size;
        uint32_t offset_to_point_data;
        uint32_t number_of_vlr;
        uint8_t point_data_record_format;
        uint16_t point_data_record_length; // bytes per point (extra data)
        uint32_t legacy_number_of_point_records;
        uint32_t legacy_number_of_points_by_return[5];

        // offset 131, size 96
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
        // end of v 1.0, 1.1, 1.2 (227 bytes)

        // offset 227, size 8
        uint64_t offset_to_wdpr;
        // end of 1.3 (235 bytes)

        // offset 235, size 140
        uint64_t offset_to_evlr;
        uint32_t number_of_evlr;
        uint64_t number_of_point_records;
        uint64_t number_of_points_by_return[15];
        // end of 1.4 (375 bytes)

        bool hasRgb() const;
        Json &serialize(Json &out) const;
    };

    /** LAS point. */
    struct Point
    {
        // format 0 to 10
        uint32_t x;
        uint32_t y;
        uint32_t z;
        uint16_t intensity; // optional

        // format 0 to 10
        uint8_t return_number;       // 0 to 15
        uint8_t number_of_returns;   // 0 to 15
        uint8_t scan_direction_flag; // 0 or 1
        uint8_t edge_of_flight_line; // 0 or 1

        // format 6 to 10
        uint8_t classification_flags; // 0 to 15
        uint8_t scanner_channel;      // 0 to 3

        // format 0 to 10
        int16_t angle;
        uint16_t source_id;     // v1.0 user_data
        uint8_t classification; // sometimes optional
        uint8_t user_data;      // optional, v1.0 file_marker
        uint8_t format;

        // format 4, 5, 9, 10
        uint8_t wave_index;
        uint32_t wave_size;

        // format 1, 3, 4, 5, 6, 7, 8, 9, 10
        double gps_time;

        // format 2, 3, 5, 7, 8, 10
        uint16_t red;
        uint16_t green;
        uint16_t blue;

        // format 8, 10
        uint16_t nir;

        // format 4, 5, 9, 10
        uint64_t wave_offset;
        float wave_return;
        float wave_x;
        float wave_y;
        float wave_z;

        Json &serialize(Json &out) const;
    };

    Header header;

    LasFile();
    ~LasFile();

    void open(const std::string &path);
    void close();

    void read(Point &pt);
    void transform(double &x, double &y, double &z, const Point &pt) const;

    void read(uint8_t *buffer);
    void transform(double &x,
                   double &y,
                   double &z,
                   const uint8_t *buffer) const;

protected:
    File file_;

    void read(Header &hdr);
    void read(Point &pt, const uint8_t *buffer, uint8_t fmt) const;
};

#endif /* LAS_FILE_HPP */
