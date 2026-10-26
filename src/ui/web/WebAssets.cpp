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

/** @file WebAssets.cpp */

// Include 3D Forest.
#include <Color.hpp>
#include <ResourceBundle.hpp>
#include <ThemeIcon.hpp>
#include <WebAssets.hpp>

// Include std.
#include <cstdio>

// Include local.
#define LOG_MODULE_NAME "WebAssets"
#include <Log.hpp>

std::string webColor(const Color &c)
{
    if (!c.isValid())
        return {};
    char s[10];
    std::snprintf(s,
                  sizeof(s),
                  "#%02x%02x%02x%02x",
                  c.red(),
                  c.green(),
                  c.blue(),
                  c.alpha());
    return s;
}

std::string webIcon(const ThemeIcon &icon)
{
    if (icon.sourceBase().empty())
        return {};
    for (const char *suffix : {"-16px-color.png",
                               "-16px.png",
                               "-20px-color.png",
                               "-20px.png",
                               "-24px-color.png",
                               "-24px.png"})
        if (ResourceRegistry::get(icon.sourceBase() + suffix))
            return webResourceImage(icon.sourceBase() + suffix);
    return {};
}

std::string webResourceImage(const std::string &path)
{
    ResourceData bytes = ResourceRegistry::get(path);
    if (!bytes || bytes->empty())
        return {};
    static const char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const bool svg =
        path.size() >= 4 && path.compare(path.size() - 4, 4, ".svg") == 0;
    std::string result =
        svg ? "data:image/svg+xml;base64," : "data:image/png;base64,";
    for (std::size_t i = 0; i < bytes->size(); i += 3)
    {
        unsigned n = unsigned((*bytes)[i]) << 16;
        if (i + 1 < bytes->size())
            n |= unsigned((*bytes)[i + 1]) << 8;
        if (i + 2 < bytes->size())
            n |= (*bytes)[i + 2];
        result += chars[(n >> 18) & 63];
        result += chars[(n >> 12) & 63];
        result += i + 1 < bytes->size() ? chars[(n >> 6) & 63] : '=';
        result += i + 2 < bytes->size() ? chars[n & 63] : '=';
    }
    return result;
}
