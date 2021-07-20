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

/** @file EditorBase.hpp */

#ifndef EDITOR_BASE_HPP
#define EDITOR_BASE_HPP

#include <ClipFilter.hpp>
#include <EditorCache.hpp>
#include <EditorClassifications.hpp>
#include <EditorDataSets.hpp>
#include <EditorDatabases.hpp>
#include <EditorFilter.hpp>
#include <EditorLayers.hpp>
#include <EditorSettings.hpp>

/** Editor Base. */
class EditorBase
{
public:
    EditorBase();
    ~EditorBase();

    void open(const std::string &path);
    void addFile(const std::string &path,
                 const EditorSettingsImport &settings = EditorSettingsImport());
    bool hasFileIndex(const std::string &path);
    void write(const std::string &path);
    const std::string &path() const { return path_; }
    const std::string &projectName() const { return projectName_; }
    bool hasUnsavedChanges() const { return unsavedChanges_; }
    void close();

    // Filters
    void addFilter(EditorFilter *filter);
    void applyFilters(EditorTile *tile);

    // Data sets
    size_t databaseSize() const { return database_.size(); }
    const EditorDatabase &databaseAt(size_t i) const { return database_.at(i); }
    const EditorDatabase &databaseId(size_t id) const
    {
        return database_.key(id);
    }

    const EditorDataSets &dataSets() const { return dataSets_; }
    void setDataSets(const EditorDataSets &dataSets);

    // Layers
    const EditorLayers &layers() const { return layers_; }
    void setLayers(const EditorLayers &layers);

    // Classifications
    const EditorClassifications &classifications() const
    {
        return classifications_;
    }
    void setClassifications(const EditorClassifications &classifications);

    // Clip filter
    const ClipFilter &clipFilter() const { return clipFilter_; }
    void setClipFilter(const ClipFilter &clipFilter);
    void resetClipFilter();

    // Settings
    const EditorSettings &settings() const { return settings_; }
    void setSettingsView(const EditorSettingsView &settings);

    // Boundary
    const Aabb<double> &boundary() const { return database_.boundary(); }
    const Aabb<double> &boundaryView() const
    {
        return database_.boundaryView();
    }

    // Tile
    void select(std::vector<FileIndex::Selection> &selected);
    Aabb<double> selection() const;
    EditorTile *tile(size_t dataset, size_t index)
    {
        return working_.tile(dataset, index);
    }
    void flush(EditorTile *tile);

    // View
    void setNumberOfViewports(size_t n);
    void updateCamera(size_t viewport, const Camera &camera);
    bool loadView();
    void tileViewClear();
    size_t tileViewSize(size_t viewport) const
    {
        return viewports_[viewport]->tileSize();
    }
    EditorTile &tileView(size_t viewport, size_t index)
    {
        return viewports_[viewport]->tile(index);
    }

    // Cache
    void clearCache();

protected:
    // Project
    std::string path_;
    std::string projectName_;
    bool unsavedChanges_;

    EditorDatabases database_;
    EditorDataSets dataSets_;
    EditorLayers layers_;
    ClipFilter clipFilter_;
    EditorSettings settings_;
    EditorClassifications classifications_;

    // Filter
    std::vector<EditorFilter *> filters_;

    void openUpdate();
    void resetRendering();

    // Cache
    std::vector<std::shared_ptr<EditorCache>> viewports_;
    EditorCache working_;
};

#endif /* EDITOR_BASE_HPP */
