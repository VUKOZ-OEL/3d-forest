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

/** @file exampleColorPalette.cpp @brief Color palette example. */

// Include 3D Forest.
#include <ColorPalette.hpp>

// Include local.
#define LOG_MODULE_NAME "exampleColorPalette"
#include <Log.hpp>

static void print(const std::vector<Vector3<double>> &pal)
{
    char buffer[32];
    uint32_t rgb;

    for (size_t i = 0; i < pal.size(); i++)
    {
        rgb = ((static_cast<uint32_t>(pal[i][0] * 255.0) & 0xffU) << 16) |
              ((static_cast<uint32_t>(pal[i][1] * 255.0) & 0xffU) << 8) |
              (static_cast<uint32_t>(pal[i][2] * 255.0) & 0xffU);

        (void)snprintf(buffer, sizeof(buffer), "0x%06x", rgb);

        if (i < 10)
        {
            std::cout << " " << i;
        }
        else
        {
            std::cout << i;
        }

        std::cout << ": " << pal[i] << " = " << buffer << "\n";
    }
}

int main()
{
    print(ColorPalette::Classification);

    size_t nColors = 16;
    print(ColorPalette::blueCyanYellowRed(nColors));

    return 0;
}
