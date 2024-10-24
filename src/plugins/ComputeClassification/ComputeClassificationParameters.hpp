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

/** @file ComputeClassificationParameters.hpp */

#ifndef COMPUTE_CLASSIFICATION_PARAMETERS_HPP
#define COMPUTE_CLASSIFICATION_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** Compute Classification Parameters. */
class ComputeClassificationParameters
{
public:
    double voxelRadius{0.1};
    double searchRadius{1.0};
    double angle{60.0};

    bool cleanGroundClassifications{true};
    bool cleanAllClassifications{false};
};

inline void toJson(Json &out, const ComputeClassificationParameters &in)
{
    toJson(out["voxelRadius"], in.voxelRadius);
    toJson(out["searchRadius"], in.searchRadius);
    toJson(out["angle"], in.angle);
    toJson(out["cleanGroundClassifications"], in.cleanGroundClassifications);
    toJson(out["cleanAllClassifications"], in.cleanAllClassifications);
}

inline std::string toString(const ComputeClassificationParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* COMPUTE_CLASSIFICATION_PARAMETERS_HPP */
