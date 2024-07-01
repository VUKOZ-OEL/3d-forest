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

/** @file SegmentationParameters.hpp */

#ifndef SEGMENTATION_PARAMETERS_HPP
#define SEGMENTATION_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** Segmentation Parameters. */
class SegmentationParameters
{
public:
    double voxelSize{0.1};
    double descriptor{25.0};
    double trunkRadius{0.25};
    double leafRadius{0.25};
    double elevationMin{1.0};
    double elevationMax{2.0};
    double treeHeight{1.0};

    bool zCoordinatesAsElevation{false};
    bool segmentOnlyTrunks{false};
};

inline void toJson(Json &out, const SegmentationParameters &in)
{
    toJson(out["voxelSize"], in.voxelSize);
    toJson(out["descriptor"], in.descriptor);
    toJson(out["trunkRadius"], in.trunkRadius);
    toJson(out["leafRadius"], in.leafRadius);
    toJson(out["elevationMin"], in.elevationMin);
    toJson(out["elevationMax"], in.elevationMax);
    toJson(out["treeHeight"], in.treeHeight);
    toJson(out["zCoordinatesAsElevation"], in.zCoordinatesAsElevation);
    toJson(out["segmentOnlyTrunks"], in.segmentOnlyTrunks);
}

inline std::string toString(const SegmentationParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* SEGMENTATION_PARAMETERS_HPP */
