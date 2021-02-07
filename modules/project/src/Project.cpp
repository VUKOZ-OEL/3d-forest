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
    @file Project.cpp
*/

#include <Error.hpp>
#include <File.hpp>
#include <Project.hpp>

static const char *PROJECT_KEY_PROJECT_NAME = "projectName";
static const char *PROJECT_KEY_DATA_SET = "dataSets";
static const char *PROJECT_KEY_LAYER = "layers";
static const char *PROJECT_KEY_CLIP_FILTER = "clipFilter";

Project::Project()
{
}

Project::~Project()
{
}

void Project::read(const std::string &path)
{
    clear();

    Json in;
    in.read(path);

    if (!in.isObject())
    {
        THROW("Project file '" + path + "' is not in JSON object");
    }

    try
    {
        size_t i;
        size_t n;

        path_ = path;

        // Project name
        if (in.contains(PROJECT_KEY_PROJECT_NAME))
        {
            projectName_ = in[PROJECT_KEY_PROJECT_NAME].string();
        }
        else
        {
            projectName_ = "Untitled";
        }

        // Data sets
        if (in.contains(PROJECT_KEY_DATA_SET))
        {
            i = 0;
            n = in[PROJECT_KEY_DATA_SET].array().size();
            dataSets_.resize(n);

            for (auto const &it : in[PROJECT_KEY_DATA_SET].array())
            {
                dataSets_[i] = std::make_shared<ProjectDataSet>();
                dataSets_[i]->read(it, path_);
                i++;
            }
        }

        // Layers
        if (in.contains(PROJECT_KEY_LAYER))
        {
            i = 0;
            n = in[PROJECT_KEY_LAYER].array().size();
            layers_.resize(n);

            for (auto const &it : in[PROJECT_KEY_LAYER].array())
            {
                layers_[i].read(it);
                i++;
            }
        }

        // Clip filter
        if (in.contains(PROJECT_KEY_CLIP_FILTER))
        {
            clipFilter_.read(in[PROJECT_KEY_CLIP_FILTER]);
        }
        else
        {
            clipFilter_.clear();
        }
    }
    catch (std::exception &e)
    {
        clear();
        throw;
    }
}

void Project::write(const std::string &path)
{
    Json out;
    size_t i;

    // Project name
    out[PROJECT_KEY_PROJECT_NAME] = projectName_;

    // Data sets
    i = 0;
    for (auto const &it : dataSets_)
    {
        it->write(out[PROJECT_KEY_DATA_SET][i]);
        i++;
    }

    // Layers
    i = 0;
    for (auto const &it : layers_)
    {
        it.write(out[PROJECT_KEY_LAYER][i]);
        i++;
    }

    // Clip filter
    clipFilter_.write(out[PROJECT_KEY_CLIP_FILTER]);

    out.write(path);
}

void Project::clear()
{
    path_ = "";
    projectName_ = "";
    dataSets_.clear();
    layers_.clear();
    clipFilter_.clear();
}

void Project::setVisibleDataSet(size_t i, bool visible)
{
    dataSets_[i]->visible = visible;
}

void Project::setVisibleLayer(size_t i, bool visible)
{
    layers_[i].visible = visible;
}

void Project::setClipFilter(const ClipFilter &clipFilter)
{
    clipFilter_ = clipFilter;
}
