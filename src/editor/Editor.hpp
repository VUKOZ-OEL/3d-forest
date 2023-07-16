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

/** @file Editor.hpp */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <mutex>

#include <Classifications.hpp>
#include <Datasets.hpp>
#include <Layers.hpp>
#include <ModifierInterface.hpp>
#include <PageManager.hpp>
#include <Region.hpp>
#include <Settings.hpp>
#include <Viewports.hpp>

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Editor. */
class EXPORT_EDITOR Editor
{
public:
    /** Editor Data Type. */
    enum Type
    {
        TYPE_CLASSIFICATION,
        TYPE_CLIP_FILTER,
        TYPE_DATA_SET,
        TYPE_DESCRIPTOR,
        TYPE_ELEVATION,
        TYPE_LAYER,
        TYPE_PROJECT_NAME,
        TYPE_SETTINGS,
    };

    Editor();
    ~Editor();

    // File
    void open(const std::string &path,
              const SettingsImport &settings = SettingsImport());
    void saveProject(const std::string &path);
    const std::string &projectPath() const { return path_; }
    const std::string &projectName() const { return projectName_; }
    bool hasUnsavedChanges() const { return unsavedChanges_; }
    void close();

    // Classifications
    const Classifications &classifications() const { return classifications_; }
    void setClassifications(const Classifications &classifications);
    const QueryFilterSet &classificationsFilter() const
    {
        return classificationsFilter_;
    }
    void setClassificationsFilter(const QueryFilterSet &filter);

    // Clip filter
    const Region &clipFilter() const { return clipFilter_; }
    void setClipFilter(const Region &clipFilter);
    void resetClipFilter();

    Box<double> clipBoundary() const;
    const Box<double> &boundary() const { return datasets_.boundary(); }

    // Elevation
    const Range<double> &elevationFilter() const { return elevationFilter_; }
    void setElevationFilter(const Range<double> &elevationFilter);

    // Descriptor
    const Range<double> &descriptorFilter() const { return descriptorFilter_; }
    void setDescriptorFilter(const Range<double> &descriptorFilter);

    // Data sets
    const Datasets &datasets() const { return datasets_; }
    void setDatasets(const Datasets &datasets);
    const QueryFilterSet &datasetsFilter() const { return datasetsFilter_; }
    void setDatasetsFilter(const QueryFilterSet &filter);

    // Layers
    const Layers &layers() const { return layers_; }
    void setLayers(const Layers &layers);
    const QueryFilterSet &layersFilter() const { return layersFilter_; }
    void setLayersFilter(const QueryFilterSet &filter);

    // Settings
    const Settings &settings() const { return settings_; }
    void setSettingsView(const SettingsView &settings);

    // Modifiers
    void addModifier(ModifierInterface *modifier);
    void runModifiers(Page *page);

    // View
    void viewportsResize(size_t n);
    Viewports &viewports() { return viewports_; }
    const Viewports &viewports() const { return viewports_; }

    // Page
    std::shared_ptr<PageData> readPage(size_t dataset, size_t index);
    void erasePage(size_t dataset, size_t index);

    // Lock
    void lock();
    void unlock();

protected:
    std::mutex mutex_;

    // Project data
    std::string path_;
    std::string projectName_;
    bool unsavedChanges_;

    Datasets datasets_;
    Layers layers_;
    Settings settings_;
    Classifications classifications_;

    Region clipFilter_;
    Range<double> elevationFilter_;
    Range<double> descriptorFilter_;
    QueryFilterSet classificationsFilter_;
    QueryFilterSet datasetsFilter_;
    QueryFilterSet layersFilter_;

    // Modifiers
    std::vector<ModifierInterface *> modifiers_;

    // Viewports
    Viewports viewports_;

    // Data
    PageManager pageManager_;

    void openProject(const std::string &path);

    void openDataset(const std::string &path,
                     const SettingsImport &settings = SettingsImport());

    void updateAfterRead();
};

#include <WarningsEnable.hpp>

#endif /* EDITOR_HPP */
