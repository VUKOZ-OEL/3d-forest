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

/**
    @file EditorSettings.hpp
*/

#ifndef EDITOR_SETTINGS_HPP
#define EDITOR_SETTINGS_HPP

#include <string>
#include <vector>

/** Editor Settings. */
class EditorSettings
{
public:
    /** Editor Settings View. */
    class View
    {
    public:
        View();

        float pointSize() const;
        void setPointSize(float size);

        size_t colorSourceSize() const;
        const char *colorSourceString(size_t id) const;
        bool isColorSourceEnabled(size_t id) const;
        void setColorSourceEnabled(size_t id, bool v);

    protected:
        float pointSize_;
        std::vector<std::string> colorSourceString_;
        std::vector<bool> colorSourceEnabled_;
    };

    const View &view() const { return view_; }
    void setView(const View &view);

protected:
    View view_;
};

#endif /* EDITOR_SETTINGS_HPP */
