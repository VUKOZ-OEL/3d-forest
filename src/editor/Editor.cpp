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

/** @file Editor.cpp */

// Include 3D Forest.
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "Editor"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

static const char *EDITOR_FILE_NAME_SETTINGS = "settings.json";
static const char *EDITOR_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_KEY_DATA_SET = "datasets";
static const char *EDITOR_KEY_SEGMENT = "segments";
static const char *EDITOR_KEY_SPECIES = "species";
static const char *EDITOR_KEY_MANAGEMENT_STATUS = "managementStatus";
static const char *EDITOR_KEY_SETTINGS = "settings";
// static const char *EDITOR_KEY_CLASSIFICATIONS = "classifications";
// static const char *EDITOR_KEY_CLIP_FILTER = "clipFilter";
// static const char *EDITOR_KEY_ELEVATION_RANGE = "elevationRange";

Editor::Editor()
{
    LOG_DEBUG(<< "Start creating the editor.");
    readSettings();
    close();
    viewportsResize(1);
    LOG_DEBUG(<< "Finished creating the editor.");
}

Editor::~Editor()
{
    LOG_DEBUG(<< "Destroy.");
}

void Editor::readSettings()
{
    LOG_DEBUG(<< "Start reading application settings.");

    Settings newSettings;

    // Create default settings.
    try
    {
        if (!File::exists(EDITOR_FILE_NAME_SETTINGS))
        {
            Json out;
            toJson(out[EDITOR_KEY_SETTINGS], newSettings);
            out.write(EDITOR_FILE_NAME_SETTINGS);
        }
    }
    catch (std::exception &e)
    {
        LOG_WARNING(<< "Failed to create application settings file, "
                    << "error message <" << e.what() << ">.");
    }
    catch (...)
    {
        LOG_WARNING(<< "Failed to create application settings file, "
                       "unknown exception is raised.");
    }

    // Read current settings.
    try
    {
        Json in;
        in.read(EDITOR_FILE_NAME_SETTINGS);
        fromJson(newSettings, in[EDITOR_KEY_SETTINGS]);
        settings_ = newSettings;
    }
    catch (std::exception &e)
    {
        LOG_WARNING(<< "Cancel reading application settings, error message <"
                    << e.what() << ">. Using settings <" << toString(settings_)
                    << ">.");
        return;
    }
    catch (...)
    {
        LOG_WARNING(<< "Cancel reading application settings, unknown exception "
                       "is raised. Using settings <"
                    << toString(settings_) << ">.");
        return;
    }

    LOG_DEBUG(<< "Finished reading application settings <"
              << toString(settings_) << ">.");
}

void Editor::writeSettings()
{
    LOG_DEBUG(<< "Start writing application settings.");

    // Write the current settings.
    try
    {
        Json out;
        toJson(out[EDITOR_KEY_SETTINGS], settings_);
        out.write(EDITOR_FILE_NAME_SETTINGS);
    }
    catch (std::exception &e)
    {
        LOG_WARNING(<< "Failed to create application settings file, "
                    << "error message <" << e.what() << ">.");
    }
    catch (...)
    {
        LOG_WARNING(<< "Failed to create application settings file, "
                       "unknown exception is raised.");
    }

    LOG_DEBUG(<< "Finished writing application settings.");
}

void Editor::close()
{
    LOG_DEBUG(<< "Start closing the editor.");

    setProjectPath(File::join(File::currentPath(), "untitled.json"));
    projectName_ = "Untitled";

    datasets_.clear();
    datasetsFilter_.clear();
    datasetsFilter_.setEnabled(true);

    segments_.setDefault();
    segmentsFilter_.clear();
    segmentsFilter_.setEnabled(0, true);
    segmentsFilter_.setEnabled(true);

    speciesList_ = settings().defaultSpeciesList();
    speciesFilter_.clear();
    for (size_t i = 0; i < speciesList_.size(); i++)
    {
        speciesFilter_.setEnabled(speciesList_[i].id, true);
    }
    speciesFilter_.setEnabled(true);

    managementStatusList_ = settings().defaultManagementStatusList();
    managementStatusFilter_.clear();
    for (size_t i = 0; i < managementStatusList_.size(); i++)
    {
        managementStatusFilter_.setEnabled(managementStatusList_[i].id, true);
    }
    managementStatusFilter_.setEnabled(true);

    classifications_.clear();
    classificationsFilter_.clear();
    for (size_t i = 0; i < classifications_.size(); i++)
    {
        classificationsFilter_.setEnabled(i, true);
    }
    classificationsFilter_.setEnabled(true);

    viewports_.clearContent();

    clipFilter_.clear();
    elevationFilter_.clear();
    descriptorFilter_.set(0.0, 1.0);
    intensityFilter_.set(0.0, 1.0);

    unsavedChanges_ = false;

    LOG_DEBUG(<< "Finished closing the editor.");
}

void Editor::open(const std::string &path, const ImportSettings &settings)
{
    LOG_DEBUG(<< "Start opening new project/dataset path <" << path << ">.");

    // Get filename extension in lower case (no UTF).
    std::string ext = toLower(File::fileExtension(path));

    if (ext == "json")
    {
        // Open new project from json format.
        openProject(path);
    }
    else
    {
        // Add new dataset to existing project.
        openDataset(path, settings);
    }

    LOG_DEBUG(<< "Finished opening the project/dataset.");
}

void Editor::openProject(const std::string &path)
{
    LOG_DEBUG(<< "Start opening project from path <" << path << ">.");

    close();

    // Load data.
    Json in;
    in.read(path);

    if (!in.typeObject())
    {
        LOG_DEBUG(<< "Cancel opening new project, exception is raised.");
        THROW("Project file '" + path + "' is not in JSON object");
    }

    try
    {
        setProjectPath(path);

        // Project name.
        if (in.contains(EDITOR_KEY_PROJECT_NAME))
        {
            fromJson(projectName_, in[EDITOR_KEY_PROJECT_NAME]);
        }

        // Data sets.
        if (in.contains(EDITOR_KEY_DATA_SET))
        {
            fromJson(datasets_,
                     in[EDITOR_KEY_DATA_SET],
                     projectPath_,
                     datasetsFilter_);
        }

        // Segments.
        if (in.contains(EDITOR_KEY_SEGMENT))
        {
            fromJson(segments_, in[EDITOR_KEY_SEGMENT]);
        }

        // Species.
        if (in.contains(EDITOR_KEY_SPECIES))
        {
            fromJson(speciesList_, in[EDITOR_KEY_SPECIES]);
        }

        // Management status.
        if (in.contains(EDITOR_KEY_MANAGEMENT_STATUS))
        {
            fromJson(managementStatusList_, in[EDITOR_KEY_MANAGEMENT_STATUS]);
        }

        // Classifications.
        // if (in.contains(EDITOR_KEY_CLASSIFICATIONS))
        // {
        //     fromJson(classifications_, in[EDITOR_KEY_CLASSIFICATIONS]);
        // }

        // Settings.
        // if (in.contains(EDITOR_KEY_SETTINGS))
        // {
        //     fromJson(settings_, in[EDITOR_KEY_SETTINGS]);
        // }

        // Clip filter.
        // if (in.contains(EDITOR_KEY_CLIP_FILTER))
        // {
        //     fromJson(clipFilter_, in[EDITOR_KEY_CLIP_FILTER]);
        // }
        // else
        // {
        //     clipFilter_.clear();
        // }

        // Elevation range.
        // if (in.contains(EDITOR_KEY_ELEVATION_RANGE))
        // {
        //     fromJson(elevationFilter_, in[EDITOR_KEY_ELEVATION_RANGE]);
        // }
    }
    catch (...)
    {
        LOG_ERROR(<< "Cancel opening new project, exception is raised.");
        close();
        throw;
    }

    // Load mesh list.
    try
    {
        segments_.importMeshList(path, 1.0);
    }
    catch (...)
    {
        LOG_ERROR(<< "Unable to read mesh list, exception is raised.");
    }

    // Update the editor.
    updateAfterRead();

    LOG_DEBUG(<< "Finished opening project.");
}

void Editor::saveProject(const std::string &path)
{
    LOG_DEBUG(<< "Start saving the project to path <" << path << ">.");

    // Save data.
    Json out;

    toJson(out[EDITOR_KEY_PROJECT_NAME], projectName_);
    toJson(out[EDITOR_KEY_DATA_SET], datasets_);
    toJson(out[EDITOR_KEY_SEGMENT], segments_);
    toJson(out[EDITOR_KEY_SPECIES], speciesList_);
    toJson(out[EDITOR_KEY_MANAGEMENT_STATUS], managementStatusList_);
    // toJson(out[EDITOR_KEY_CLASSIFICATIONS], classifications_);
    // toJson(out[EDITOR_KEY_SETTINGS], settings_);
    // toJson(out[EDITOR_KEY_CLIP_FILTER], clipFilter_);
    // toJson(out[EDITOR_KEY_ELEVATION_RANGE], elevationFilter_);

    out.write(path);

    // Save mesh list.
    segments_.exportMeshList(path, 1.0);

    // Mark as saved.
    unsavedChanges_ = false;

    LOG_DEBUG(<< "Finished saving the project.");
}

void Editor::openDataset(const std::string &path,
                         const ImportSettings &settings)
{
    LOG_DEBUG(<< "Start opening new dataset from path <" << path << ">.");

    try
    {
        std::string projectPath = projectPath_;
        if (File::fileName(projectPath) == "untitled.json")
        {
            projectPath = File::resolvePath(path, File::currentPath());
            projectPath = File::replaceExtension(projectPath, ".json");
        }

        datasets_.read(path,
                       projectPath,
                       settings,
                       datasets_.boundary(),
                       datasetsFilter_);

        setProjectPath(projectPath);
    }
    catch (...)
    {
        LOG_DEBUG(<< "Cancel opening new dataset, exception is raised.");
        throw;
    }

    updateAfterRead();

    unsavedChanges_ = true;

    LOG_DEBUG(<< "Finished opening new dataset.");
}

void Editor::setProjectPath(const std::string &projectPath)
{
    LOG_DEBUG(<< "Set project path to <" << projectPath << ">.");
    projectPath_ = projectPath;
}

void Editor::setClassifications(const Classifications &classifications)
{
    LOG_DEBUG(<< "Set classifications.");
    classifications_ = classifications;
    // viewClearRendered();
    unsavedChanges_ = true;
}

void Editor::setClassificationsFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set classifications filter.");
    classificationsFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setClassification(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setClipFilter(const Region &clipFilter)
{
    LOG_DEBUG(<< "Set clip filter <" << clipFilter << ">.");
    clipFilter_ = clipFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setRegion(clipFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::resetClipFilter()
{
    LOG_DEBUG(<< "Reset clip filter.");
    clipFilter_.box = clipFilter_.boundary;
    setClipFilter(clipFilter_);
}

Box<double> Editor::clipBoundary() const
{
    if (clipFilter_.shape == Region::Shape::BOX)
    {
        return clipFilter_.box;
    }

    return clipFilter_.boundary;
}

void Editor::setElevationFilter(const Range<double> &elevationFilter)
{
    LOG_DEBUG(<< "Set elevation filter <" << elevationFilter << ">.");
    elevationFilter_ = elevationFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setElevation(elevationFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::setDescriptorFilter(const Range<double> &descriptorFilter)
{
    LOG_DEBUG(<< "Set descriptor filter <" << descriptorFilter << ">.");
    descriptorFilter_ = descriptorFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setDescriptor(descriptorFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::setIntensityFilter(const Range<double> &intensityFilter)
{
    LOG_DEBUG(<< "Set intensity filter <" << intensityFilter << ">.");
    intensityFilter_ = intensityFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setIntensity(intensityFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::setDatasets(const Datasets &datasets)
{
    LOG_DEBUG(<< "Set datasets.");

    size_t datasetsSizeOld = datasets_.size();

    datasets_ = datasets;

    if (datasetsSizeOld != datasets_.size())
    {
        updateAfterSet();
        unsavedChanges_ = true;
    }
}

void Editor::setDatasetsFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set datasets filter.");
    datasetsFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setDataset(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setSegments(const Segments &segments)
{
    LOG_DEBUG(<< "Set segments.");
    segments_ = segments;
    unsavedChanges_ = true;
}

void Editor::setSegment(const Segment &segment)
{
    LOG_DEBUG(<< "Set segments.");
    segments_[segments_.index(segment.id)] = segment;
    unsavedChanges_ = true;
}

void Editor::setSegmentsFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set segments filter.");
    segmentsFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setSegment(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setSpeciesList(const SpeciesList &speciesList)
{
    LOG_DEBUG(<< "Set species list.");
    speciesList_ = speciesList;
    unsavedChanges_ = true;
}

void Editor::setSpecies(const Species &species)
{
    LOG_DEBUG(<< "Set species.");
    speciesList_[speciesList_.index(species.id)] = species;
    unsavedChanges_ = true;
}

void Editor::setSpeciesFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set species filter.");
    speciesFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setSpecies(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setManagementStatusList(
    const ManagementStatusList &managementStatusList)
{
    LOG_DEBUG(<< "Set management status list.");
    managementStatusList_ = managementStatusList;
    unsavedChanges_ = true;
}

void Editor::setManagementStatus(const ManagementStatus &managementStatus)
{
    LOG_DEBUG(<< "Set management status.");
    managementStatusList_[managementStatusList_.index(managementStatus.id)] =
        managementStatus;
    unsavedChanges_ = true;
}

void Editor::setManagementStatusFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set management status filter.");
    managementStatusFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setManagementStatus(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::updateAfterSet()
{
    datasetsRange_ = datasets_.range();
    LOG_DEBUG(<< "Datasets range <" << toString(datasetsRange_) << ">.");

    elevationFilter_.set(static_cast<double>(datasetsRange_.elevationMin),
                         static_cast<double>(datasetsRange_.elevationMax));

    clipFilter_.boundary = datasets_.boundary();
    clipFilter_.box = clipFilter_.boundary;
    // clipFilter_.enabled = Region::Shape::BOX;
}

void Editor::updateAfterRead()
{
    LOG_DEBUG(<< "Start editor update after read.");

    updateAfterSet();

    LOG_DEBUG(<< "Use clip box filter region <" << clipFilter_ << ">.");
    LOG_DEBUG(<< "Use elevation filter range <" << elevationFilter_ << ">.");
    LOG_DEBUG(<< "Use descriptor filter range <" << descriptorFilter_ << ">.");
    LOG_DEBUG(<< "Use intensity filter range <" << intensityFilter_ << ">.");

    if (datasets_.size() > 0)
    {
        UnitsSettings unitsSettings = settings_.unitsSettings();
        unitsSettings.setLasFileScaling(datasets_.at(0).scalingFile());

        setUnitsSettings(unitsSettings);
    }

    for (size_t i = 0; i < segments_.size(); i++)
    {
        segmentsFilter_.setEnabled(segments_[i].id, true);
    }

    for (size_t i = 0; i < speciesList_.size(); i++)
    {
        speciesFilter_.setEnabled(speciesList_[i].id, true);
    }

    for (size_t i = 0; i < managementStatusList_.size(); i++)
    {
        managementStatusFilter_.setEnabled(managementStatusList_[i].id, true);
    }

    applyFilters();

    LOG_DEBUG(<< "Finished editor update after read.");
}

void Editor::applyFilters()
{
    if (viewports_.size() > 0)
    {
        viewports_.where().setDataset(datasetsFilter_);
        viewports_.where().setRegion(clipFilter_);
        viewports_.where().setElevation(elevationFilter_);
        viewports_.where().setDescriptor(descriptorFilter_);
        viewports_.where().setIntensity(intensityFilter_);
        viewports_.where().setClassification(classificationsFilter_);
        viewports_.where().setSegment(segmentsFilter_);
        viewports_.where().setSpecies(speciesFilter_);
        viewports_.where().setManagementStatus(managementStatusFilter_);

        viewports_.applyWhereToAll();
    }
}

void Editor::setRenderingSettings(const RenderingSettings &renderingSettings)
{
    settings_.setRenderingSettings(renderingSettings);
    writeSettings();
}

void Editor::setTreeSettings(const TreeSettings &treeSettings)
{
    settings_.setTreeSettings(treeSettings);
    writeSettings();
}

void Editor::setUnitsSettings(const UnitsSettings &unitsSettings)
{
    settings_.setUnitsSettings(unitsSettings);
    writeSettings();
}

void Editor::setViewSettings(const ViewSettings &viewSettings)
{
    settings_.setViewSettings(viewSettings);
    writeSettings();
}

void Editor::addModifier(ModifierInterface *modifier)
{
    modifiers_.push_back(modifier);
}

void Editor::runModifiers(Page *page)
{
    for (auto &it : modifiers_)
    {
        /** @todo Collect enabled modifiers during preprocessing. */
        if (it->modifierEnabled())
        {
            it->applyModifier(page);
        }
    }
}

void Editor::viewportsResize(size_t n)
{
    LOG_DEBUG(<< "Set number of viewports to <" << n << ">.");
    viewports_.resize(this, n);
    viewports_.applyWhereToAll();
}

std::shared_ptr<PageData> Editor::readPage(size_t dataset, size_t index)
{
    return pageManager_.readPage(this, dataset, index);
}

void Editor::erasePage(size_t dataset, size_t index)
{
    pageManager_.erasePage(this, dataset, index);
}
