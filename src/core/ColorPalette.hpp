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

// Include std.
#include <vector>

// Include 3D Forest.
#include <Vector3.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Color Palette. */
class EXPORT_CORE ColorPalette
{
public:
    static const std::vector<Vector3<double>> Classification;
    static const std::vector<Vector3<double>> BlackRed16;
    static const std::vector<Vector3<double>> BlackGreen16;
    static const std::vector<Vector3<double>> RedBlack16;
    static const std::vector<Vector3<double>> RedGreen16;
    static const std::vector<Vector3<double>> RedYellow16;
    static const std::vector<Vector3<double>> BlueYellow16;
    static const std::vector<Vector3<double>> BlueCyanYellowRed16;
    static const std::vector<Vector3<double>> BlueCyanYellowRed256;
    static const std::vector<Vector3<double>> BlueCyanGreenYellowRed16;

    static const std::vector<Vector3<double>> WindowsXp32;
    static const std::vector<Vector3<double>> MPN65;

    static std::vector<Vector3<double>> blueCyanYellowRed(size_t nColors);
    static std::vector<Vector3<double>> blueCyanGreenYellowRed(size_t nColors);
    static std::vector<Vector3<double>> gray(size_t nColors);
};

#include <WarningsEnable.hpp>

#endif /* COLOR_PALETTE_HPP */
