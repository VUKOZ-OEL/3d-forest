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

/** @file Dataset.cpp */

#include <Dataset.hpp>
#include <Error.hpp>
#include <File.hpp>
#include <IndexFile.hpp>
#include <IndexFileBuilder.hpp>
#include <LasFile.hpp>
#include <Log.hpp>

Dataset::Dataset() : id_(0)
{
}

void Dataset::setLabel(const std::string &label)
{
    label_ = label;
}

void Dataset::setColor(const Vector3<double> &color)
{
    color_ = color;
}

void Dataset::setTranslation(const Vector3<double> &translation)
{
    translation_ = translation;
    updateBoundary();
}

void Dataset::read(size_t id,
                   const std::string &path,
                   const std::string &projectPath,
                   const SettingsImport &settings,
                   const Box<double> &projectBoundary)
{
    pathUnresolved_ = path;
    setPath(pathUnresolved_, projectPath);

    id_ = id;
    label_ = fileName_;
    color_.set(1.0, 1.0, 1.0);

    read();

    if (settings.isCenterEnabled())
    {
        Vector3<double> c1 = projectBoundary.getCenter();
        Vector3<double> c2 = boundaryFile_.getCenter();
        c1[2] = projectBoundary.min(2);
        c2[2] = boundaryFile_.min(2);
        translation_ = c1 - c2;
        updateBoundary();
    }
    else
    {
        Vector3<double> s = 1.0 / scalingFile_;
        translation_ = translationFile_ * s;
        updateBoundary();
    }
}

void Dataset::read(const Json &in, const std::string &projectPath)
{
    if (!in.isObject())
    {
        THROW("Data set is not JSON object");
    }

    // Data set path
    if (!in.containsString("path"))
    {
        THROW("Can't find string 'path' in JSON object");
    }

    pathUnresolved_ = in["path"].string();
    setPath(pathUnresolved_, projectPath);

    // Date Created
    if (in.contains("dateCreated"))
    {
        dateCreated_ = in["dateCreated"].string();
    }

    // ID
    id_ = in["id"].uint32();

    // Label
    if (in.contains("label"))
    {
        label_ = in["label"].string();
    }
    else
    {
        label_ = fileName_;
    }

    // Color
    if (in.contains("color"))
    {
        color_.read(in["color"]);
    }
    else
    {
        color_.set(1.0, 1.0, 1.0);
    }

    // Read
    read();

    // Transformation
    if (in.contains("translation"))
    {
        translation_.read(in["translation"]);
    }

    if (in.contains("scaling"))
    {
        scaling_.read(in["scaling"]);
    }

    updateBoundary();
}

Json &Dataset::write(Json &out) const
{
    out["id"] = id_;
    out["label"] = label_;
    color_.write(out["color"]);

    out["path"] = pathUnresolved_;
    out["dateCreated"] = dateCreated_;

    translation_.write(out["translation"]);
    scaling_.write(out["scaling"]);

    return out;
}

void Dataset::setPath(const std::string &path, const std::string &projectPath)
{
    // Data set absolute path
    path_ = File::resolvePath(path, projectPath);

    // Data set file name
    fileName_ = File::fileName(path_);
}

void Dataset::read()
{
    LasFile las;
    las.open(path_);
    las.readHeader();

    if (dateCreated_.empty())
    {
        dateCreated_ = las.header.dateCreated();
    }

    translationFile_.set(las.header.x_offset,
                         las.header.y_offset,
                         las.header.z_offset);

    translation_ = translationFile_;

    scalingFile_.set(las.header.x_scale_factor,
                     las.header.y_scale_factor,
                     las.header.z_scale_factor);

    scaling_.set(1.0, 1.0, 1.0);

    // Boundary
    const std::string pathIndex = IndexFileBuilder::extension(path_);
    index_.read(pathIndex);

    boundaryFile_ = index_.boundaryPoints();
    updateBoundary();

    nPoints_ = las.header.number_of_point_records;
}

void Dataset::updateBoundary()
{
    boundary_ = boundaryFile_;
    boundary_.translate(translation_);
    index_.translate(translation_);
}
