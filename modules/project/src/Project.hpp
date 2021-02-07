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
    @file Project.hpp
*/

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <ClipFilter.hpp>
#include <Json.hpp>
#include <ProjectDataSet.hpp>
#include <ProjectLayer.hpp>
#include <string>
#include <vector>

/** Project. */
class Project
{
public:
    Project();
    ~Project();

    void read(const std::string &path);
    void write(const std::string &path);
    void clear();

    const std::string &path() const { return path_; }

    size_t dataSetSize() const { return dataSets_.size(); }
    const ProjectDataSet &dataSet(size_t i) const { return *dataSets_[i]; }
    size_t layerSize() const { return layers_.size(); }
    const ProjectLayer &layer(size_t i) const { return layers_[i]; }
    const ClipFilter &clipFilter() const { return clipFilter_; }

    void setVisibleDataSet(size_t i, bool visible);
    void setVisibleLayer(size_t i, bool visible);
    void setClipFilter(const ClipFilter &clipFilter);

protected:
    std::string path_;
    std::string projectName_;
    std::vector<std::shared_ptr<ProjectDataSet>> dataSets_;
    std::vector<ProjectLayer> layers_;
    ClipFilter clipFilter_;
};

#endif /* PROJECT_HPP */
