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

/** @file Icon.hpp */

#ifndef ICON_HPP
#define ICON_HPP

// Include std.
#include <string>
#include <variant>
#include <vector>

// Include 3D Forest.
#include <Pixmap.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Theme Icon. */
class EXPORT_UI_COMMON Icon
{
public:
    enum Mode
    {
        Normal,
        Disabled,
        Active,
        Selected
    };

    enum State
    {
        Off,
        On
    };

    struct Entry
    {
        std::variant<std::string, Pixmap> source;
        Mode mode{Normal};
        State state{Off};
    };

    Icon() = default;

    explicit Icon(const std::string &fileName);
    explicit Icon(const Pixmap &pixmap);

    void clear();

    // Reports whether there are any sources.
    // File existence/decoding is checked by the backend.
    bool isNull() const { return entries_.empty(); }

    void addFile(const std::string &fileName,
                 Mode mode = Normal,
                 State state = Off);

    void addPixmap(const Pixmap &pixmap, Mode mode = Normal, State state = Off);

    const std::vector<Entry> &entries() const { return entries_; }

private:
    std::vector<Entry> entries_;
};

#include <WarningsEnable.hpp>

#endif /* ICON_HPP */
