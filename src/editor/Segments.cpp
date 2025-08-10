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

/** @file Segments.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <Segments.hpp>

// Include local.
#define LOG_MODULE_NAME "Segments"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

Segments::Segments()
{
    setDefault();
}

void Segments::clear()
{
    LOG_DEBUG(<< "Clear.");
    segments_.clear();
    hashTableId_.clear();
}

void Segments::setDefault()
{
    LOG_DEBUG(<< "Set default.");
    size_t id = 0;
    size_t idx = 0;

    segments_.resize(1);
    segments_[idx].id = id;
    segments_[idx].label = "unsegmented";
    segments_[idx].color = {0.6, 0.6, 0.6};

    hashTableId_.clear();
    hashTableId_[id] = idx;
}

void Segments::push_back(const Segment &segment)
{
    LOG_DEBUG(<< "Append segment <" << segment << ">.");
    size_t id = segment.id;
    size_t idx = segments_.size();

    segments_.push_back(segment);

    hashTableId_[id] = idx;
}

void Segments::erase(size_t pos)
{
    LOG_DEBUG(<< "Erase item <" << pos << ">.");

    if (segments_.size() == 0)
    {
        return;
    }

    size_t key = id(pos);
    hashTableId_.erase(key);

    size_t n = segments_.size() - 1;
    for (size_t i = pos; i < n; i++)
    {
        segments_[i] = segments_[i + 1];
        hashTableId_[segments_[i].id] = i;
    }
    segments_.resize(n);
}

size_t Segments::index(size_t id, bool throwException) const
{
    const auto &it = hashTableId_.find(id);
    if (it != hashTableId_.end())
    {
        return it->second;
    }

    if (throwException)
    {
        THROW("Invalid segment id");
    }
    else
    {
        return SIZE_MAX;
    }
}

bool Segments::contains(size_t id) const
{
    const auto &it = hashTableId_.find(id);
    if (it != hashTableId_.end())
    {
        return true;
    }
    return false;
}

size_t Segments::unusedId() const
{
    LOG_DEBUG(<< "Obtain unused id.");
    // Return minimum available id value.
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (hashTableId_.find(rval) == hashTableId_.end())
        {
            return rval;
        }
    }

    THROW("New segment identifier is not available.");
}

void Segments::addTree(size_t id, const Box<double> &boundary)
{
    Segment segment;

    segment.id = id;
    segment.label = "Tree " + std::to_string(segment.id);
    segment.color =
        ColorPalette::MPN65[segment.id % ColorPalette::MPN65.size()];
    segment.boundary = boundary;

    push_back(segment);
}

void Segments::exportMeshList(const std::string &projectFilePath,
                              double scale) const
{
    for (auto const &it : segments_)
    {
        std::string extId = "." + toString(it.id) + ".ply";
        std::string pathId = File::replaceExtension(projectFilePath, extId);

        for (const auto &m : it.meshList)
        {
            std::string extMesh = "." + m.first + ".ply";
            std::string pathMesh = File::replaceExtension(pathId, extMesh);
            m.second.exportPLY(pathMesh, scale);
        }
    }
}

void Segments::importMeshList(const std::string &projectFilePath, double scale)
{
    std::filesystem::path path = projectFilePath;
    std::filesystem::path dir = path.parent_path();
    std::string projectFileName = path.filename().string();
    std::string projectName = File::replaceExtension(projectFileName, "");

    LOG_DEBUG(<< "Project file path <" << projectFilePath << ">.");
    LOG_DEBUG(<< "Project name <" << projectName << ">.");

    // List file names in project directory.
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        // Skip directories etc.
        if (!entry.is_regular_file())
        {
            continue;
        }

        // The file name must start with the project name.
        // For example look for "forest.1.mesh.ply" for "forest.json" project.
        std::string fileName = entry.path().filename().string();
        std::string fileNamePath = entry.path().string();
        LOG_DEBUG(<< "File name <" << fileName << "> path <" << fileNamePath
                  << ">.");
        if (fileName.rfind(projectName + ".", 0) != 0)
        {
            continue;
        }

        std::vector<std::string> tokens = split(fileName, '.');

        // Need at least 4 tokens to have "<project>.<number>.<name>.ply".
        size_t n = tokens.size();
        if (n < 4)
        {
            continue;
        }

        if (tokens[n - 1] != "ply")
        {
            continue;
        }

        // Read segment ID.
        size_t id;
        try
        {
            id = toSize(tokens[n - 3]);
        }
        catch (...)
        {
            LOG_ERROR(<< "Unexpected segment ID <" << tokens[n - 3]
                      << "> format in mesh file name <" << fileNamePath
                      << ">.");
        }

        // Read mesh name.
        std::string meshName = tokens[n - 2];

        // Import the mesh.
        LOG_DEBUG(<< "File name <" << fileName << "> id <" << id << "> mesh <"
                  << meshName << ">.");

        size_t i;
        try
        {
            i = index(id);
        }
        catch (...)
        {
            LOG_ERROR(<< "Unexpected segment ID <" << id
                      << "> in mesh file name <" << fileNamePath
                      << "> not found in segments.");
        }

        Segment &segment = segments_[i];

        Mesh m;
        m.name = meshName;
        m.importPLY(fileNamePath, scale);
        segment.meshList[m.name] = std::move(m);
    }
}

void fromJson(Segments &out, const Json &in, double scale)
{
    out.clear();

    size_t i = 0;
    size_t n = in.array().size();

    out.segments_.resize(n);

    for (auto const &it : in.array())
    {
        fromJson(out.segments_[i], it, scale);
        size_t id = out.segments_[i].id;
        out.hashTableId_[id] = i;
        i++;
    }

    // Set default.
    if (out.segments_.size() == 0)
    {
        out.setDefault();
    }
}

void toJson(Json &out, const Segments &in, double scale)
{
    size_t i = 0;

    for (auto const &it : in.segments_)
    {
        toJson(out[i], it, scale);
        i++;
    }
}
