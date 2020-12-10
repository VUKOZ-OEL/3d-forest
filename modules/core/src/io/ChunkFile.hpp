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
    @file ChunkFile.hpp
*/

#ifndef CHUNK_FILE_HPP
#define CHUNK_FILE_HPP

#include <File.hpp>
#include <Json.hpp>

/** Chunk file. */
class ChunkFile
{
public:
    static const size_t CHUNK_HEADER_SIZE;

    /** Chunk. */
    struct Chunk
    {
        uint32_t type;
        uint8_t major_version;
        uint8_t minor_version;
        uint16_t header_lenght;
        uint64_t total_length;

        Json &serialize(Json &out) const;
    };

    ChunkFile();
    ~ChunkFile();

    void open(const std::string &path, const std::string &mode);
    void close();

    void seek(uint64_t offset);
    void skip(uint64_t nbyte);

    void read(Chunk &c);
    void read(uint8_t *buffer, uint64_t nbyte);

    void write(const Chunk &c);
    void write(const uint8_t *buffer, uint64_t nbyte);

    bool eof() const;
    uint64_t size() const;
    uint64_t offset() const;
    const std::string &path() const;

protected:
    File file_;

    std::string status() const;
};

#endif /* CHUNK_FILE_HPP */
