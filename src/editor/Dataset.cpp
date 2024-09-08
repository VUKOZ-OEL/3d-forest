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

// Include 3D Forest.
#include <Dataset.hpp>
#include <Error.hpp>
#include <File.hpp>
#include <IndexFileBuilder.hpp>

// Include local.
#define LOG_MODULE_NAME "Dataset"
#define LOG_MODULE_DEBUG_ENABLED 1
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

    if (settings.centerPointsOnScreen)
    {
        Vector3<double> c1 = projectBoundary.getCenter();
        Vector3<double> c2 = boundaryFile_.getCenter();
        c1[2] = projectBoundary.min(2);
        c2[2] = boundaryFile_.min(2);
        translation_ = c1 - c2;
        LOG_DEBUG(<< "Centered translation <" << translation_ << ">.");
        updateBoundary();
    }
    else
    {
        Vector3<double> s = 1.0 / scalingFile_;
        translation_ = translationFile_ * s;
        LOG_DEBUG(<< "Scaled translation <" << translation_ << ">.");
        updateBoundary();
    }
}

void fromJson(Dataset &out, const Json &in, const std::string &projectPath)
{
    LOG_DEBUG(<< "Open from json. Project path <" << projectPath << ">.");

    if (!in.isObject())
    {
        THROW("Data set is not JSON object");
    }

    // Data set path.
    fromJson(out.pathUnresolved_, in["path"]);
    out.setPath(out.pathUnresolved_, projectPath);

    // Date Created.
    if (in.contains("dateCreated"))
    {
        fromJson(out.dateCreated_, in["dateCreated"]);
    }

    // ID.
    fromJson(out.id_, in["id"]);

    // Label.
    if (in.contains("label"))
    {
        fromJson(out.label_, in["label"]);
    }
    else
    {
        out.label_ = out.fileName_;
    }

    // Color.
    if (in.contains("color"))
    {
        fromJson(out.color_, in["color"]);
    }
    else
    {
        out.color_.set(1.0, 1.0, 1.0);
    }

    // Read.
    out.read();

    // Transformation.
    if (in.contains("translation"))
    {
        fromJson(out.translation_, in["translation"]);
    }

    // if (in.contains("scaling"))
    // {
    //     fromJson(out.scaling_, in["scaling"]);
    // }

    out.updateBoundary();
}

void toJson(Json &out, const Dataset &in)
{
    toJson(out["id"], in.id_);
    toJson(out["label"], in.label_);
    toJson(out["color"], in.color_);
    toJson(out["path"], in.pathUnresolved_);
    toJson(out["dateCreated"], in.dateCreated_);
    toJson(out["translation"], in.translation_);
    toJson(out["scaling"], in.scaling_);
}

void Dataset::setPath(const std::string &path, const std::string &projectPath)
{
    // Data set absolute path.
    path_ = File::resolvePath(path, projectPath);

    // Data set file name.
    fileName_ = File::fileName(path_);
}

void Dataset::read()
{
    LOG_INFO(<< "Read dataset <" << path_ << ">.");

    las_ = std::make_shared<LasFile>();
    las_->open(path_);
    las_->readHeader();

    if (dateCreated_.empty())
    {
        dateCreated_ = las_->header.dateCreated();
    }

    translationFile_.set(las_->header.x_offset,
                         las_->header.y_offset,
                         las_->header.z_offset);

    translation_ = translationFile_;

    LOG_DEBUG(<< "Translation <" << translation_ << ">.");

    scalingFile_.set(las_->header.x_scale_factor,
                     las_->header.y_scale_factor,
                     las_->header.z_scale_factor);

    scaling_.set(1.0, 1.0, 1.0);

    // Boundary.
    const std::string pathIndex = IndexFileBuilder::extension(path_);
    index_ = std::make_shared<IndexFile>();
    index_->read(pathIndex);

    boundaryFile_ = index_->boundaryPoints();
    updateBoundary();

    nPoints_ = las_->header.number_of_point_records;

    LOG_DEBUG(<< "Number of points <" << nPoints_ << ">.");
}

void Dataset::updateBoundary()
{
    boundary_ = boundaryFile_;
    boundary_.translate(translation_);
    index_->translate(translation_);

    LOG_DEBUG(<< "Boundary <" << boundary_ << ">.");
}
