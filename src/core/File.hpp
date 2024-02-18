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

/** @file File.hpp */

#ifndef FILE_HPP
#define FILE_HPP

// Include std.
#include <cstdint>
#include <string>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** File. */
class EXPORT_CORE File
{
public:
    File();
    ~File();
    File(const File &other);
    File &operator=(const File &other);

    void create(const std::string &path);
    void open(const std::string &path);
    void open(const std::string &path, const std::string &mode);
    void close();

    void seek(uint64_t offset);
    void skip(uint64_t nbyte);

    void read(uint8_t *buffer, uint64_t nbyte);
    void write(const uint8_t *buffer, uint64_t nbyte);
    void write(const std::string &str);
    void write(File &input, uint64_t nbyte);

    bool isOpen() const { return fd_ != INVALID_DESCRIPTOR; }
    bool eof() const;
    uint64_t size() const;
    uint64_t offset() const;
    const std::string &path() const;

    static std::string join(const std::string &path1, const std::string &path2);
    static std::string currentPath();
    static bool exists(const std::string &path);
    static bool isAbsolute(const std::string &path);
    static std::string fileName(const std::string &path);
    static std::string fileExtension(const std::string &path);
    static std::string replaceFileName(const std::string &path,
                                       const std::string &newFileName);
    static std::string replaceExtension(const std::string &path,
                                        const std::string &newExtension);
    static std::string resolvePath(const std::string &path,
                                   const std::string &basePath);

    static void read(uint8_t *buffer,
                     const std::string &path,
                     uint64_t nbyte,
                     uint64_t offset);

    static void write(const uint8_t *buffer,
                      const std::string &path,
                      uint64_t nbyte,
                      uint64_t offset);

    static std::string read(const std::string &path);
    static void write(const std::string &path, const std::string &data);

    static std::string tmpname(const std::string &path);
    static std::string tmpname(const std::string &outputPath,
                               const std::string &inputPath);

    static void sort(const std::string &path,
                     size_t element_size,
                     int (*comp)(const void *, const void *));

    static void move(const std::string &outputPath,
                     const std::string &inputPath);

    static void remove(const std::string &path);

private:
    int fd_;
    uint64_t size_;
    uint64_t offset_;
    std::string path_;

    static const int INVALID_DESCRIPTOR;

    void create();
    static int seek(int fd, uint64_t offset);
    static int read(int fd, uint8_t *buffer, uint64_t nbyte);
    static int write(int fd, const uint8_t *buffer, uint64_t nbyte);
};

std::ostream &operator<<(std::ostream &os, const File &obj);

#include <WarningsEnable.hpp>

#endif /* FILE_HPP */
