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
    @file Database.hpp
*/

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <Aabb.hpp>
#include <DatabaseCell.hpp>
#include <LasFile.hpp>
#include <string>

/** Database. */
class Database
{
public:
    Aabbd aabb;

    Database();
    ~Database();

    void open(const std::string &path);
    void close();

    // size_t map(uint64_t index);

    size_t getCellSize() const { return cells_.size(); }
    const DatabaseCell &getCell(size_t i) const { return *cells_[i]; }

protected:
    // LasFile las_;
    std::vector<std::shared_ptr<DatabaseCell>> cells_;
};

#endif /* DATABASE_HPP */
