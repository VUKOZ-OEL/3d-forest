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
    LOG_DEBUG(<< "Read dataset from path <" << path << "> project path <"
              << projectPath << ".");

    pathUnresolved_ = path;
    setPath(pathUnresolved_, projectPath);

    id_ = id;
    label_ = fileName_;
    color_.set(1.0, 1.0, 1.0);

    read();

    if (settings.translateToOrigin)
    {
        Vector3<double> c1 = projectBoundary.center();
        Vector3<double> c2 = boundaryFile_.center();
        c1[2] = projectBoundary.min(2);
        c2[2] = boundaryFile_.min(2);
        translation_ = c1 - c2;
        LOG_DEBUG(<< "Translation to the origin <" << translation_ << ">.");
    }

    updateBoundary();
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
    las_->range(1, range_.elevationMin, range_.elevationMax);

    if (dateCreated_.empty())
    {
        dateCreated_ = las_->header.dateCreated();
    }

    translationFile_.set(las_->header.x_offset,
                         las_->header.y_offset,
                         las_->header.z_offset);

    translation_ = translationFile_;

    LOG_DEBUG(<< "File translation <" << translationFile_ << ">.");
    LOG_DEBUG(<< "Translation <" << translation_ << ">.");

    scalingFile_.set(las_->header.x_scale_factor,
                     las_->header.y_scale_factor,
                     las_->header.z_scale_factor);

    LOG_DEBUG(<< "File scaling <" << scalingFile_ << ">.");

    scaling_.set(1.0, 1.0, 1.0);

    LOG_DEBUG(<< "Scaling <" << scaling_ << ">.");

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

    LOG_DEBUG(<< "File boundary <" << boundaryFile_ << ">.");
    LOG_DEBUG(<< "Boundary <" << boundary_ << ">.");
}

void Dataset::Range::extend(const Dataset::Range &range)
{
    if (range.elevationMin < elevationMin)
    {
        elevationMin = range.elevationMin;
    }

    if (range.elevationMax > elevationMax)
    {
        elevationMax = range.elevationMax;
    }
}

void fromJson(Dataset &out, const Json &in, const std::string &projectPath)
{
    LOG_DEBUG(<< "Open from json. Project path <" << projectPath << ">.");

    if (!in.typeObject())
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
        fromJson(out.translationFile_, in["translation"]);
        out.translation_ = out.translationFile_;
    }

    if (in.contains("scaling"))
    {
        fromJson(out.scalingFile_, in["scaling"]);
    }

    out.updateBoundary();
}

void toJson(Json &out, const Dataset &in)
{
    toJson(out["id"], in.id_);
    toJson(out["label"], in.label_);
    toJson(out["color"], in.color_);
    toJson(out["path"], in.pathUnresolved_);
    toJson(out["dateCreated"], in.dateCreated_);
    toJson(out["translation"], in.translationFile_);
    toJson(out["scaling"], in.scalingFile_);
}

void fromJson(Dataset::Range &out, const Json &in)
{
    fromJson(out.elevationMin, in["elevationMin"]);
    fromJson(out.elevationMax, in["elevationMax"]);
}

void toJson(Json &out, const Dataset::Range &in)
{
    toJson(out["elevationMin"], in.elevationMin);
    toJson(out["elevationMax"], in.elevationMax);
}

std::string toString(const Dataset::Range &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}
