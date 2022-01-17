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

/** @file EditorDatabase.hpp */

#ifndef EDITOR_DATABASE_HPP
#define EDITOR_DATABASE_HPP

#include <ClipFilter.hpp>
#include <EditorClassifications.hpp>
#include <EditorDatasets.hpp>
#include <EditorFilter.hpp>
#include <EditorLayers.hpp>
#include <EditorSettings.hpp>
#include <EditorViewports.hpp>

/** Editor Database. */
class EditorDatabase
{
public:
    EditorDatabase();
    ~EditorDatabase();

    // File
    void newProject();
    void openProject(const std::string &path);
    void openDataset(
        const std::string &path,
        const EditorSettingsImport &settings = EditorSettingsImport());
    void saveProject(const std::string &path);
    const std::string &projectPath() const { return path_; }
    const std::string &projectName() const { return projectName_; }
    bool hasUnsavedChanges() const { return unsavedChanges_; }

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

    Box<double> clipBoundary() const;
    const Box<double> &boundary() const { return datasets_.boundary(); }

    // Data sets
    const EditorDatasets &datasets() const { return datasets_; }
    void setDatasets(const EditorDatasets &datasets);

    // Layers
    const EditorLayers &layers() const { return layers_; }
    void setLayers(const EditorLayers &layers);

    // Settings
    const EditorSettings &settings() const { return settings_; }
    void setSettingsView(const EditorSettingsView &settings);

    // Filters
    void addFilter(EditorFilter *filter);
    void applyFilters(EditorPage *page);

    // View
    void viewportsResize(size_t n);
    EditorViewports &viewports() { return viewports_; }
    const EditorViewports &viewports() const { return viewports_; }

protected:
    // Project data
    std::string path_;
    std::string projectName_;
    bool unsavedChanges_;

    EditorDatasets datasets_;
    EditorLayers layers_;
    EditorSettings settings_;
    EditorClassifications classifications_;

    ClipFilter clipFilter_;

    // Filters
    std::vector<EditorFilter *> filters_;

    // Viewports
    EditorViewports viewports_;

    void updateAfterRead();
};

#endif /* EDITOR_DATABASE_HPP */
