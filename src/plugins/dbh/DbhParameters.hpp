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

/** @file DbhParameters.hpp */

#ifndef DBH_PARAMETERS_HPP
#define DBH_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** DBH (Diameter at Breast Height) Parameters. */
class DbhParameters
{
public:
    double elevation{1.3};
    double elevationTolerance{0.05};
    size_t taubinFitIterationsMax{20}; // Usually, 4-6 iterations are enough.
    size_t geometricCircleIterationsMax{500};
    double geometricCircleFactorUp{1.0};
    double geometricCircleFactorDown{0.004};
    double geometricCircleParameterLimit{1e6};
};

inline void toJson(Json &out, const DbhParameters &in)
{
    toJson(out["elevation"], in.elevation);
    toJson(out["elevationTolerance"], in.elevationTolerance);
    toJson(out["taubinFitIterationsMax"], in.taubinFitIterationsMax);
    toJson(out["geometricCircleIterationsMax"],
           in.geometricCircleIterationsMax);
    toJson(out["geometricCircleFactorUp"], in.geometricCircleFactorUp);
    toJson(out["geometricCircleFactorDown"], in.geometricCircleFactorDown);
    toJson(out["geometricCircleParameterLimit"],
           in.geometricCircleParameterLimit);
}

inline std::string toString(const DbhParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* DBH_PARAMETERS_HPP */
