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

/** @file DescriptorParameters.hpp */

#ifndef DESCRIPTOR_PARAMETERS_HPP
#define DESCRIPTOR_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** Segmentation Parameters. */
class DescriptorParameters
{
public:
    enum Method
    {
        METHOD_DENSITY = 0,
        METHOD_PCA_INTENSITY
    };

    Method method{METHOD_DENSITY};

    double voxelRadius{0.1};
    double searchRadius{0.5};

    bool includeGroundPoints{false};
};

inline void toJson(Json &out, const DescriptorParameters &in)
{
    if (in.method == DescriptorParameters::METHOD_DENSITY)
    {
        toJson(out["method"], std::string("density"));
    }
    else if (in.method == DescriptorParameters::METHOD_PCA_INTENSITY)
    {
        toJson(out["method"], std::string("pca"));
    }
    else
    {
        THROW("DescriptorParameters method not implemented.");
    }

    toJson(out["voxelRadius"], in.voxelRadius);
    toJson(out["searchRadius"], in.searchRadius);
    toJson(out["includeGroundPoints"], in.includeGroundPoints);
}

inline std::string toString(const DescriptorParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* DESCRIPTOR_PARAMETERS_HPP */
