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

/** @file RecordFile.cpp */

// Include std.
#include <cstring>

// Include 3D Forest.
#include <Endian.hpp>
#include <Error.hpp>
#include <RecordFile.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "RecordFile"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define RECORD_FILE_SIGNATURE_0 0x52
#define RECORD_FILE_SIGNATURE_1 0x45
#define RECORD_FILE_SIGNATURE_2 0x43
#define RECORD_FILE_SIGNATURE_3 0x46

RecordFile::RecordFile()
{
    init();
}

void RecordFile::init()
{
    recordType_ = RecordFile::TYPE_CUSTOM;
    recordSize_ = 0;
    headerSize_ = 0;
    name_.clear();
}

RecordFile::~RecordFile()
{
}

void RecordFile::create(const std::string &path,
                        const std::string &name,
                        RecordFile::Type recordType,
                        size_t recordSize)
{
    init();
    file_.create(path);
    writeHeader(name, recordType, recordSize);
}

void RecordFile::open(const std::string &path)
{
    init();
    if (!File::exists(path))
    {
        THROW("Can't open file '" + path + "': file does not exist");
    }
    file_.open(path);
    readHeader();
}

void RecordFile::writeHeader(const std::string &name,
                             RecordFile::Type recordType,
                             size_t recordSize)
{
    // Set new record type.
    setupRecordType(recordType, recordSize);
    headerSize_ = sizeof(RecordFile::Header);
    name_ = name;

    // Fill header.
    uint8_t buffer[128];
    std::memset(&buffer[0], 0, headerSize_);
    buffer[0] = RECORD_FILE_SIGNATURE_0;
    buffer[1] = RECORD_FILE_SIGNATURE_1;
    buffer[2] = RECORD_FILE_SIGNATURE_2;
    buffer[3] = RECORD_FILE_SIGNATURE_3;
    buffer[4] = 1; // Major.
    buffer[5] = 0; // Minor.
    htol16(&buffer[6], static_cast<uint16_t>(headerSize_));
    htol32(&buffer[8], static_cast<uint32_t>(recordType_));
    htol32(&buffer[12], static_cast<uint32_t>(recordSize_));
    for (size_t i = 0; i < name_.length() && i < 31; i++)
    {
        buffer[16 + i] = static_cast<uint8_t>(name_[i]);
    }

    // Write header.
    file_.write(buffer, headerSize_);
}

void RecordFile::readHeader()
{
    if (file_.size() < 8)
    {
        THROW("Record file '" + file_.path() + "' has invalid size");
    }

    // Read the first part of header.
    uint8_t buffer[128];
    file_.read(buffer, 8);

    if ((buffer[0] != RECORD_FILE_SIGNATURE_0) ||
        (buffer[1] != RECORD_FILE_SIGNATURE_1) ||
        (buffer[2] != RECORD_FILE_SIGNATURE_2) ||
        (buffer[3] != RECORD_FILE_SIGNATURE_3))
    {
        THROW("Record file '" + file_.path() + "' has invalid signature");
    }

    if (buffer[4] != 1)
    {
        THROW("LAS '" + file_.path() + "' has incompatible major version");
    }

    headerSize_ = ltoh16(&buffer[6]);
    if (headerSize_ < 40 || headerSize_ > 100)
    {
        THROW("Record file '" + file_.path() + "' has invalid header size");
    }

    // Read the second part of header.
    file_.read(buffer, headerSize_ - 8);

    int recordType = static_cast<int>(ltoh32(&buffer[0]));
    size_t recordSize = ltoh32(&buffer[4]);
    setupRecordType(recordType, recordSize);

    char name[32];
    std::memcpy(name, buffer + 8, 32);
    name[31] = 0;
    name_ = name;
}

void RecordFile::setupRecordType(int recordType, size_t recordSize)
{
    switch (recordType)
    {
        case RecordFile::TYPE_CUSTOM:
            recordType_ = RecordFile::TYPE_CUSTOM;
            recordSize_ = recordSize;
            break;
        case RecordFile::TYPE_U32:
            recordType_ = RecordFile::TYPE_U32;
            recordSize_ = sizeof(uint32_t);
            break;
        case RecordFile::TYPE_U64:
            recordType_ = RecordFile::TYPE_U64;
            recordSize_ = sizeof(uint64_t);
            break;
        case RecordFile::TYPE_F64:
            recordType_ = RecordFile::TYPE_F64;
            recordSize_ = sizeof(double);
            break;
        default:
            THROW("Record file '" + file_.path() + "' has unknown type <" +
                  std::to_string(recordType) + ">.");
            break;
    }
}

void RecordFile::close()
{
    file_.close();
}

uint64_t RecordFile::size() const
{
    return (file_.size() - headerSize_) / recordSize_;
}

uint64_t RecordFile::index() const
{
    return (file_.offset() - headerSize_) / recordSize_;
}

void RecordFile::setIndex(uint64_t index)
{
    uint64_t max = size();
    LOG_DEBUG(<< "Seek record <" << index << "> from <" << size()
              << "> records in file <" << file_ << ">.");
    if (index > max)
    {
        fill(index - max);
    }

    file_.seek(headerSize_ + (index * recordSize_));
}

void RecordFile::fill(uint64_t n)
{
    LOG_DEBUG(<< "Fill <" << n << "> records in file <" << file_ << ">.");

    std::vector<uint8_t> buffer;
    buffer.resize(1000 * 1000);
    std::memset(buffer.data(), 0, buffer.size());

    uint64_t written = 0;
    uint64_t totalBytes = n * recordSize_;

    while (written < totalBytes)
    {
        uint64_t nbyte = totalBytes - written;
        if (nbyte > buffer.size())
        {
            nbyte = buffer.size();
        }
        file_.write(buffer.data(), nbyte);
        written += nbyte;
    }
}

void RecordFile::write(uint32_t data)
{
    uint8_t buffer[8];
    htol32(&buffer[0], data);
    file_.write(buffer, recordSize_);
}

void RecordFile::write(uint64_t data)
{
    uint8_t buffer[8];
    htol64(&buffer[0], data);
    file_.write(buffer, recordSize_);
}

void RecordFile::write(double data)
{
    uint8_t buffer[8];
    htold(&buffer[0], data);
    file_.write(buffer, recordSize_);
}

void RecordFile::read(uint32_t &data)
{
    uint8_t buffer[8];
    file_.read(buffer, recordSize_);
    data = ltoh32(&buffer[0]);
}

void RecordFile::read(uint64_t &data)
{
    uint8_t buffer[8];
    file_.read(buffer, recordSize_);
    data = ltoh64(&buffer[0]);
}

void RecordFile::read(double &data)
{
    uint8_t buffer[8];
    file_.read(buffer, recordSize_);
    data = ltohd(&buffer[0]);
}

void RecordFile::readBuffer(uint8_t *buffer, uint64_t nbyte)
{
    if (file_.size() - file_.offset() < nbyte)
    {
        LOG_DEBUG(<< "Clear <" << nbyte
                  << "> bytes in buffer instead of reading from file <" << file_
                  << ">.");
        std::memset(buffer, 0, nbyte);
    }
    else
    {
        LOG_DEBUG(<< "Read <" << nbyte << "> bytes from file <" << file_
                  << ">.");
        file_.read(buffer, nbyte);
    }
}

void RecordFile::writeBuffer(const uint8_t *buffer, uint64_t nbyte)
{
    LOG_DEBUG(<< "Write <" << nbyte << "> bytes to file <" << file_ << ">.");
    file_.write(buffer, nbyte);
}

void RecordFile::createBuffer(RecordFile::Buffer &buffer,
                              uint64_t n,
                              bool setZero) const
{
    LOG_DEBUG(<< "Create buffer with <" << n << "> records for file <" << file_
              << ">.");

    buffer.recordType = recordType_;
    buffer.recordSize = recordSize_;
    buffer.size = static_cast<size_t>(n);
    buffer.name = name_;

    size_t nbyte = buffer.size * recordSize_;
    if (buffer.data.size() < nbyte)
    {
        buffer.data.resize(nbyte);
    }

    if (setZero)
    {
        std::memset(buffer.data.data(), 0, nbyte);
    }
}

void RecordFile::readBuffer(RecordFile::Buffer &buffer, uint64_t n)
{
    LOG_DEBUG(<< "Read <" << n << "> records to buffer from file <" << file_
              << ">.");

    createBuffer(buffer, n);
    uint64_t nbyte = n * recordSize_;
    readBuffer(buffer.data.data(), nbyte);
}

void RecordFile::writeBuffer(const RecordFile::Buffer &buffer,
                             uint64_t n,
                             uint64_t from)
{
    LOG_DEBUG(<< "Write <" << n << "> records from buffer position <" << from
              << "> to file <" << file_ << ">.");

    size_t offset = static_cast<size_t>(from) * buffer.recordSize;
    uint64_t nbyte = n * buffer.recordSize;
    if (offset + nbyte <= buffer.data.size())
    {
        writeBuffer(buffer.data.data() + offset, nbyte);
    }
}

void RecordFile::range(uint32_t &min, uint32_t &max, uint64_t n, uint64_t from)
{
    // std::vector<uint32_t> data;
    uint32_t data;

    setIndex(from);

    for (uint64_t i = 0; i < n; i++)
    {
        read(data);

        if (data < min)
        {
            min = data;
        }

        if (data > max)
        {
            max = data;
        }
    }
}

template <class T>
static void recordFileBufferWrite(std::vector<uint8_t> &dst,
                                  const std::vector<T> &src,
                                  RecordFile::Type recordType,
                                  size_t n)
{
    if (dst.size() < n)
    {
        dst.resize(n);
    }

    switch (recordType)
    {
        case RecordFile::TYPE_U32:
            htol32(dst.data(), src.data(), n);
            break;
        case RecordFile::TYPE_U64:
            htol64(dst.data(), src.data(), n);
            break;
        case RecordFile::TYPE_F64:
            htold(dst.data(), src.data(), n);
            break;
        case RecordFile::TYPE_CUSTOM:
        default:
            THROW("Can not write record file type <" +
                  std::to_string(recordType) + ">.");
            break;
    }
}

void RecordFile::Buffer::write(const std::vector<size_t> &v)
{
    recordFileBufferWrite(data, v, recordType, size);
}

void RecordFile::Buffer::write(const std::vector<double> &v)
{
    recordFileBufferWrite(data, v, recordType, size);
}

template <class T>
static void recordFileBufferRead(std::vector<T> &dst,
                                 const std::vector<uint8_t> &src,
                                 RecordFile::Type recordType,
                                 size_t n)
{
    if (dst.size() < n)
    {
        dst.resize(n);
    }

    switch (recordType)
    {
        case RecordFile::TYPE_U32:
            ltoh32(dst.data(), src.data(), n);
            break;
        case RecordFile::TYPE_U64:
            ltoh64(dst.data(), src.data(), n);
            break;
        case RecordFile::TYPE_F64:
            ltohd(dst.data(), src.data(), n);
            break;
        case RecordFile::TYPE_CUSTOM:
        default:
            THROW("Can not read record file type <" +
                  std::to_string(recordType) + ">.");
            break;
    }
}

void RecordFile::Buffer::read(std::vector<size_t> &v) const
{
    recordFileBufferRead(v, data, recordType, size);
}

void RecordFile::Buffer::read(std::vector<double> &v) const
{
    recordFileBufferRead(v, data, recordType, size);
}

void RecordFile::Buffer::copy(const RecordFile::Buffer &src,
                              uint64_t n,
                              uint64_t to,
                              uint64_t from)
{
    size_t dstOffset = static_cast<size_t>(to) * recordSize;
    size_t srcOffset = static_cast<size_t>(from) * src.recordSize;
    size_t nbyte = static_cast<size_t>(n) * src.recordSize;

    std::memcpy(data.data() + dstOffset, src.data.data() + srcOffset, nbyte);
}
