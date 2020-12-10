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
    @file DatabaseCell.hpp
*/

#ifndef DATABASE_CELL_HPP
#define DATABASE_CELL_HPP

#include <cstdint>
#include <vector>

/** Database Cell. */
class DatabaseCell
{
public:
    struct Laser
    {
        uint16_t intensity;
        int16_t scanAngle;
        uint8_t returnNumber;
        uint8_t numberOfReturns;
        uint8_t classification;
        uint8_t userData;
    };

    std::vector<double> xyz;
    std::vector<float> rgb;
    std::vector<Laser> laser;
    std::vector<double> gps;

    uint64_t fileFrom;
    uint64_t fileSize;
    uint64_t id;

    DatabaseCell();
    ~DatabaseCell();
};

#endif /* DATABASE_CELL_HPP */
