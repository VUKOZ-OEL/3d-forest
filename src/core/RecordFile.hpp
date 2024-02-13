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

/** @file RecordFile.hpp */

#ifndef RECORD_FILE_HPP
#define RECORD_FILE_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <File.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Simple Data File Format Containing Homogenous Records. */
class EXPORT_CORE RecordFile
{
public:
    /** Record File Type. */
    enum Type
    {
        TYPE_CUSTOM,
        TYPE_U32,
        TYPE_U64,
        TYPE_F64
    };

    /** Record File Header. */
    struct EXPORT_CORE Header
    {
        char signature[4];
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint16_t headerSize;
        uint32_t recordType;
        uint32_t recordSize;
        char name[32];
    };

    /** Record File Buffer. */
    struct EXPORT_CORE Buffer
    {
        RecordFile::Type recordType;
        size_t recordSize;
        size_t size;
        std::string name;
        std::vector<uint8_t> data;

        void write(const std::vector<size_t> &v);
        void write(const std::vector<double> &v);

        void read(std::vector<size_t> &v) const;
        void read(std::vector<double> &v) const;

        void copy(const RecordFile::Buffer &src,
                  uint64_t n,
                  uint64_t to,
                  uint64_t from);
    };

    RecordFile();
    ~RecordFile();

    void create(const std::string &path,
                const std::string &name,
                RecordFile::Type recordType,
                size_t recordSize = 0);
    void open(const std::string &path);
    void close();
    bool isOpen() const { return file_.isOpen(); }

    RecordFile::Type recordType() const { return recordType_; }
    size_t recordSize() const { return recordSize_; }
    const std::string &name() const { return name_; }

    uint64_t size() const;
    uint64_t index() const;
    void setIndex(uint64_t index);

    /** Fill 'n' file records with zero's. */
    void fill(uint64_t n);

    void write(uint32_t data);
    void write(uint64_t data);
    void write(double data);

    void read(uint32_t &data);
    void read(uint64_t &data);
    void read(double &data);

    void readBuffer(uint8_t *buffer, uint64_t nbyte);
    void writeBuffer(const uint8_t *buffer, uint64_t nbyte);

    void createBuffer(RecordFile::Buffer &buffer,
                      uint64_t n,
                      bool setZero = false) const;
    void readBuffer(RecordFile::Buffer &buffer, uint64_t n);
    void writeBuffer(const RecordFile::Buffer &buffer,
                     uint64_t n,
                     uint64_t from = 0);

protected:
    RecordFile::Type recordType_;
    size_t recordSize_;
    uint64_t headerSize_;
    std::string name_;
    File file_;

    void init();
    void setupRecordType(int recordType, size_t recordSize);
    void writeHeader(const std::string &name,
                     RecordFile::Type recordType,
                     size_t recordSize);
    void readHeader();
};

#endif /* RECORD_FILE_HPP */
