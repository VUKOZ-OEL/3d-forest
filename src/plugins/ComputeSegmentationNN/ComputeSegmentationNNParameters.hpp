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

/** @file ComputeSegmentationNNParameters.hpp */

#ifndef COMPUTE_SEGMENTATION_NN_PARAMETERS_HPP
#define COMPUTE_SEGMENTATION_NN_PARAMETERS_HPP

// Include 3D Forest.
#include <Json.hpp>

/** Compute Segmentation NN Parameters. */
class ComputeSegmentationNNParameters
{
public:
    /** Compute Segmentation NN Parameters Channel. */
    enum Channel
    {
        CHANNEL_DESCRIPTOR = 0,
        CHANNEL_INTENSITY
    };

    Channel leafToWoodChannel{CHANNEL_INTENSITY};

    double voxelRadius{0.1};
    double woodThresholdMin{25.0};
    double searchRadiusTrunkPoints{0.25};
    double searchRadiusLeafPoints{0.25};
    double treeBaseElevationMin{0.0};
    double treeBaseElevationMax{2.0};
    double treeHeightMin{1.0};

    bool zCoordinatesAsElevation{false};
    bool segmentOnlyTrunks{false};
};

inline void toJson(Json &out, const ComputeSegmentationNNParameters &in)
{
    if (in.leafToWoodChannel ==
        ComputeSegmentationNNParameters::CHANNEL_DESCRIPTOR)
    {
        toJson(out["leafToWoodChannel"], std::string("descriptor"));
    }
    else if (in.leafToWoodChannel ==
             ComputeSegmentationNNParameters::CHANNEL_INTENSITY)
    {
        toJson(out["leafToWoodChannel"], std::string("intensity"));
    }
    else
    {
        THROW("ComputeSegmentationNNParameters leafToWoodChannel not "
              "implemented.");
    }

    toJson(out["voxelRadius"], in.voxelRadius);
    toJson(out["woodThresholdMin"], in.woodThresholdMin);
    toJson(out["searchRadiusTrunkPoints"], in.searchRadiusTrunkPoints);
    toJson(out["searchRadiusLeafPoints"], in.searchRadiusLeafPoints);
    toJson(out["treeBaseElevationMin"], in.treeBaseElevationMin);
    toJson(out["treeBaseElevationMax"], in.treeBaseElevationMax);
    toJson(out["treeHeightMin"], in.treeHeightMin);
    toJson(out["zCoordinatesAsElevation"], in.zCoordinatesAsElevation);
    toJson(out["segmentOnlyTrunks"], in.segmentOnlyTrunks);
}

inline std::string toString(const ComputeSegmentationNNParameters &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* COMPUTE_SEGMENTATION_NN_PARAMETERS_HPP */
