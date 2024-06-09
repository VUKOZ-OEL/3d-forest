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

/** @file ChunkFile.cpp */

// Include std.
#include <cassert>
#include <sstream>

// Include 3D Forest.
#include <ChunkFile.hpp>
#include <Endian.hpp>
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "ChunkFile"
#include <Log.hpp>

#define CHUNK_HEADER_SIZE 16U

ChunkFile::ChunkFile()
{
}

ChunkFile::~ChunkFile()
{
}

bool ChunkFile::eof() const
{
    return file_.eof();
}

uint64_t ChunkFile::size() const
{
    return file_.size();
}

uint64_t ChunkFile::offset() const
{
    return file_.offset();
}

const std::string &ChunkFile::path() const
{
    return file_.path();
}

std::string ChunkFile::status() const
{
    std::ostringstream output;

    output << "file '" << path() << "' offset " << offset();

    return output.str();
}

void ChunkFile::open(const std::string &path, const std::string &mode)
{
    file_.open(path, mode);
}

void ChunkFile::close()
{
    file_.close();
}

void ChunkFile::seek(uint64_t offset)
{
    file_.seek(offset);
}

void ChunkFile::skip(uint64_t nbyte)
{
    file_.skip(nbyte);
}

void ChunkFile::read(uint8_t *buffer, uint64_t nbyte)
{
    file_.read(buffer, nbyte);
}

void ChunkFile::write(const uint8_t *buffer, uint64_t nbyte)
{
    file_.write(buffer, nbyte);
}

void ChunkFile::read(ChunkFile::Chunk &chunk)
{
    uint8_t buffer[CHUNK_HEADER_SIZE];

    file_.read(buffer, CHUNK_HEADER_SIZE);

    chunk.type = ltoh32(&buffer[0]);
    chunk.majorVersion = buffer[4];
    chunk.minorVersion = buffer[5];
    chunk.headerLength = ltoh16(&buffer[6]);
    chunk.dataLength = ltoh64(&buffer[8]);
}

void ChunkFile::validate(const Chunk &chunk,
                         uint32_t type,
                         uint8_t majorVersion,
                         uint8_t minorVersion) const
{
    if ((chunk.type != type) || (chunk.majorVersion != majorVersion) ||
        (chunk.minorVersion > minorVersion))
    {
        THROW("Unexpected chunk in " + status());
    }
}

void ChunkFile::write(const ChunkFile::Chunk &chunk)
{
    uint8_t buffer[CHUNK_HEADER_SIZE];

    htol32(&buffer[0], chunk.type);
    buffer[4] = chunk.majorVersion;
    buffer[5] = chunk.minorVersion;
    htol16(&buffer[6], chunk.headerLength);
    htol64(&buffer[8], chunk.dataLength);

    file_.write(buffer, CHUNK_HEADER_SIZE);
}
