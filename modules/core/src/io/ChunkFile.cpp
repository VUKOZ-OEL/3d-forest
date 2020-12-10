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
    @file ChunkFile.cpp
*/

#include <ChunkFile.hpp>
#include <Endian.hpp>
#include <Error.hpp>
#include <cassert>
#include <sstream>

const size_t ChunkFile::CHUNK_HEADER_SIZE = 16;

ChunkFile::ChunkFile()
{
    // empty
}

ChunkFile::~ChunkFile()
{
    // empty
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

void ChunkFile::read(ChunkFile::Chunk &c)
{
    uint8_t buffer[CHUNK_HEADER_SIZE];

    file_.read(buffer, CHUNK_HEADER_SIZE);

    c.type = ltoh32(&buffer[0]);
    c.major_version = buffer[4];
    c.minor_version = buffer[5];
    c.header_lenght = ltoh16(&buffer[6]);
    c.total_length = ltoh64(&buffer[8]);
}

void ChunkFile::write(const ChunkFile::Chunk &c)
{
    uint8_t buffer[CHUNK_HEADER_SIZE];

    htol32(&buffer[0], c.type);
    buffer[4] = c.major_version;
    buffer[5] = c.minor_version;
    htol16(&buffer[6], c.header_lenght);
    htol64(&buffer[8], c.total_length);

    file_.write(buffer, CHUNK_HEADER_SIZE);
}

Json &ChunkFile::Chunk::serialize(Json &out) const
{
    std::string str;

    str.resize(4);
    str[0] = static_cast<char>((type >> 24) & 0xFFU);
    str[1] = static_cast<char>((type >> 16) & 0xFFU);
    str[2] = static_cast<char>((type >> 8) & 0xFFU);
    str[3] = static_cast<char>(type & 0xFFU);

    out["type"] = str;
    out["major_version"] = major_version;
    out["minor_version"] = minor_version;
    out["header_lenght"] = header_lenght;
    out["total_length"] = total_length;

    return out;
}
