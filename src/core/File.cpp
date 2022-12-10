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

#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <limits>
#include <sys/types.h>

#if defined(_MSC_VER)
    #define _CRT_INTERNAL_NONSTDC_NAMES 1
    #include <sys/stat.h>

    #if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
        #define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
    #endif

    #if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
        #define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
    #endif

    #define S_IRUSR 0400
    #define S_IRGRP (S_IRUSR >> 3)
    #define S_IROTH (S_IRGRP >> 3)
    #define S_IWUSR 0200
    #include <io.h>
    #define ssize_t int64_t
typedef unsigned short mode_t;
    #include <limits.h>
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif /* _MSC_VER */

#include <vector>

#include <Error.hpp>
#include <File.hpp>
#include <Time.hpp>

#ifndef O_BINARY
    #define O_BINARY 0
    #define O_TEXT 0
#endif

#if !defined(EXPORT_CORE_IMPORT)
const int File::INVALID_DESCRIPTOR = -1;
#endif

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

#if defined(_MSC_VER)
    struct _stat64 st;
    ret = ::_fstat64(fd_, &st);
#else
    struct stat st;
    ret = ::fstat(fd_, &st);
#endif

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
#if defined(_MSC_VER)
    if (offset > static_cast<uint64_t>((std::numeric_limits<__int64>::max)()))
    {
        errno = ERANGE;
        return -1;
    }

    __int64 ret;
    ret = ::_lseeki64(fd, static_cast<__int64>(offset), SEEK_SET);
    if (ret == -1LL)
    {
        return -1;
    }
#else
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
#endif

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
    const size_t buffer_size = 1048576U;
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

void File::write(const std::string &str)
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(str.c_str());
    write(ptr, str.size());
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

std::string File::join(const std::string &path1, const std::string &path2)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    const char *separator = "\\";
    const char *separator2 = "\\\\";
#else
    const char *separator = "/";
    const char *separator2 = "//";
#endif

    std::string result;

    if (path1.size() > 0)
    {
        if (path2.size() > 0)
        {
            result = path1 + separator + path2;
        }
        else
        {
            result = path1;
        }
    }
    else
    {
        result = path2;
    }

    size_t pos = 0;
    while (true)
    {
        pos = result.find(separator2, pos);
        if (pos == std::string::npos)
        {
            break;
        }

        result.replace(pos, 2, separator);
    }

    return result;
}

std::string File::currentPath()
{
    char buffer[8192];

#if defined(_MSC_VER)
    DWORD len = static_cast<DWORD>(sizeof(buffer));
    len = GetCurrentDirectoryA(len, buffer);
    if (len == 0)
    {
        THROW_LAST_ERROR("Cannot get current working directory");
    }
#else
    char *pathname = ::getcwd(buffer, sizeof(buffer));
    if (!pathname)
    {
        THROW_ERRNO("Cannot get current working directory");
    }
#endif

    std::string path(buffer);
    if (path.size() > 2 && path[1] == ':')
    {
        return path + "\\";
    }
    else
    {
        return path + "/";
    }
}

bool File::exists(const std::string &path)
{
    int ret;

#if defined(_MSC_VER)
    struct _stat64 st;
    ret = ::_stat64(path.c_str(), &st);
#else
    struct stat st;
    ret = ::stat(path.c_str(), &st);
#endif

    (void)st;

    return ret == 0;
}

bool File::isAbsolute(const std::string &path)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    if (path.size() > 2)
    {
        return path[1] == ':'; // path="C:\Users\user" or "C:\"
    }
#else
    if (path.size() > 0)
    {
        return path[0] == '/'; // path="/home/user" or "/"
    }
#endif

    return false;
}

std::string File::fileName(const std::string &path)
{
    size_t i = path.size();
    if (i > 0)
    {
        i--;

        // Find last '/'
        while (i > 0)
        {
            if (path[i] == '/' || path[i] == '\\')
            {
                return path.substr(i + 1);
            }

            i--;
        }

        return path.substr(i);
    }

    return "";
}

std::string File::fileExtension(const std::string &path)
{
    size_t i = path.size();
    if (i > 0)
    {
        i--;

        // Find last '.'
        while (i > 0)
        {
            if (path[i] == '/' || path[i] == '\\')
            {
                return "";
            }

            if (path[i] == '.')
            {
                return path.substr(i + 1);
            }

            i--;
        }
    }

    return "";
}

std::string File::replaceFileName(const std::string &path,
                                  const std::string &newFileName)
{
    size_t i = path.size();
    if (i > 0)
    {
        i--;

        // Find last '/'
        while (i > 0)
        {
            if (path[i] == '/' || path[i] == '\\')
            {
                return path.substr(0, i + 1) + newFileName;
            }

            i--;
        }
    }

    return newFileName;
}

std::string File::replaceExtension(const std::string &path,
                                   const std::string &newExtension)
{
    size_t i = path.size();
    if (i > 0)
    {
        i--;

        // Find last '.'
        while (i > 0)
        {
            if (path[i] == '/' || path[i] == '\\')
            {
                return path + newExtension;
            }

            if (path[i] == '.')
            {
                return path.substr(0, i) + newExtension;
            }

            i--;
        }
    }

    return path + newExtension;
}

std::string File::resolvePath(const std::string &path,
                              const std::string &basePath)
{
    std::string rval;

    rval = path;
    if (!File::isAbsolute(rval))
    {
        // Resolve path
        rval = File::replaceFileName(basePath, rval);
    }

    if (!File::exists(rval))
    {
        THROW("File '" + rval + "' doesn't exist");
    }

    return rval;
}

std::string File::tmpname(const std::string &path)
{
    unsigned long long t = getRealTime64();
    char buffer[32];
    (void)snprintf(buffer, sizeof(buffer), "%016llX", t);
    msleep(1);
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

    if (!File::exists(inputPath))
    {
        THROW("Cannot move: File '" + inputPath + "' doesn't exist");
    }

    File::remove(outputPath);

    int error = ::rename(inputPath.c_str(), outputPath.c_str());
    if (error != 0)
    {
        THROW_ERRNO("Cannot move file '" + inputPath + "'");
    }
}

void File::remove(const std::string &path)
{
    if (!File::exists(path))
    {
        // Nothing to remove
        return;
    }

    int error = ::unlink(path.c_str());
    if (error != 0)
    {
        THROW_ERRNO("Cannot remove file '" + path + "'");
    }
}
