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

/** @file VectorFile.hpp */

#ifndef VECTOR_FILE_HPP
#define VECTOR_FILE_HPP

// Include std.
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Include 3D Forest.
#include <File.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Vector File. */
template <class T, class F> class VectorFile
{
public:
    static const size_t npos{SIZE_MAX};

    // Construct/Copy/Destroy.
    VectorFile();
    ~VectorFile();

    // Storage.
    void create(const std::string &path,
                size_t pageSize = 1000,
                size_t cacheSize = 1000);
    void open(const std::string &path,
              size_t pageSize = 1000,
              size_t cacheSize = 1000);
    void close();
    void flush();

    // Capacity.
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    // Element access.
    T &operator[](size_t pos) { return at(pos); }
    const T &operator[](size_t pos) const { return at(pos); }
    T &at(size_t pos, bool modify = true);
    const T &at(size_t pos) const;

    // Modifiers.
    void push_back(const T &e);
    void push_back(T &&e);
    void clear();

    // Other.
    std::string dumpToString() const;

private:
    /** Vector File Page. */
    class Page
    {
    public:
        std::vector<T> elements_;
        uint64_t timestamp_;
        size_t id_;
        bool dirtyFlag_;
    };

    size_t size_;
    size_t pageSize_;
    size_t cacheSize_;
    mutable uint64_t timestamp_;

    mutable File file_;
    mutable std::vector<std::shared_ptr<Page>> pages_;
    mutable std::vector<size_t> index_;
    mutable std::vector<uint8_t> buffer_;

    void init(size_t pageSize, size_t cacheSize);
    void seek(size_t pos) const;
    void readPage(size_t id) const;
    void readPage(size_t id, Page &page) const;
    void writePage(Page &page) const;
    void createIndex();
};

template <class T, class F>
inline VectorFile<T, F>::VectorFile()
    : size_(0),
      pageSize_(1000),
      cacheSize_(1000),
      timestamp_(0)
{
}

template <class T, class F> inline VectorFile<T, F>::~VectorFile()
{
    file_.close();
}

template <class T, class F>
inline void VectorFile<T, F>::create(const std::string &path,
                                     size_t pageSize,
                                     size_t cacheSize)
{
    close();
    file_.create(path);
    init(pageSize, cacheSize);
}

template <class T, class F>
inline void VectorFile<T, F>::open(const std::string &path,
                                   size_t pageSize,
                                   size_t cacheSize)
{
    close();
    file_.open(path);
    init(pageSize, cacheSize);
    createIndex();
}

template <class T, class F> inline void VectorFile<T, F>::close()
{
    flush();
    file_.close();
    clear();
}

template <class T, class F> inline void VectorFile<T, F>::seek(size_t pos) const
{
    file_.seek(static_cast<uint64_t>(pos) * sizeof(T));
}

template <class T, class F> inline void VectorFile<T, F>::flush()
{
    for (size_t i = 0; i < pages_.size(); i++)
    {
        if (pages_[i]->dirtyFlag_)
        {
            writePage(*pages_[i]);
        }
    }
}

template <class T, class F>
inline void VectorFile<T, F>::init(size_t pageSize, size_t cacheSize)
{
    size_ = static_cast<size_t>(file_.size() / sizeof(T));
    timestamp_ = 0;
    pageSize_ = pageSize;
    cacheSize_ = cacheSize;
    pages_.reserve(cacheSize_);
    index_.reserve(cacheSize_);
    buffer_.resize(pageSize_ * sizeof(T));
}

template <class T, class F> inline void VectorFile<T, F>::clear()
{
    size_ = 0;
    timestamp_ = 0;
    pages_.resize(0);
    index_.resize(0);
    buffer_.resize(0);
}

template <class T, class F> inline void VectorFile<T, F>::push_back(const T &e)
{
    size_t p = size_ / pageSize_;
    if (p == index_.size())
    {
        index_.resize(p + 1);
        index_[p] = npos;
    }
    if (index_[p] == npos)
    {
        readPage(p);
    }
    size_t c = index_[p];
    timestamp_++;
    size_++;
    pages_[c]->timestamp_ = timestamp_;
    pages_[c]->dirtyFlag_ = true;
    pages_[c]->elements_.push_back(e);
}

template <class T, class F> inline void VectorFile<T, F>::push_back(T &&e)
{
    size_t p = size_ / pageSize_;
    if (p == index_.size())
    {
        index_.resize(p + 1);
        index_[p] = npos;
    }
    if (index_[p] == npos)
    {
        readPage(p);
    }
    size_t c = index_[p];
    timestamp_++;
    size_++;
    pages_[c]->timestamp_ = timestamp_;
    pages_[c]->dirtyFlag_ = true;
    pages_[c]->elements_.push_back(std::move(e));
}

template <class T, class F> const T &VectorFile<T, F>::at(size_t pos) const
{
    size_t p = pos / pageSize_;
    size_t r = pos % pageSize_;
    if (index_[p] == npos)
    {
        readPage(p);
    }
    size_t c = index_[p];
    timestamp_++;
    pages_[c]->timestamp_ = timestamp_;
    return pages_[c]->elements_[r];
}

template <class T, class F> T &VectorFile<T, F>::at(size_t pos, bool modify)
{
    size_t p = pos / pageSize_;
    size_t r = pos % pageSize_;
    if (index_[p] == npos)
    {
        readPage(p);
    }
    size_t c = index_[p];
    timestamp_++;
    pages_[c]->timestamp_ = timestamp_;
    if (modify)
    {
        pages_[c]->dirtyFlag_ = true;
    }
    return pages_[c]->elements_[r];
}

template <class T, class F>
inline void VectorFile<T, F>::readPage(size_t id) const
{
    size_t idx = pages_.size();

    if (idx < cacheSize_)
    {
        pages_.resize(idx + 1);
        pages_[idx] = std::make_shared<Page>();
        pages_[idx]->elements_.reserve(pageSize_);
    }
    else
    {
        idx = 0;
        for (size_t i = 1; i < pages_.size(); i++)
        {
            if (pages_[i]->timestamp_ < pages_[idx]->timestamp_)
            {
                idx = i;
            }
        }

        if (pages_[idx]->dirtyFlag_)
        {
            writePage(*pages_[idx]);
        }

        index_[pages_[idx]->id_] = npos;
    }

    readPage(id, *pages_[idx]);
    index_[id] = idx;
}

template <class T, class F>
inline void VectorFile<T, F>::readPage(size_t id, Page &page) const
{
    page.timestamp_ = timestamp_;
    page.id_ = id;
    page.dirtyFlag_ = false;

    size_t pos = page.id_ * pageSize_;
    size_t n = size() - pos;
    if (n > pageSize_)
    {
        n = pageSize_;
    }
    page.elements_.resize(n);

    uint8_t *buffer = buffer_.data();
    seek(pos);
    file_.read(buffer, n * sizeof(T));
    for (size_t i = 0; i < n; i++)
    {
        F::read(buffer, page.elements_[i]);
        buffer += sizeof(T);
    }
}

template <class T, class F>
inline void VectorFile<T, F>::writePage(Page &page) const
{
    size_t pos = page.id_ * pageSize_;
    size_t n = page.elements_.size();
    uint8_t *buffer = buffer_.data();
    uint8_t *ptr = buffer;
    for (size_t i = 0; i < n; i++)
    {
        F::write(page.elements_[i], ptr);
        ptr += sizeof(T);
    }
    seek(pos);
    file_.write(buffer, n * sizeof(T));
    page.dirtyFlag_ = false;
}

template <class T, class F> inline void VectorFile<T, F>::createIndex()
{
    size_t n = static_cast<size_t>(size() / pageSize_);
    if (size() % pageSize_ > 0)
    {
        n++;
    }

    index_.resize(n);
    for (size_t i = 0; i < n; i++)
    {
        index_[i] = npos;
    }
}

template <class T, class F>
inline std::string VectorFile<T, F>::dumpToString() const
{
    std::stringstream stream;

    stream << "size <" << size_ << "> page size <" << pageSize_
           << "> cache size <" << cacheSize_ << "> used cache size <"
           << pages_.size() << "> index size <" << index_.size() << "> buffer <"
           << buffer_.size() << "> file size <" << file_.size()
           << "> timestamp <" << timestamp_ << "> ";

    stream << "index <";
    for (size_t i = 0; i < index_.size(); i++)
    {
        if (i > 0)
        {
            stream << ", ";
        }
        stream << index_[i];
    }

    stream << "> cache <";
    for (size_t i = 0; i < pages_.size(); i++)
    {
        if (i > 0)
        {
            stream << ", ";
        }
        auto page = pages_[i];
        stream << "<id <" << page->id_ << "> dirty <" << page->dirtyFlag_
               << "> timestamp <" << page->timestamp_ << "> size <"
               << page->elements_.size() << "> data <";
        for (size_t j = 0; j < page->elements_.size(); j++)
        {
            if (j > 0)
            {
                stream << ", ";
            }
            stream << page->elements_[j];
        }
        stream << ">";
    }
    stream << ">";

    return stream.str();
}

#include <WarningsEnable.hpp>

#endif /* VECTOR_FILE_HPP */
