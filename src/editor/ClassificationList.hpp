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

/** @file ClassificationList.hpp */

#ifndef CLASSIFICATION_LIST_HPP
#define CLASSIFICATION_LIST_HPP

#include <string>
#include <unordered_set>
#include <vector>

#include <Classification.hpp>
#include <Vector3.hpp>

/** Classification List. */
class ClassificationList
{
public:
    ClassificationList();

    void clear();

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool b);

    size_t size() const { return classes_.size(); }

    bool isEnabled(size_t i) const { return classes_[i].enabled; }
    void setEnabled(size_t i, bool b) { classes_[i].enabled = b; }
    std::unordered_set<size_t> enabledList() const;
    const std::string &label(size_t i) const { return classes_[i].label; }

    void setEnabledAll(bool b);
    void setInvertAll();

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    std::vector<Classification> classes_;
    bool enabled_;
};

inline std::unordered_set<size_t> ClassificationList::enabledList() const
{
    std::unordered_set<size_t> list;

    for (size_t i = 0; i < classes_.size(); i++)
    {
        if (classes_[i].enabled)
        {
            list.insert(i);
        }
    }

    return list;
}

#endif /* CLASSIFICATION_LIST_HPP */
