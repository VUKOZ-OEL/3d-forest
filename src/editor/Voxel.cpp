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

/** @file Voxel.cpp */

#include <cstring>

#include <Log.hpp>
#include <Voxel.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("Voxel", msg)

Voxel::Voxel(uint32_t x,
             uint32_t y,
             uint32_t z,
             double meanX,
             double meanY,
             double meanZ,
             float intensity,
             float density)
    : status_(0),
      x_(x),
      y_(y),
      z_(z),
      intensity_(intensity),
      density_(density),
      meanX_(meanX),
      meanY_(meanY),
      meanZ_(meanZ),
      element_(0),
      cluster_(0)
{
}

void Voxel::set(uint32_t x,
                uint32_t y,
                uint32_t z,
                double meanX,
                double meanY,
                double meanZ,
                float intensity,
                float density,
                uint32_t status,
                uint32_t element,
                uint32_t cluster)

{
    status_ = status;

    x_ = x;
    y_ = y;
    z_ = z;

    intensity_ = intensity;
    density_ = density;

    meanX_ = meanX;
    meanY_ = meanY;
    meanZ_ = meanZ;

    element_ = element;
    cluster_ = cluster;
}

void Voxel::clear()
{
    status_ = 0;

    x_ = 0;
    y_ = 0;
    z_ = 0;

    intensity_ = 0;
    density_ = 0;

    meanX_ = 0;
    meanY_ = 0;
    meanZ_ = 0;

    element_ = 0;
    cluster_ = 0;
}
