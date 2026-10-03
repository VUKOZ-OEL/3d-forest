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

/** @file ResourceBundle.hpp */

#ifndef RESOURCE_BUNDLE_HPP
#define RESOURCE_BUNDLE_HPP

// Include std.
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

using ResourceBytes = std::vector<std::uint8_t>;
using ResourceData = std::shared_ptr<const ResourceBytes>;

struct ResourceEntry
{
    const char *path;
    const std::uint8_t *data;
    std::size_t size;
};

// Implemented once in the shared common library, never separately per plugin.
class EXPORT_UI_COMMON ResourceRegistry
{
public:
    // Exact UTF-8 logical name, e.g. :/FilterClassificationResources/icon.png.
    // A null pointer means missing; a non-null empty vector is an empty file.
    static ResourceData get(const std::string &path);
};

// Owns registration, not the generated arrays. Bytes are copied into the
// common library, so previously returned ResourceData survives plugin unload.
class EXPORT_UI_COMMON ResourceBundle
{
public:
    ResourceBundle() = default;
    explicit ResourceBundle(std::initializer_list<ResourceEntry> entries);
    ~ResourceBundle();

    ResourceBundle(const ResourceBundle &) = delete;
    ResourceBundle &operator=(const ResourceBundle &) = delete;
    ResourceBundle(ResourceBundle &&other) noexcept;
    ResourceBundle &operator=(ResourceBundle &&other) noexcept;

    void reset() noexcept;

private:
    std::vector<std::string> paths_;
};

#include <WarningsEnable.hpp>

#endif
