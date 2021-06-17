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

/** @file File.cpp */

#include <Error.hpp>
#include <File.hpp>
#include <Time.hpp>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#ifndef O_BINARY
#define O_BINARY 0
#define O_TEXT 0
#endif

const int File::INVALID_DESCRIPTOR = -1;

File::File() : fd_(INVALID_DESCRIPTOR), size_(0), offset_(0), path_()
{
}

File::~File()
{
    if (fd_ != INVALID_DESCRIPTOR)
    {
        (void)::close(fd_);
    }
}

bool File::eof() const
{
    return offset_ == size_;
}

uint64_t File::size() const
{
    return size_;
}

uint64_t File::offset() const
{
    return offset_;
}

const std::string &File::path() const
{
    return path_;
}

void File::create()
{
    // Close
    if (fd_ != INVALID_DESCRIPTOR)
    {
        (void)::close(fd_);
    }

    // Temporary file with a unique auto-generated fileName, "wb+"
    std::FILE *tmpf = std::tmpfile();
    if (!tmpf)
    {
        THROW("Can't create temporary file");
    }

    fd_ = fileno(tmpf);
    size_ = 0;
    offset_ = 0;
    path_ = "temporary";
}

void File::create(const std::string &path)
{
    if (path == "")
    {
        create();
    }
    else
    {
        open(path, "w+");
    }
}

void File::open(const std::string &path)
{
    if (File::exists(path))
    {
        open(path, "r+");
    }
    else
    {
        open(path, "w+");
    }
}

void File::open(const std::string &path, const std::string &mode)
{
    int ret;
    int oflag;
    mode_t omode;
    struct stat st;

    // Close
    if (fd_ != INVALID_DESCRIPTOR)
    {
        (void)::close(fd_);
    }

    // Open
    oflag = 0;

    if (mode.find("r") != std::string::npos)
    {
        if (mode.find("+") != std::string::npos)
        {
            oflag |= O_RDWR;
        }
        else
        {
            oflag |= O_RDONLY;
        }

        oflag |= O_BINARY;

        fd_ = ::open(path.c_str(), oflag);
    }
    else if (mode.find("w") != std::string::npos ||
             mode.find("a") != std::string::npos)
    {
        if (mode.find("+") != std::string::npos)
        {
            oflag |= O_RDWR;
        }
        else
        {
            oflag |= O_WRONLY;
        }

        oflag |= O_BINARY;
        oflag |= O_CREAT;

        if (mode.find("a") != std::string::npos)
        {
            oflag |= O_APPEND;
        }
        else
        {
            oflag |= O_TRUNC;
        }

        omode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

        fd_ = ::open(path.c_str(), oflag, omode);
    }

    if (fd_ == INVALID_DESCRIPTOR)
    {
        THROW_ERRNO("Can't open file '" + path + "'");
    }

    ret = ::fstat(fd_, &st);
    if (ret != 0)
    {
        THROW_ERRNO("Can't stat file '" + path + "'");
    }

    size_ = static_cast<uint64_t>(st.st_size);
    offset_ = 0;
    path_ = path;
}

void File::close()
{
    int ret;

    if (fd_ != INVALID_DESCRIPTOR)
    {
        ret = ::close(fd_);
        if (ret != 0)
        {
            THROW_ERRNO("Can't close file '" + path_ + "'");
        }
        fd_ = INVALID_DESCRIPTOR;
    }

    size_ = 0;
    offset_ = 0;
    path_ = "";
}

int File::seek(int fd, uint64_t offset)
{
    off_t ret;

    if (offset > static_cast<uint64_t>(std::numeric_limits<off_t>::max()))
    {
        errno = ERANGE;
        return -1;
    }

    ret = ::lseek(fd, static_cast<off_t>(offset), SEEK_SET);
    if (ret == -1)
    {
        return -1;
    }

    return 0;
}

void File::skip(uint64_t nbyte)
{
    seek(offset_ + nbyte);
}

void File::seek(uint64_t offset)
{
    int ret;

    if (offset_ == offset)
    {
        return;
    }

    ret = seek(fd_, offset);
    if (ret == -1)
    {
        THROW_ERRNO("Can't seek file '" + path_ + "'");
    }

    offset_ = offset;
}

std::string File::read(const std::string &path)
{
    File f;
    f.open(path, "r");

    std::string ret;
    ret.resize(f.size());

    f.read(reinterpret_cast<uint8_t *>(&ret[0]), ret.size());
    f.close();

    return ret;
}

void File::read(uint8_t *buffer,
                const std::string &path,
                uint64_t nbyte,
                uint64_t offset)
{
    int ret;
    int fd;

    if (nbyte == 0)
    {
        return;
    }

    fd = ::open(path.c_str(), O_RDONLY | O_BINARY);
    if (fd == INVALID_DESCRIPTOR)
    {
        THROW_ERRNO("Can't open file '" + path + "'");
    }

    if (offset != 0)
    {
        ret = seek(fd, offset);
        if (ret == -1)
        {
            THROW_ERRNO("Can't seek file '" + path + "'");
        }
    }

    ret = read(fd, buffer, nbyte);
    if (ret == -1)
    {
        THROW_ERRNO("Can't read file '" + path + "'");
    }

    ret = ::close(fd);
    if (ret != 0)
    {
        THROW_ERRNO("Can't close file '" + path + "'");
    }
}

void File::read(uint8_t *buffer, uint64_t nbyte)
{
    int ret;

    if (nbyte == 0)
    {
        return;
    }

    ret = read(fd_, buffer, nbyte);
    if (ret == -1)
    {
        THROW_ERRNO("Can't read file '" + path_ + "'");
    }

    offset_ += nbyte;
}

int File::read(int fd, uint8_t *buffer, uint64_t nbyte)
{
    uint64_t total;
    uint64_t nread;
    ssize_t ret;

    assert(buffer);

    total = 0;
    while (nbyte > 0)
    {
        nread = nbyte;
        if (nread > UINT_MAX)
        {
            nread = UINT_MAX;
        }
        ret = ::read(fd, buffer + total, static_cast<unsigned int>(nread));
        if (ret == 0)
        {
            nbyte = 0;
        }
        else if (ret == -1)
        {
            if (errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            total += static_cast<uint64_t>(ret);
            nbyte -= static_cast<uint64_t>(ret);
        }
    }
    return 0;
}

void File::write(const std::string &path, const std::string &data)
{
    File f;
    f.open(path, "w");
    f.write(reinterpret_cast<const uint8_t *>(&data[0]), data.size());
    f.close();
}

void File::write(const uint8_t *buffer,
                 const std::string &path,
                 uint64_t nbyte,
                 uint64_t offset)
{
    int ret;
    int fd;

    if (nbyte == 0)
    {
        return;
    }

    fd = ::open(path.c_str(), O_WRONLY | O_BINARY);
    if (fd == INVALID_DESCRIPTOR)
    {
        THROW_ERRNO("Can't open file '" + path + "'");
    }

    if (offset != 0)
    {
        ret = seek(fd, offset);
        if (ret == -1)
        {
            THROW_ERRNO("Can't seek file '" + path + "'");
        }
    }

    ret = write(fd, buffer, nbyte);
    if (ret == -1)
    {
        THROW_ERRNO("Can't write file '" + path + "'");
    }

    ret = ::close(fd);
    if (ret != 0)
    {
        THROW_ERRNO("Can't close file '" + path + "'");
    }
}

void File::write(File &input, uint64_t nbyte)
{
    const size_t buffer_size = 1024 * 1024;
    std::vector<uint8_t> buffer;
    size_t n;

    buffer.resize(buffer_size);

    while (nbyte > 0)
    {
        n = nbyte;
        if (n > buffer_size)
        {
            n = buffer_size;
        }

        input.read(buffer.data(), n);
        write(buffer.data(), n);

        nbyte -= n;
    }
}

void File::write(const uint8_t *buffer, uint64_t nbyte)
{
    ssize_t ret;

    if (nbyte == 0)
    {
        return;
    }

    ret = write(fd_, buffer, nbyte);
    if (ret == -1)
    {
        THROW_ERRNO("Can't write file '" + path_ + "'");
    }

    offset_ += nbyte;
    if (offset_ > size_)
    {
        size_ = offset_;
    }
}

int File::write(int fd, const uint8_t *buffer, uint64_t nbyte)
{
    uint64_t total;
    uint64_t nwrite;
    ssize_t ret;

    assert(buffer);

    total = 0;
    while (nbyte > 0)
    {
        nwrite = nbyte;
        if (nwrite > UINT_MAX)
        {
            nwrite = UINT_MAX;
        }
        ret = ::write(fd, buffer + total, static_cast<unsigned int>(nwrite));
        if (ret == -1)
        {
            if (errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            total += static_cast<uint64_t>(ret);
            nbyte -= static_cast<uint64_t>(ret);
        }
    }
    return 0;
}

std::string File::currentPath()
{
    return std::filesystem::current_path().string();
}

bool File::exists(const std::string &path)
{
    int ret;
    struct stat st;

    ret = ::stat(path.c_str(), &st);
    (void)st;

    return ret == 0;
}

bool File::isAbsolute(const std::string &path)
{
    std::filesystem::path fsPath(path);
    return fsPath.is_absolute();
}

std::string File::fileName(const std::string &path)
{
    std::filesystem::path fsPath(path);
    return fsPath.filename().string();
}

std::string File::fileExtension(const std::string &path)
{
    std::filesystem::path fsPath(path);
    return fsPath.extension().string();
}

std::string File::replaceFileName(const std::string &path,
                                  const std::string &newFileName)
{
    std::filesystem::path fsPath(path);
    fsPath.replace_filename(newFileName);
    return fsPath.string();
}

std::string File::replaceExtension(const std::string &path,
                                   const std::string &newExtension)
{
    std::filesystem::path fsPath(path);
    fsPath.replace_extension(newExtension);
    return fsPath.string();
}

std::string File::tmpname(const std::string &path)
{
    uint64_t t = getRealTime64();
    char buffer[32];
    (void)snprintf(buffer, sizeof(buffer), "%016llX", t);
    return path + "." + std::string(buffer);
}

std::string File::tmpname(const std::string &outputPath,
                          const std::string &inputPath)
{
    if (inputPath == outputPath)
    {
        return File::tmpname(outputPath);
    }

    return outputPath;
}

void File::sort(const std::string &path,
                size_t element_size,
                int (*comp)(const void *, const void *))
{
    File src;

    src.open(path, "r");

    size_t bucket_size = src.size();
    size_t nelements = bucket_size / element_size;
    std::vector<uint8_t> bucket;
    bucket.resize(bucket_size);
    src.read(bucket.data(), bucket_size);
    src.close();

    std::qsort(bucket.data(), nelements, element_size, comp);

    src.open(path, "w");
    src.write(bucket.data(), bucket_size);
    src.close();
}

void File::move(const std::string &outputPath, const std::string &inputPath)
{
    if (inputPath == outputPath)
    {
        return;
    }

    if (!std::filesystem::exists(inputPath))
    {
        THROW("Cannot move: File '" + inputPath + "' doesn't exist");
    }
    if (std::filesystem::exists(outputPath))
    {
        (void)std::filesystem::remove(outputPath);
    }
    std::filesystem::rename(inputPath, outputPath);
}

void File::remove(const std::string &path)
{
    if (std::filesystem::exists(path))
    {
        (void)std::filesystem::remove(path);
    }
}
