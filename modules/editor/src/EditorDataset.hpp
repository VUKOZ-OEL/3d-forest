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

/** @file EditorDataset.hpp */

#ifndef EDITOR_DATA_SET_HPP
#define EDITOR_DATA_SET_HPP

#include <Box.hpp>
#include <EditorSettingsImport.hpp>
#include <FileIndex.hpp>
#include <Json.hpp>

/** Editor Data Set. */
class EditorDataset
{
public:
    EditorDataset();

    size_t id() const { return id_; }

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool b);

    const std::string &label() const { return label_; }
    void setLabel(const std::string &label);

    const Vector3<float> &color() const { return color_; }
    void setColor(const Vector3<float> &color);

    const std::string &path() const { return path_; }
    const std::string &fileName() const { return fileName_; }
    const std::string &dateCreated() const { return dateCreated_; }

    const Vector3<double> &translation() const { return translation_; }
    void setTranslation(const Vector3<double> &translation);

    const Vector3<double> &scaling() const { return scaling_; }
    const Vector3<double> &scalingFile() const { return scalingFile_; }

    const Box<double> &boundary() const { return boundary_; }

    uint64_t nPoints() const { return nPoints_; }

    const FileIndex &index() const { return index_; }

    // I/O
    void read(size_t id,
              const std::string &path,
              const std::string &projectPath,
              const EditorSettingsImport &settings,
              const Box<double> &projectBoundary);
    void read(const Json &in, const std::string &projectPath);
    Json &write(Json &out) const;

protected:
    // Stored
    size_t id_;
    std::string label_; /**< Inconsistent with LAS in shared projects */
    bool enabled_;
    Vector3<float> color_;
    std::string pathUnresolved_;
    std::string dateCreated_; /**< Inconsistent with LAS in shared projects */
    Vector3<double> translation_;
    Vector3<double> scaling_;

    // Derived
    std::string path_;
    std::string fileName_;

    // Data
    uint64_t nPoints_;
    Vector3<double> translationFile_;
    Vector3<double> scalingFile_;
    Box<double> boundaryFile_;
    Box<double> boundary_;
    FileIndex index_;

    void setPath(const std::string &path, const std::string &projectPath);
    void read();
    void updateBoundary();
};

#endif /* EDITOR_DATA_SET_HPP */
