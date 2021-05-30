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
    @file EditorBase.hpp
*/

#ifndef EDITOR_BASE_HPP
#define EDITOR_BASE_HPP

#include <Aabb.hpp>
#include <Camera.hpp>
#include <ClipFilter.hpp>
#include <EditorCache.hpp>
#include <EditorDataSet.hpp>
#include <EditorFilter.hpp>
#include <EditorLayer.hpp>
#include <EditorTile.hpp>
#include <map>
#include <string>
#include <vector>

/** Editor Base. */
class EditorBase
{
public:
    EditorBase();
    ~EditorBase();

    void open(const std::string &path);
    void write(const std::string &path);
    const std::string &path() const { return path_; }
    bool hasUnsavedChanges() const { return unsavedChanges_; }
    void close();

    void addFilter(EditorFilter *filter);

    size_t dataSetSize() const { return dataSets_.size(); }
    const EditorDataSet &dataSet(size_t i) const { return *dataSets_[i]; }

    size_t layerSize() const { return layers_.size(); }
    const EditorLayer &layer(size_t i) const { return layers_[i]; }

    const ClipFilter &clipFilter() const { return clipFilter_; }

    void setVisibleDataSet(size_t i, bool visible);
    void setVisibleLayer(size_t i, bool visible);
    void setClipFilter(const ClipFilter &clipFilter);
    void resetClipFilter();

    const Aabb<double> &boundary() const { return boundary_; }
    const Aabb<double> &boundaryView() const { return boundaryView_; }
    void updateCamera(const Camera &camera);
    bool loadView();

    void select(std::vector<OctreeIndex::Selection> &selected);
    Aabb<double> selection() const;

    void tileViewClear();
    size_t tileViewSize() const { return view_.size(); }
    EditorTile &tileView(size_t i) { return *view_[i]; }
    const EditorTile &tileView(size_t i) const { return *view_[i]; }

    EditorTile *tile(size_t d, size_t c);

protected:
    // Project
    std::string path_;
    std::string projectName_;
    bool unsavedChanges_;

    std::vector<std::shared_ptr<EditorDataSet>> dataSets_;
    std::vector<EditorLayer> layers_;
    ClipFilter clipFilter_;

    // Filter
    std::vector<EditorFilter *> filters_;

    void applyFilters(EditorTile *tile);

    // Database
    Aabb<double> boundary_;
    Aabb<double> boundaryView_;

    void updateBoundary();

    // Cache
    struct Key
    {
        size_t dataSetId;
        size_t tileId;

        bool operator<(const Key &rhs) const;
    };
    size_t cacheSizeMax_;
    std::map<Key, std::shared_ptr<EditorTile>> cache_;

    bool isCached(size_t d, size_t c) const;

    // View
    std::vector<std::shared_ptr<EditorTile>> view_;

    void loadView(size_t idx);
    void resetRendering();
};

#endif /* EDITOR_BASE_HPP */
