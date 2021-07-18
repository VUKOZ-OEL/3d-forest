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

/** @file EditorClassifications.hpp */

#ifndef EDITOR_CLASSIFICATIONS_HPP
#define EDITOR_CLASSIFICATIONS_HPP

#include <Vector3.hpp>
#include <string>
#include <vector>

/** Editor Classifications. */
class EditorClassifications
{
public:
    struct Class
    {
        std::string label;
        bool enabled;
    };

    EditorClassifications();

    void clear();

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool b);

    size_t size() const { return classes_.size(); }

    bool isEnabled(size_t i) const { return classes_[i].enabled; }
    void setEnabled(size_t i, bool b) { classes_[i].enabled = b; }
    const std::string &label(size_t i) const { return classes_[i].label; }

    void setEnabledAll(bool b);
    void setInvertAll();

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    std::vector<Class> classes_;
    bool enabled_;
};

#endif /* EDITOR_CLASSIFICATIONS_HPP */
