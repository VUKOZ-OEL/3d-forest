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

#include <Editor.hpp>
#include <IndexFileBuilder.hpp>
#include <Log.hpp>

static const char *EDITOR_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_KEY_DATA_SET = "datasets";
static const char *EDITOR_KEY_LAYER = "layers";
static const char *EDITOR_KEY_SETTINGS = "settings";
static const char *EDITOR_KEY_CLASSIFICATIONS = "classifications";
// static const char *EDITOR_KEY_CLIP_FILTER = "clipFilter";

Editor::Editor()
{
    close();
    viewportsResize(1);
}

Editor::~Editor()
{
}

void Editor::close()
{
    path_ = File::currentPath() + "\\untitled.json";
    projectName_ = "Untitled";

    datasets_.clear();
    layers_.setDefault();
    classifications_.clear();
    viewports_.clearContent();

    unsavedChanges_ = false;
}

void Editor::open(const std::string &path, const SettingsImport &settings)
{
    // Get filename extension in lower case (no UTF).
    std::string ext = File::fileExtension(path);
    for (auto &c : ext)
    {
        c = static_cast<char>(std::tolower(c));
    }

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
}

void Editor::openProject(const std::string &path)
{
    close();

    Json in;
    in.read(path);

    if (!in.isObject())
    {
        THROW("Project file '" + path + "' is not in JSON object");
    }

    try
    {
        path_ = path;

        // Project name
        if (in.contains(EDITOR_KEY_PROJECT_NAME))
        {
            projectName_ = in[EDITOR_KEY_PROJECT_NAME].string();
        }

        // Data sets
        if (in.contains(EDITOR_KEY_DATA_SET))
        {
            datasets_.read(in[EDITOR_KEY_DATA_SET], path_);
        }

        // Layers
        if (in.contains(EDITOR_KEY_LAYER))
        {
            layers_.read(in[EDITOR_KEY_LAYER]);
        }

        // Classifications
        if (in.contains(EDITOR_KEY_CLASSIFICATIONS))
        {
            classifications_.read(in[EDITOR_KEY_CLASSIFICATIONS]);
        }

        // Settings
        if (in.contains(EDITOR_KEY_SETTINGS))
        {
            settings_.read(in[EDITOR_KEY_SETTINGS]);
        }

        // Clip filter
        // if (in.contains(EDITOR_KEY_CLIP_FILTER))
        // {
        //     clipFilter_.read(in[EDITOR_KEY_CLIP_FILTER]);
        // }
        // else
        // {
        //     clipFilter_.clear();
        // }
    }
    catch (std::exception &e)
    {
        close();
        throw;
    }

    updateAfterRead();
}

void Editor::save(const std::string &path)
{
    Json out;

    // Project name
    out[EDITOR_KEY_PROJECT_NAME] = projectName_;

    // Data sets
    datasets_.write(out[EDITOR_KEY_DATA_SET]);

    // Layers
    layers_.write(out[EDITOR_KEY_LAYER]);

    // Classifications
    classifications_.write(out[EDITOR_KEY_CLASSIFICATIONS]);

    // Settings
    settings_.write(out[EDITOR_KEY_SETTINGS]);

    // Clip filter
    // clipFilter_.write(out[EDITOR_KEY_CLIP_FILTER]);

    out.write(path);

    unsavedChanges_ = false;
}

void Editor::openDataset(const std::string &path,
                         const SettingsImport &settings)
{
    try
    {
        datasets_.read(path, path_, settings, datasets_.boundary());
    }
    catch (std::exception &e)
    {
        throw;
    }

    updateAfterRead();

    unsavedChanges_ = true;
}

void Editor::setClassifications(const ClassificationList &classifications)
{
    classifications_ = classifications;

    if (classifications_.isEnabled())
    {
        viewports_.selectClassifications(classifications_.enabledList());
    }
    else
    {
        viewports_.selectClassifications({});
    }

    // viewClearRendered();
    unsavedChanges_ = true;
}

void Editor::setClipFilter(const Region &clipFilter)
{
    clipFilter_ = clipFilter;
    clipFilter_.boxView.setPercent(datasets_.boundary(),
                                   datasets_.boundary(),
                                   clipFilter_.box);

    viewports_.selectBox(clipBoundary());

    // unsavedChanges_ = true;
}

void Editor::resetClipFilter()
{
    clipFilter_.box = datasets_.boundary();
    setClipFilter(clipFilter_);
}

Box<double> Editor::clipBoundary() const
{
    if (clipFilter_.enabled)
    {
        return clipFilter_.box;
    }

    return datasets_.boundary();
}

void Editor::updateAfterRead()
{
    clipFilter_.box = datasets_.boundary();
    clipFilter_.boxView = clipFilter_.box;
}

void Editor::setDatasets(const DatasetList &datasets)
{
    datasets_ = datasets;
    unsavedChanges_ = true;
}

void Editor::setLayers(const LayerList &layers)
{
    layers_ = layers;

    viewports_.selectLayers(layers_.enabledList());

    unsavedChanges_ = true;
}

void Editor::setSettingsView(const SettingsView &settings)
{
    settings_.setView(settings);
    unsavedChanges_ = true;
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
            it->modifier(page);
        }
    }
}

void Editor::viewportsResize(size_t n)
{
    viewports_.resize(this, n);
}

void Editor::lock()
{
    mutex_.lock();
}

void Editor::unlock()
{
    mutex_.unlock();
}
