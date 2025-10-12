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

/** @file ComputeHullParameters.hpp */

#ifndef COMPUTE_HULL_PARAMETERS_HPP
#define COMPUTE_HULL_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** Compute Hull Parameters. */
class ComputeHullParameters
{
public:
    bool computeConvexHull{false};
    bool computeConvexHullProjection{false};
    bool computeConcaveHull{true};
    bool computeConcaveHullProjection{true};

    bool findOptimalAlpha{true};
    double alpha{0.8};

    double voxelRadius{0.5};
};

inline void toJson(Json &out, const ComputeHullParameters &in)
{
    toJson(out["computeConvexHull"], in.computeConvexHull);
    toJson(out["computeConvexHullProjection"], in.computeConvexHullProjection);
    toJson(out["computeConcaveHull"], in.computeConcaveHull);
    toJson(out["computeConcaveHullProjection"],
           in.computeConcaveHullProjection);
    toJson(out["findOptimalAlpha"], in.findOptimalAlpha);
    toJson(out["alpha"], in.alpha);
    toJson(out["voxelRadius"], in.voxelRadius);
}

inline std::string toString(const ComputeHullParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* COMPUTE_HULL_PARAMETERS_HPP */
