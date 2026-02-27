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

// Include std.
#include <mutex>

// Include 3D Forest.
#include <Classifications.hpp>
#include <Datasets.hpp>
#include <ManagementStatusList.hpp>
#include <ModifierInterface.hpp>
#include <PageManager.hpp>
#include <Region.hpp>
#include <RenderingSettings.hpp>
#include <Segments.hpp>
#include <Settings.hpp>
#include <SpeciesList.hpp>
#include <Viewports.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Editor. */
class EXPORT_EDITOR Editor
{
public:
    /** Editor Data Type. */
    enum Type
    {
        TYPE_FILTER,
        TYPE_CLASSIFICATION,
        TYPE_CLIP_FILTER,
        TYPE_DATA_SET,
        TYPE_DESCRIPTOR,
        TYPE_ELEVATION,
        TYPE_INTENSITY,
        TYPE_SEGMENT,
        TYPE_SPECIES,
        TYPE_MANAGEMENT_STATUS,
        TYPE_PROJECT_NAME,
        TYPE_SETTINGS,
    };

    Editor();
    ~Editor();

    // File.
    void open(const std::string &path,
              const ImportSettings &settings = ImportSettings());
    void reload();
    void saveProject(const std::string &path);
    const std::string &projectPath() const { return projectPath_; }
    const std::string &projectName() const { return projectName_; }
    bool unsavedChanges() const { return unsavedChanges_; }
    void close();

    // Filters.
    void applyFilters();

    // Data sets.
    const Datasets &datasets() const { return datasets_; }
    Datasets &datasets() { return datasets_; }
    void setDatasets(const Datasets &datasets);
    const QueryFilterSet &datasetsFilter() const { return datasetsFilter_; }
    void setDatasetsFilter(const QueryFilterSet &filter);

    // Clip filter.
    const Region &clipFilter() const { return clipFilter_; }
    void setClipFilter(const Region &clipFilter);
    void resetClipFilter();

    Box<double> clipBoundary() const;
    const Box<double> &boundary() const { return datasets_.boundary(); }

    // Elevation.
    const Range<double> &elevationFilter() const { return elevationFilter_; }
    void setElevationFilter(const Range<double> &elevationFilter);

    // Descriptor.
    const Range<double> &descriptorFilter() const { return descriptorFilter_; }
    void setDescriptorFilter(const Range<double> &descriptorFilter);

    // Intensity.
    const Range<double> &intensityFilter() const { return intensityFilter_; }
    void setIntensityFilter(const Range<double> &intensityFilter);

    // Classifications.
    const Classifications &classifications() const { return classifications_; }
    void setClassifications(const Classifications &classifications);
    const QueryFilterSet &classificationsFilter() const
    {
        return classificationsFilter_;
    }
    void setClassificationsFilter(const QueryFilterSet &filter);

    // Segments.
    const Segments &segments() const { return segments_; }
    void setSegments(const Segments &segments);
    const Segment &segment(size_t id) const
    {
        return segments_[segments_.index(id)];
    }
    void setSegment(const Segment &segment);
    const QueryFilterSet &segmentsFilter() const { return segmentsFilter_; }
    void setSegmentsFilter(const QueryFilterSet &filter);

    Vector3<double> segmentColor(const Segment &segment) const;

    // Species.
    const SpeciesList &speciesList() const { return speciesList_; }
    void setSpeciesList(const SpeciesList &speciesList);
    const Species &species(size_t id) const
    {
        return speciesList_[speciesList_.index(id)];
    }
    void setSpecies(const Species &species);
    const QueryFilterSet &speciesFilter() const { return speciesFilter_; }
    void setSpeciesFilter(const QueryFilterSet &filter);

    // Management status.
    const ManagementStatusList &managementStatusList() const
    {
        return managementStatusList_;
    }
    void setManagementStatusList(
        const ManagementStatusList &managementStatusList);
    const ManagementStatus &managementStatus(size_t id) const
    {
        return managementStatusList_[managementStatusList_.index(id)];
    }
    void setManagementStatus(const ManagementStatus &managementStatus);
    const QueryFilterSet &managementStatusFilter() const
    {
        return managementStatusFilter_;
    }
    void setManagementStatusFilter(const QueryFilterSet &filter);

    // Settings.
    const Settings &settings() const { return settings_; }
    void setRenderingSettings(const RenderingSettings &renderingSettings);
    void setTreeSettings(const TreeSettings &treeSettings);
    void setUnitsSettings(const UnitsSettings &unitsSettings);
    void setViewSettings(const ViewSettings &viewSettings);

    // Modifiers.
    void addModifier(ModifierInterface *modifier);
    void runModifiers(Page *page);

    // View.
    void viewportsResize(size_t n);
    Viewports &viewports() { return viewports_; }
    const Viewports &viewports() const { return viewports_; }

    // Page.
    std::shared_ptr<PageData> readPage(size_t dataset, size_t index);
    void erasePage(size_t dataset, size_t index);

    // Lock.
    std::mutex editorMutex_;

protected:
    // Project data.
    std::string projectPath_;
    std::string projectName_;
    bool unsavedChanges_;

    Datasets datasets_;
    Dataset::Range datasetsRange_;

    Segments segments_;
    SpeciesList speciesList_;
    ManagementStatusList managementStatusList_;
    Settings settings_;
    Classifications classifications_;

    Region clipFilter_;
    Range<double> elevationFilter_;
    Range<double> descriptorFilter_;
    Range<double> intensityFilter_;
    QueryFilterSet classificationsFilter_;
    QueryFilterSet datasetsFilter_;
    QueryFilterSet segmentsFilter_;
    QueryFilterSet speciesFilter_;
    QueryFilterSet managementStatusFilter_;

    // Modifiers.
    std::vector<ModifierInterface *> modifiers_;

    // Viewports.
    Viewports viewports_;

    // Data.
    PageManager pageManager_;

    void readSettings();
    void writeSettings();

    void openProject(std::string path, bool reload);

    void openDataset(const std::string &path,
                     const ImportSettings &settings = ImportSettings());

    void setProjectPath(const std::string &projectPath);

    void updateAfterSet();
    void updateAfterRead();
};

inline std::ostream &operator<<(std::ostream &out, const Editor::Type &in)
{
    switch (in)
    {
        case Editor::TYPE_CLASSIFICATION:
            out << "CLASSIFICATION";
            break;
        case Editor::TYPE_CLIP_FILTER:
            out << "CLIP_FILTER";
            break;
        case Editor::TYPE_DATA_SET:
            out << "DATA_SET";
            break;
        case Editor::TYPE_DESCRIPTOR:
            out << "DESCRIPTOR";
            break;
        case Editor::TYPE_ELEVATION:
            out << "ELEVATION";
            break;
        case Editor::TYPE_INTENSITY:
            out << "INTENSITY";
            break;
        case Editor::TYPE_SEGMENT:
            out << "SEGMENT";
            break;
        case Editor::TYPE_PROJECT_NAME:
            out << "PROJECT_NAME";
            break;
        case Editor::TYPE_SETTINGS:
            out << "SETTINGS";
            break;
        default:
            out << "UNKNOWN";
            break;
    }

    return out;
}

#include <WarningsEnable.hpp>

#endif /* EDITOR_HPP */
