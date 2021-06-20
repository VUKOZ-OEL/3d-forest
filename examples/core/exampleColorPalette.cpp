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

/** @file exampleColorPalette.cpp @brief ColorPalette example. */

#include <ColorPalette.hpp>

static void print(const std::vector<Vector3<float>> &pal)
{
    for (size_t i = 0; i < pal.size(); i++)
    {
        std::cout << i << ": " << pal[i] << "\n";
    }
}

int main()
{
    print(ColorPalette::Classification);

    size_t nColors = 16;
    print(ColorPalette::gray(nColors));

    return 0;
}
