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

/** @file SegmentationL1Parameters.hpp */

#ifndef SEGMENTATION_L1_PARAMETERS_HPP
#define SEGMENTATION_L1_PARAMETERS_HPP

#include <cstdint>
#include <sstream>

/** Segmentation L1 Parameters. */
class SegmentationL1Parameters
{
public:
    int initialSamplesCount;
    int initialSamplesDensityMinimum;
    int initialSamplesDensityMaximum;
    int neighborhoodRadiusMinimum;
    int neighborhoodRadiusMaximum;

    SegmentationL1Parameters() { setDefault(); }

    void setDefault()
    {
        initialSamplesCount = 5;
        initialSamplesDensityMinimum = 0;
        initialSamplesDensityMaximum = 100;
        neighborhoodRadiusMinimum = 100;
        neighborhoodRadiusMaximum = 500;
    }

    void set(int initialSamplesCount_,
             int initialSamplesDensityMinimum_,
             int initialSamplesDensityMaximum_,
             int neighborhoodRadiusMinimum_,
             int neighborhoodRadiusMaximum_)
    {
        initialSamplesCount = initialSamplesCount_;
        initialSamplesDensityMinimum = initialSamplesDensityMinimum_;
        initialSamplesDensityMaximum = initialSamplesDensityMaximum_;
        neighborhoodRadiusMinimum = neighborhoodRadiusMinimum_;
        neighborhoodRadiusMaximum = neighborhoodRadiusMaximum_;
    }
};

inline std::ostream &operator<<(std::ostream &os,
                                const SegmentationL1Parameters &obj)
{
    return os << "initialSamplesCount <" << obj.initialSamplesCount
              << "> initialSamplesDensityMinimum <"
              << obj.initialSamplesDensityMinimum
              << "> initialSamplesDensityMaximum <"
              << obj.initialSamplesDensityMaximum
              << "> neighborhoodRadiusMinimum <"
              << obj.neighborhoodRadiusMinimum
              << "> neighborhoodRadiusMaximum <"
              << obj.neighborhoodRadiusMaximum << ">";
}

#endif /* SEGMENTATION_L1_PARAMETERS_HPP */
