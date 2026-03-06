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

/** @file TreeAttributes.hpp */

#ifndef TREE_ATTRIBUTES_HPP
#define TREE_ATTRIBUTES_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Tree Attributes. */
class EXPORT_EDITOR TreeAttributes
{
public:
    /// Calculated tree position from X and Y coordinates in tree base range.
    Vector3<double> position;

    /// Calculated tree height.
    double height{0.0};

    // Crown center coordinates.
    Vector3<double> crownCenter;

    /// Calculated crown start.
    double crownStartHeight{0.0};

    /// Crown voxel counts per each meter.
    std::vector<size_t> crownVoxelCountPerMeters;

    /// Crown voxel count.
    size_t crownVoxelCount{0};

    /// Shared crown voxel count [other tree id : count].
    std::map<size_t, size_t> crownVoxelCountShared;

    /// Crown voxel size.
    double crownVoxelSize{0.0};

    /// Area of tree projection from top view.
    double surfaceAreaProjection{0.0};

    /// Tree surface area.
    double surfaceArea{0.0};

    /// Tree volume.
    double volume{0.0};

    /// The position of calculated DBH circle.
    Vector3<double> dbhPosition;

    /// The normal vector of calculated DBH circle.
    Vector3<double> dbhNormal;

    /// Calculated DBH (Diameter at Breast Height) value.
    double dbh{0.0};

    /// Validate position and height.
    bool isValid() const;

    /// Validate position.
    bool isPositionValid() const;

    /// Validate height.
    bool isHeightValid() const;

    /// Validate DBH.
    bool isDbhValid() const;

    /// Attributes.
    std::map<std::string, std::any> attributes;

    double number(const std::string &key, double defaultValue = 0.0) const;
};

void EXPORT_EDITOR fromJson(TreeAttributes &out, const Json &in, double scale);
void EXPORT_EDITOR toJson(Json &out, const TreeAttributes &in, double scale);

std::string EXPORT_EDITOR toString(const TreeAttributes &in);

#include <WarningsEnable.hpp>

#endif /* TREE_ATTRIBUTES_HPP */
