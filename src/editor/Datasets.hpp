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

/** @file Datasets.hpp */

#ifndef DATASETS_HPP
#define DATASETS_HPP

#include <unordered_map>
#include <unordered_set>

#include <Dataset.hpp>
#include <ExportEditor.hpp>
#include <QueryFilterSet.hpp>

/** Dataset List. */
class EXPORT_EDITOR Datasets
{
public:
    Datasets();

    void clear();

    size_t size() const { return datasets_.size(); }

    const std::unordered_set<size_t> &idList() const { return datasetsIds_; }

    const Dataset &at(size_t i) const { return datasets_[i]; }
    const Dataset &key(size_t id) const
    {
        const auto &it = hashTable_.find(id);
        if (it != hashTable_.end())
        {
            return datasets_[it->second];
        }
        THROW("Invalid dataset id");
    }

    void erase(size_t i);

    size_t id(size_t i) const { return datasets_[i].id(); }
    size_t index(size_t id) { return hashTable_[id]; }
    size_t unusedId() const;

    const std::string &label(size_t i) const { return datasets_[i].label(); }
    void setLabel(size_t i, const std::string &label);

    const Vector3<double> &color(size_t i) const
    {
        return datasets_[i].color();
    }
    void setColor(size_t i, const Vector3<double> &color);

    const std::string &fileName(size_t i) const
    {
        return datasets_[i].fileName();
    }
    const std::string &dateCreated(size_t i) const
    {
        return datasets_[i].dateCreated();
    }

    const Vector3<double> &translation(size_t i) const
    {
        return datasets_[i].translation();
    }
    void setTranslation(size_t i, const Vector3<double> &translation);

    const Vector3<double> &scaling(size_t i) const
    {
        return datasets_[i].scaling();
    }

    const Vector3<double> &scalingFile(size_t i) const
    {
        return datasets_[i].scalingFile();
    }

    const Box<double> &boundary() const { return boundary_; }
    Box<double> boundary(const QueryFilterSet &datasetFilter) const;
    void updateBoundary();

    uint64_t nPoints() const;
    uint64_t nPoints(const QueryFilterSet &datasetFilter) const;

    void selectPages(const QueryFilterSet &datasetFilter,
                     const Box<double> &box,
                     std::vector<IndexFile::Selection> &selected) const;

    void read(const std::string &path,
              const std::string &projectPath,
              const SettingsImport &settings,
              const Box<double> &projectBoundary);
    void read(const Json &in, const std::string &projectPath);
    Json &write(Json &out) const;

protected:
    std::vector<Dataset> datasets_;
    std::unordered_set<size_t> datasetsIds_;
    std::unordered_map<size_t, size_t> hashTable_;
    Box<double> boundary_;
};

std::ostream &operator<<(std::ostream &os, const Datasets &obj);

#endif /* DATASETS_HPP */
