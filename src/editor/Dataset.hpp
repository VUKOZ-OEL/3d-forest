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

/** @file Dataset.hpp */

#ifndef DATASET_HPP
#define DATASET_HPP

// Include 3D Forest.
#include <Box.hpp>
#include <ImportSettings.hpp>
#include <IndexFile.hpp>
#include <Json.hpp>
#include <LasFile.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Dataset. */
class EXPORT_EDITOR Dataset
{
public:
    /** Dataset Range. */
    class EXPORT_EDITOR Range
    {
    public:
        uint32_t elevationMin{UINT32_MAX};
        uint32_t elevationMax{0};

        void extend(const Dataset::Range &range);
    };

    Dataset();

    size_t id() const { return id_; }

    const std::string &label() const { return label_; }
    void setLabel(const std::string &label);

    const Vector3<double> &color() const { return color_; }
    void setColor(const Vector3<double> &color);

    const std::string &path() const { return path_; }
    const std::string &fileName() const { return fileName_; }
    const std::string &dateCreated() const { return dateCreated_; }

    const Vector3<double> &translation() const { return translation_; }
    void setTranslation(const Vector3<double> &translation);

    const Vector3<double> &scaling() const { return scaling_; }
    const Vector3<double> &scalingFile() const { return scalingFile_; }

    const Box<double> &boundary() const { return boundary_; }

    uint64_t nPoints() const { return nPoints_; }

    const IndexFile &index() const { return *index_; }

    const LasFile &las() const { return *las_; }
    LasFile &las() { return *las_; }

    const Dataset::Range &range() const { return range_; }

    // I/O.
    void read(size_t id,
              const std::string &path,
              const std::string &projectPath,
              const ImportSettings &settings,
              const Box<double> &projectBoundary);

protected:
    // Stored.
    size_t id_;
    std::string label_; /**< Inconsistent with LAS in shared projects. */
    Vector3<double> color_;
    std::string pathUnresolved_;
    std::string dateCreated_; /**< Inconsistent with LAS in shared projects. */
    Vector3<double> translation_;
    Vector3<double> scaling_;

    // Derived.
    std::string path_;
    std::string fileName_;

    // Data.
    uint64_t nPoints_;
    Vector3<double> translationFile_;
    Vector3<double> scalingFile_;
    Box<double> boundaryFile_;
    Box<double> boundary_;
    Dataset::Range range_;

    std::shared_ptr<IndexFile> index_;
    std::shared_ptr<LasFile> las_;

    void setPath(const std::string &path, const std::string &projectPath);
    void read();
    void updateBoundary();

    friend void fromJson(Dataset &out,
                         const Json &in,
                         const std::string &projectPath);
    friend void toJson(Json &out, const Dataset &in);
};

void fromJson(Dataset &out, const Json &in, const std::string &projectPath);
void toJson(Json &out, const Dataset &in);

void fromJson(Dataset::Range &out, const Json &in);
void toJson(Json &out, const Dataset::Range &in);

std::string toString(const Dataset::Range &in);

#include <WarningsEnable.hpp>

#endif /* DATASET_HPP */
