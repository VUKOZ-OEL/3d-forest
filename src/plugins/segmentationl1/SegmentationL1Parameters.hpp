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
    int numberOfSamples;
    int initialSamplesDensityMinimum;
    int initialSamplesDensityMaximum;
    int neighborhoodRadiusMinimum;
    int neighborhoodRadiusMaximum;
    int numberOfIterations;

    SegmentationL1Parameters() { setDefault(); }

    void clear()
    {
        numberOfSamples = 0;
        initialSamplesDensityMinimum = 0;
        initialSamplesDensityMaximum = 0;
        neighborhoodRadiusMinimum = 0;
        neighborhoodRadiusMaximum = 0;
        numberOfIterations = 0;
    }

    void setDefault()
    {
        numberOfSamples = 5;
        initialSamplesDensityMinimum = 0;
        initialSamplesDensityMaximum = 100;
        neighborhoodRadiusMinimum = 100;
        neighborhoodRadiusMaximum = 500;
        numberOfIterations = 1;
    }

    void set(int numberOfSamples_,
             int initialSamplesDensityMinimum_,
             int initialSamplesDensityMaximum_,
             int neighborhoodRadiusMinimum_,
             int neighborhoodRadiusMaximum_,
             int numberOfIterations_)
    {
        numberOfSamples = numberOfSamples_;
        initialSamplesDensityMinimum = initialSamplesDensityMinimum_;
        initialSamplesDensityMaximum = initialSamplesDensityMaximum_;
        neighborhoodRadiusMinimum = neighborhoodRadiusMinimum_;
        neighborhoodRadiusMaximum = neighborhoodRadiusMaximum_;
        numberOfIterations = numberOfIterations_;
    }
};

inline std::ostream &operator<<(std::ostream &os,
                                const SegmentationL1Parameters &obj)
{
    return os << "numberOfSamples <" << obj.numberOfSamples
              << "> initialSamplesDensityMinimum <"
              << obj.initialSamplesDensityMinimum
              << "> initialSamplesDensityMaximum <"
              << obj.initialSamplesDensityMaximum
              << "> neighborhoodRadiusMinimum <"
              << obj.neighborhoodRadiusMinimum
              << "> neighborhoodRadiusMaximum <"
              << obj.neighborhoodRadiusMaximum << "> numberOfIterations <"
              << obj.numberOfIterations << ">";
}

#endif /* SEGMENTATION_L1_PARAMETERS_HPP */
