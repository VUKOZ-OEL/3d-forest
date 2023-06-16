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

#include <QueryWhere.hpp>

/** Segmentation L1 Parameters. */
class SegmentationL1Parameters
{
public:
    QueryWhere sampleFilter;

    int sampleDescriptorMinimum;
    int sampleDescriptorMaximum;
    int numberOfSamples;
    int neighborhoodRadiusPca;
    int neighborhoodRadiusMinimum;
    int neighborhoodRadiusMaximum;
    int numberOfIterations;

    SegmentationL1Parameters();

    void clear();

    void setDefault();

    void set(int sampleDescriptorMinimum_,
             int sampleDescriptorMaximum_,
             int numberOfSamples_,
             int neighborhoodRadiusPca_,
             int neighborhoodRadiusMinimum_,
             int neighborhoodRadiusMaximum_,
             int numberOfIterations_);
};

inline std::ostream &operator<<(std::ostream &os,
                                const SegmentationL1Parameters &obj)
{
    // clang-format off
    return os << "sampleDescriptorMinimum <"
              << obj.sampleDescriptorMinimum
              << "> sampleDescriptorMaximum <"
              << obj.sampleDescriptorMaximum
              << "> numberOfSamples <"
              << obj.numberOfSamples
              << "> neighborhoodRadiusPca <"
              << obj.neighborhoodRadiusPca
              << "> neighborhoodRadiusMinimum <"
              << obj.neighborhoodRadiusMinimum
              << "> neighborhoodRadiusMaximum <"
              << obj.neighborhoodRadiusMaximum
              << "> numberOfIterations <"
              << obj.numberOfIterations
              << ">";
    // clang-format on
}

#endif /* SEGMENTATION_L1_PARAMETERS_HPP */
