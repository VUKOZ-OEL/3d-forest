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

/** @file EditorDataSets.hpp */

#ifndef EDITOR_DATA_SETS_HPP
#define EDITOR_DATA_SETS_HPP

#include <EditorDataSet.hpp>
#include <unordered_map>

/** Editor Data Sets. */
class EditorDataSets
{
public:
    EditorDataSets();

    void clear();

    size_t size() const { return dataSets_.size(); }
    const EditorDataSet &at(size_t i) const { return dataSets_[i]; }
    void remove(size_t i);

    size_t id(size_t i) const { return dataSets_[i].id(); }
    size_t index(size_t id) { return hashTable_[id]; }
    size_t unusedId() const;

    bool isEnabled(size_t i) const { return dataSets_[i].isEnabled(); }
    void setEnabled(size_t i, bool b);
    void setEnabledAll(bool b);
    void setInvertAll();

    const std::string &label(size_t i) const { return dataSets_[i].label(); }
    void setLabel(size_t i, const std::string &label);

    const Vector3<float> &color(size_t i) const { return dataSets_[i].color(); }
    void setColor(size_t i, const Vector3<float> &color);

    const std::string &fileName(size_t i) const
    {
        return dataSets_[i].fileName();
    }
    const std::string &dateCreated(size_t i) const
    {
        return dataSets_[i].dateCreated();
    }

    const Vector3<double> &translation(size_t i) const
    {
        return dataSets_[i].translation();
    }
    void setTranslation(size_t i, const Vector3<double> &translation);

    void read(const std::string &path,
              const std::string &projectPath,
              const EditorSettingsImport &settings,
              const Aabb<double> &projectBoundary);
    void read(const Json &in, const std::string &projectPath);
    Json &write(Json &out) const;

protected:
    std::vector<EditorDataSet> dataSets_;
    std::unordered_map<size_t, size_t> hashTable_;
};

#endif /* EDITOR_DATA_SETS_HPP */
