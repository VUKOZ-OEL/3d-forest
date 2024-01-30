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

/** @file ExportFileProperties.hpp */

#ifndef EXPORT_FILE_PROPERTIES_HPP
#define EXPORT_FILE_PROPERTIES_HPP

// Include 3D Forest.
#include <LasFile.hpp>

/** Export File Properties. */
class ExportFileProperties
{
public:
    ExportFileProperties();
    ~ExportFileProperties();

    void clear();

    void setFileName(const std::string &fileName) { fileName_ = fileName; }
    const std::string &fileName() const { return fileName_; }

    void setNumberOfPoints(uint64_t n) { numberOfPoints_ = n; }
    uint64_t numberOfPoints() const { return numberOfPoints_; }

    void setFormat(const LasFile::Format &format) { format_ = format; }
    const LasFile::Format &format() const { return format_; }

    void setRegion(const Box<double> &region) { region_ = region; }
    const Box<double> &region() const { return region_; }

    void setScale(const Vector3<double> &scale) { scale_ = scale; }
    void setScale(double scale) { scale_.set(scale, scale, scale); }
    const Vector3<double> &scale() const { return scale_; }

    void setOffset(const Vector3<double> &offset) { offset_ = offset; }
    const Vector3<double> &offset() const { return offset_; }

    void setFilterEnabled(bool b) { filterEnabled_ = b; }
    bool isFilterEnabled() const { return filterEnabled_; }

private:
    std::string fileName_;
    uint64_t numberOfPoints_;
    LasFile::Format format_;
    Box<double> region_;
    Vector3<double> scale_;
    Vector3<double> offset_;
    bool filterEnabled_;
};

#endif /* EXPORT_FILE_PROPERTIES_HPP */
