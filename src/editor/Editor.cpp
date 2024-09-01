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

static const char *EDITOR_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_KEY_DATA_SET = "datasets";
static const char *EDITOR_KEY_SEGMENT = "segments";
static const char *EDITOR_KEY_SETTINGS = "settings";
static const char *EDITOR_KEY_CLASSIFICATIONS = "classifications";
// static const char *EDITOR_KEY_CLIP_FILTER = "clipFilter";
static const char *EDITOR_KEY_ELEVATION_RANGE = "elevationRange";

Editor::Editor()
{
    LOG_DEBUG(<< "Start creating the editor.");
    close();
    viewportsResize(1);
    LOG_DEBUG(<< "Finished creating the editor.");
}

Editor::~Editor()
{
    LOG_DEBUG(<< "Destroy.");
}

void Editor::close()
{
    LOG_DEBUG(<< "Start closing the editor.");

    setProjectPath(File::join(File::currentPath(), "untitled.json"));
    projectName_ = "Untitled";

    datasets_.clear();
    datasetsFilter_.clear();
    datasetsFilter_.setFilterEnabled(true);

    segments_.setDefault();
    segmentsFilter_.clear();
    segmentsFilter_.setFilter(0, true);
    segmentsFilter_.setFilterEnabled(true);

    classifications_.clear();
    classificationsFilter_.clear();
    for (size_t i = 0; i < classifications_.size(); i++)
    {
        classificationsFilter_.setFilter(i, true);
    }
    classificationsFilter_.setFilterEnabled(true);

    viewports_.clearContent();

    clipFilter_.clear();
    elevationFilter_.clear();
    descriptorFilter_.set(0.0, 1.0);
    intensityFilter_.set(0.0, 1.0);

    unsavedChanges_ = false;

    LOG_DEBUG(<< "Finished closing the editor.");
}

void Editor::open(const std::string &path, const SettingsImport &settings)
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

    Json in;
    in.read(path);

    if (!in.isObject())
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

        // Classifications.
        if (in.contains(EDITOR_KEY_CLASSIFICATIONS))
        {
            fromJson(classifications_, in[EDITOR_KEY_CLASSIFICATIONS]);
        }

        // Settings.
        if (in.contains(EDITOR_KEY_SETTINGS))
        {
            fromJson(settings_, in[EDITOR_KEY_SETTINGS]);
        }

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
        if (in.contains(EDITOR_KEY_ELEVATION_RANGE))
        {
            fromJson(elevationFilter_, in[EDITOR_KEY_ELEVATION_RANGE]);
        }
    }
    catch (...)
    {
        LOG_DEBUG(<< "Cancel opening new project, exception is raised.");
        close();
        throw;
    }

    updateAfterRead();

    LOG_DEBUG(<< "Finished opening project.");
}

void Editor::saveProject(const std::string &path)
{
    LOG_DEBUG(<< "Start saving the project to path <" << path << ">.");

    Json out;

    toJson(out[EDITOR_KEY_PROJECT_NAME], projectName_);
    toJson(out[EDITOR_KEY_DATA_SET], datasets_);
    toJson(out[EDITOR_KEY_SEGMENT], segments_);
    toJson(out[EDITOR_KEY_CLASSIFICATIONS], classifications_);
    toJson(out[EDITOR_KEY_SETTINGS], settings_);
    // toJson(out[EDITOR_KEY_CLIP_FILTER], clipFilter_);
    toJson(out[EDITOR_KEY_ELEVATION_RANGE], elevationFilter_);

    out.write(path);

    unsavedChanges_ = false;

    LOG_DEBUG(<< "Finished saving the project.");
}

void Editor::openDataset(const std::string &path,
                         const SettingsImport &settings)
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

    LOG_DEBUG(<< "Finished opening the dataset.");
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
    LOG_DEBUG(<< "Set region filter <" << clipFilter << ">.");
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
    LOG_DEBUG(<< "Reset region filter.");
    clipFilter_.box = clipFilter_.boundary;
    setClipFilter(clipFilter_);
}

Box<double> Editor::clipBoundary() const
{
    if (clipFilter_.shape == Region::SHAPE_BOX)
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
    datasets_ = datasets;
    unsavedChanges_ = true;
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

void Editor::updateAfterRead()
{
    LOG_DEBUG(<< "Update after read.");

    clipFilter_.boundary = datasets_.boundary();
    clipFilter_.box = clipFilter_.boundary;
    // clipFilter_.enabled = Region::SHAPE_BOX;

    if (datasets_.size() > 0)
    {
        SettingsUnits settingsUnits = settings_.units;
        settingsUnits.setLasFileScaling(datasets_.at(0).scalingFile());

        setSettingsUnits(settingsUnits);
    }

    for (size_t i = 0; i < segments_.size(); i++)
    {
        segmentsFilter_.setFilter(segments_[i].id, true);
    }

    if (viewports_.size() > 0)
    {
        viewports_.where().setDataset(datasetsFilter_);
        viewports_.where().setRegion(clipFilter_);
        viewports_.where().setElevation(elevationFilter_);
        viewports_.where().setDescriptor(descriptorFilter_);
        viewports_.where().setIntensity(intensityFilter_);
        viewports_.where().setClassification(classificationsFilter_);
        viewports_.where().setSegment(segmentsFilter_);

        viewports_.applyWhereToAll();
    }
}

void Editor::setSettingsView(const SettingsView &settingsView)
{
    settings_.view = settingsView;
    unsavedChanges_ = true;
}

void Editor::setSettingsUnits(const SettingsUnits &settingsUnits)
{
    unsavedChanges_ = settings_.units.apply(settingsUnits);
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
        if (it->isModifierEnabled())
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
    return pageManager_.get(this, dataset, index);
}

void Editor::erasePage(size_t dataset, size_t index)
{
    pageManager_.erase(this, dataset, index);
}

void Editor::lock(const std::string &message)
{
    mutex_.lock();
    (void)message;
}

void Editor::unlock(const std::string &message)
{
    mutex_.unlock();
    (void)message;
}
