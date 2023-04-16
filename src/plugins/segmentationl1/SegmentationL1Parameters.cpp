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

#include <SegmentationL1Parameters.hpp>

#define LOG_MODULE_NAME "SegmentationL1Parameters"
#include <Log.hpp>

SegmentationL1Parameters::SegmentationL1Parameters()
{
    setDefault();
}

void SegmentationL1Parameters::clear()
{
    sampleDescriptorMinimum = 0;
    sampleDescriptorMaximum = 0;
    numberOfSamples = 0;
    neighborhoodRadiusPca = 0;
    neighborhoodRadiusMinimum = 0;
    neighborhoodRadiusMaximum = 0;
    numberOfIterations = 0;
}

void SegmentationL1Parameters::setDefault()
{
    sampleDescriptorMinimum = 0;
    sampleDescriptorMaximum = 100;
    numberOfSamples = 20;
    neighborhoodRadiusPca = 100;
    neighborhoodRadiusMinimum = 100;
    neighborhoodRadiusMaximum = 500;
    numberOfIterations = 1;
}

void SegmentationL1Parameters::set(int sampleDescriptorMinimum_,
                                   int sampleDescriptorMaximum_,
                                   int numberOfSamples_,
                                   int neighborhoodRadiusPca_,
                                   int neighborhoodRadiusMinimum_,
                                   int neighborhoodRadiusMaximum_,
                                   int numberOfIterations_)
{
    sampleDescriptorMinimum = sampleDescriptorMinimum_;
    sampleDescriptorMaximum = sampleDescriptorMaximum_;
    numberOfSamples = numberOfSamples_;
    neighborhoodRadiusPca = neighborhoodRadiusPca_;
    neighborhoodRadiusMinimum = neighborhoodRadiusMinimum_;
    neighborhoodRadiusMaximum = neighborhoodRadiusMaximum_;
    numberOfIterations = numberOfIterations_;
}
