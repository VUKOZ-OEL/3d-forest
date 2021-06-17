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

/** @file FileChunk.cpp */

#include <Endian.hpp>
#include <Error.hpp>
#include <FileChunk.hpp>
#include <cassert>
#include <sstream>

const size_t FileChunk::CHUNK_HEADER_SIZE = 16;

FileChunk::FileChunk()
{
}

FileChunk::~FileChunk()
{
}

bool FileChunk::eof() const
{
    return file_.eof();
}

uint64_t FileChunk::size() const
{
    return file_.size();
}

uint64_t FileChunk::offset() const
{
    return file_.offset();
}

const std::string &FileChunk::path() const
{
    return file_.path();
}

std::string FileChunk::status() const
{
    std::ostringstream output;

    output << "file '" << path() << "' offset " << offset();

    return output.str();
}

void FileChunk::open(const std::string &path, const std::string &mode)
{
    file_.open(path, mode);
}

void FileChunk::close()
{
    file_.close();
}

void FileChunk::seek(uint64_t offset)
{
    file_.seek(offset);
}

void FileChunk::skip(uint64_t nbyte)
{
    file_.skip(nbyte);
}

void FileChunk::read(uint8_t *buffer, uint64_t nbyte)
{
    file_.read(buffer, nbyte);
}

void FileChunk::write(const uint8_t *buffer, uint64_t nbyte)
{
    file_.write(buffer, nbyte);
}

void FileChunk::read(FileChunk::Chunk &chunk)
{
    uint8_t buffer[CHUNK_HEADER_SIZE];

    file_.read(buffer, CHUNK_HEADER_SIZE);

    chunk.type = ltoh32(&buffer[0]);
    chunk.majorVersion = buffer[4];
    chunk.minorVersion = buffer[5];
    chunk.headerLength = ltoh16(&buffer[6]);
    chunk.dataLength = ltoh64(&buffer[8]);
}

void FileChunk::validate(const Chunk &chunk,
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

void FileChunk::write(const FileChunk::Chunk &chunk)
{
    uint8_t buffer[CHUNK_HEADER_SIZE];

    htol32(&buffer[0], chunk.type);
    buffer[4] = chunk.majorVersion;
    buffer[5] = chunk.minorVersion;
    htol16(&buffer[6], chunk.headerLength);
    htol64(&buffer[8], chunk.dataLength);

    file_.write(buffer, CHUNK_HEADER_SIZE);
}

Json &FileChunk::Chunk::write(Json &out) const
{
    std::string str;

    str.resize(4);
    str[0] = static_cast<char>((type >> 24) & 0xFFU);
    str[1] = static_cast<char>((type >> 16) & 0xFFU);
    str[2] = static_cast<char>((type >> 8) & 0xFFU);
    str[3] = static_cast<char>(type & 0xFFU);

    out["type"] = str;
    out["majorVersion"] = majorVersion;
    out["minorVersion"] = minorVersion;
    out["headerLength"] = headerLength;
    out["dataLength"] = dataLength;

    return out;
}
