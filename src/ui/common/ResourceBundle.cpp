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

/** @file ResourceBundle.cpp */

// Include std.
#include <map>
#include <mutex>
#include <stdexcept>
#include <utility>

// Include 3D Forest.
#include <ResourceBundle.hpp>

// Include local.
#define LOG_MODULE_NAME "ResourceBundle"
#include <Log.hpp>

namespace
{
struct Registry
{
    std::mutex mutex;
    std::map<std::string, ResourceData> files;
};

Registry &registry()
{
    static Registry value;
    return value;
}
} // namespace

ResourceData ResourceRegistry::get(const std::string &path)
{
    auto &state = registry();
    const std::lock_guard<std::mutex> lock(state.mutex);
    const auto it = state.files.find(path);
    return it == state.files.end() ? ResourceData{} : it->second;
}

ResourceBundle::ResourceBundle(std::initializer_list<ResourceEntry> entries)
{
    // Stage everything first: invalid entries, duplicate paths, or allocation
    // failure must not leave a partially registered bundle.
    std::map<std::string, ResourceData> staged;
    paths_.reserve(entries.size());

    for (const auto &entry : entries)
    {
        if (!entry.path || std::string(entry.path).compare(0, 2, ":/") != 0)
        {
            throw std::invalid_argument("Resource path must start with :/");
        }
        if (entry.size && !entry.data)
        {
            throw std::invalid_argument("Resource data is null");
        }

        auto bytes = std::make_shared<ResourceBytes>();
        if (entry.size)
        {
            bytes->assign(entry.data, entry.data + entry.size);
        }

        if (!staged.emplace(entry.path, std::move(bytes)).second)
        {
            throw std::runtime_error(std::string("Duplicate resource: ") +
                                     entry.path);
        }
        paths_.emplace_back(entry.path);
    }

    auto &state = registry();
    const std::lock_guard<std::mutex> lock(state.mutex);
    for (const auto &entry : staged)
    {
        if (state.files.count(entry.first))
        {
            throw std::runtime_error("Resource already registered: " +
                                     entry.first);
        }
    }

    // Transfers preallocated map nodes; std::string comparison does not throw.
    state.files.merge(staged);
}

ResourceBundle::~ResourceBundle()
{
    reset();
}

ResourceBundle::ResourceBundle(ResourceBundle &&other) noexcept
{
    paths_.swap(other.paths_);
}

ResourceBundle &ResourceBundle::operator=(ResourceBundle &&other) noexcept
{
    if (this != &other)
    {
        reset();
        paths_.swap(other.paths_);
    }
    return *this;
}

void ResourceBundle::reset() noexcept
{
    if (paths_.empty())
    {
        return;
    }
    auto &state = registry();
    const std::lock_guard<std::mutex> lock(state.mutex);
    for (const auto &path : paths_)
    {
        state.files.erase(path);
    }
    paths_.clear();
}
