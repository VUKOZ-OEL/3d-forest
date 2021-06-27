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

/** @file ColorPalette.hpp */

#ifndef COLOR_PALETTE_HPP
#define COLOR_PALETTE_HPP

#include <Vector3.hpp>
#include <vector>

/** Color Palette. */
class ColorPalette
{
public:
    static const std::vector<Vector3<float>> Classification;
    static const std::vector<Vector3<float>> BlackRed16;
    static const std::vector<Vector3<float>> BlackGreen16;
    static const std::vector<Vector3<float>> RedBlack16;
    static const std::vector<Vector3<float>> RedGreen16;
    static const std::vector<Vector3<float>> RedYellow16;
    static const std::vector<Vector3<float>> BlueYellow16;
    static const std::vector<Vector3<float>> BlueCyanYellowRed16;
    static const std::vector<Vector3<float>> BlueCyanYellowRed256;
    static const std::vector<Vector3<float>> BlueCyanGreenYellowRed16;
    static const std::vector<Vector3<float>> WindowsXp32;

    static std::vector<Vector3<float>> blueCyanYellowRed(size_t nColors);
    static std::vector<Vector3<float>> blueCyanGreenYellowRed(size_t nColors);
    static std::vector<Vector3<float>> gray(size_t nColors);
};

#endif /* COLOR_PALETTE_HPP */
