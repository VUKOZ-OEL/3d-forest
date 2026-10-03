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

/** @file Icon.cpp */

// Include std.
#include <utility>

// Include 3D Forest.
#include <Icon.hpp>

// Include local.
#define LOG_MODULE_NAME "Icon"
#include <Log.hpp>

#include <Icon.hpp>

Icon::Icon(const std::string &fileName)
{
    addFile(fileName);
}

Icon::Icon(const Pixmap &pixmap)
{
    addPixmap(pixmap);
}

void Icon::clear()
{
    entries_.clear();
}

void Icon::addFile(const std::string &fileName, Mode mode, State state)
{
    if (fileName.empty())
    {
        return;
    }

    entries_.push_back(Entry{fileName, mode, state});
}

void Icon::addPixmap(const Pixmap &pixmap, Mode mode, State state)
{
    if (pixmap.isNull())
    {
        return;
    }

    entries_.push_back(Entry{pixmap, mode, state});
}
