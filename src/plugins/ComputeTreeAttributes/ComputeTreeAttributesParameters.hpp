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

/** @file ComputeTreeAttributesParameters.hpp */

#ifndef COMPUTE_TREE_ATTRIBUTES_PARAMETERS_HPP
#define COMPUTE_TREE_ATTRIBUTES_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** Compute Tree Attributes Parameters. */
class ComputeTreeAttributesParameters
{
public:
    // Tree position parameters.
    double treePositionHeightRange{0.6};
    double treeTipHeightRange{0.1};

    // DBH parameters.
    double dbhElevation{1.3};
    double dbhElevationRange{0.05};
    double maximumValidCalculatedDbh{2.0};

    // Least Squared Regression parameters.
    size_t taubinFitIterationsMax{20}; // Usually, 4-6 iterations are enough.
    size_t geometricCircleIterationsMax{500};
    double geometricCircleFactorUp{1.0};
    double geometricCircleFactorDown{0.004};
    double geometricCircleParameterLimit{1e6};
};

inline void toJson(Json &out, const ComputeTreeAttributesParameters &in)
{
    toJson(out["treePositionHeightRange"], in.treePositionHeightRange);
    toJson(out["dbhElevation"], in.dbhElevation);
    toJson(out["dbhElevationRange"], in.dbhElevationRange);
    toJson(out["maximumValidCalculatedDbh"], in.maximumValidCalculatedDbh);
    toJson(out["taubinFitIterationsMax"], in.taubinFitIterationsMax);
    toJson(out["geometricCircleIterationsMax"],
           in.geometricCircleIterationsMax);
    toJson(out["geometricCircleFactorUp"], in.geometricCircleFactorUp);
    toJson(out["geometricCircleFactorDown"], in.geometricCircleFactorDown);
    toJson(out["geometricCircleParameterLimit"],
           in.geometricCircleParameterLimit);
}

inline std::string toString(const ComputeTreeAttributesParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* COMPUTE_TREE_ATTRIBUTES_PARAMETERS_HPP */
