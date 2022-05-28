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

/** @file ChunkFile.hpp */

#ifndef CHUNK_FILE_HPP
#define CHUNK_FILE_HPP

#include <File.hpp>
#include <Json.hpp>

/** Chunk File.
    Simplified PNG/IFF like binary file format. It contains series of chunks.
    The first chunk type provides file signature.
    Each chunk contains both metadata (header) and data.
    Chunk bytes are { sizeof(Chunk), headerLength, dataLength }.
    The maximum length of chunk data is 64-bit value (16 exabytes).
    All multi-byte integers are in little-endian format.
    All integers should be aligned to file offsets by their size.
*/
class ChunkFile
{
public:
    static const size_t CHUNK_HEADER_SIZE;

    /** Chunk Header. */
    struct Chunk
    {
        uint32_t type;
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint16_t headerLength;
        uint64_t dataLength;

        Json &write(Json &out) const;
    };

    ChunkFile();
    ~ChunkFile();

    void open(const std::string &path, const std::string &mode);
    void close();

    void seek(uint64_t offset);
    void skip(uint64_t nbyte);

    void read(Chunk &chunk);
    void read(uint8_t *buffer, uint64_t nbyte);

    void write(const Chunk &chunk);
    void write(const uint8_t *buffer, uint64_t nbyte);

    void validate(const Chunk &chunk,
                  uint32_t type,
                  uint8_t majorVersion,
                  uint8_t minorVersion) const;

    bool eof() const;
    uint64_t size() const;
    uint64_t offset() const;
    const std::string &path() const;

protected:
    File file_;

    std::string status() const;
};

#endif /* CHUNK_FILE_HPP */
