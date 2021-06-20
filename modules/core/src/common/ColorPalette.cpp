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

/** @file ColorPalette.cpp */

#include <ColorPalette.hpp>

const std::vector<Vector3<float>> ColorPalette::Classification{
    0xffffffU, //  0 : Never classified
    0xb2b2b2U, //  1 : Unassigned
    0xa87102U, //  2 : Ground
    0x407d00U, //  3 : Low Vegetation
    0x5cb200U, //  4 : Medium Vegetation
    0x85ff00U, //  5 : High Vegetation
    0xe64d43U, //  6 : Building
    0xe60101U, //  7 : Low Point
    0xff55ffU, //  8 : Reserved
    0x035ee6U, //  9 : Water
    0x8504a9U, // 10 : Rail
    0xffff03U, // 11 : Road Surface
    0xee4feeU, // 12 : Reserved
    0xaaffffU, // 13 : Wire - Guard (Shield)
    0x00ffffU, // 14 : Wire - Conductor (Phase)
    0xb3b300U, // 15 : Transmission Tower
    0xaaaaffU, // 16 : Wire-Structure Connector (Insulator)
    0xe6e6e6U, // 17 : Bridge Deck
    0xff0000U, // 18 : High Noise
               // 19 - 63 : Reserved
               // 64 - 255 : User Definable
};

const std::vector<Vector3<float>> ColorPalette::BlackRed16 = {
    {0x0 * (1.0F / 15), 0.0F, 0.0F},
    {0x1 * (1.0F / 15), 0.0F, 0.0F},
    {0x2 * (1.0F / 15), 0.0F, 0.0F},
    {0x3 * (1.0F / 15), 0.0F, 0.0F},
    {0x4 * (1.0F / 15), 0.0F, 0.0F},
    {0x5 * (1.0F / 15), 0.0F, 0.0F},
    {0x6 * (1.0F / 15), 0.0F, 0.0F},
    {0x7 * (1.0F / 15), 0.0F, 0.0F},
    {0x8 * (1.0F / 15), 0.0F, 0.0F},
    {0x9 * (1.0F / 15), 0.0F, 0.0F},
    {0xa * (1.0F / 15), 0.0F, 0.0F},
    {0xb * (1.0F / 15), 0.0F, 0.0F},
    {0xc * (1.0F / 15), 0.0F, 0.0F},
    {0xd * (1.0F / 15), 0.0F, 0.0F},
    {0xe * (1.0F / 15), 0.0F, 0.0F},
    {0xf * (1.0F / 15), 0.0F, 0.0F},
};

const std::vector<Vector3<float>> ColorPalette::BlackGreen16 = {
    {0.0F, 0x0 * (1.0F / 15), 0.0F},
    {0.0F, 0x1 * (1.0F / 15), 0.0F},
    {0.0F, 0x2 * (1.0F / 15), 0.0F},
    {0.0F, 0x3 * (1.0F / 15), 0.0F},
    {0.0F, 0x4 * (1.0F / 15), 0.0F},
    {0.0F, 0x5 * (1.0F / 15), 0.0F},
    {0.0F, 0x6 * (1.0F / 15), 0.0F},
    {0.0F, 0x7 * (1.0F / 15), 0.0F},
    {0.0F, 0x8 * (1.0F / 15), 0.0F},
    {0.0F, 0x9 * (1.0F / 15), 0.0F},
    {0.0F, 0xa * (1.0F / 15), 0.0F},
    {0.0F, 0xb * (1.0F / 15), 0.0F},
    {0.0F, 0xc * (1.0F / 15), 0.0F},
    {0.0F, 0xd * (1.0F / 15), 0.0F},
    {0.0F, 0xe * (1.0F / 15), 0.0F},
    {0.0F, 0xf * (1.0F / 15), 0.0F},
};

const std::vector<Vector3<float>> ColorPalette::RedBlack16 = {
    {1.0F - 0x0 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x1 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x2 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x3 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x4 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x5 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x6 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x7 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x8 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0x9 * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0xa * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0xb * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0xc * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0xd * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0xe * (1.0F / 15), 0.0F, 0.0F},
    {1.0F - 0xf * (1.0F / 15), 0.0F, 0.0F},
};

const std::vector<Vector3<float>> ColorPalette::RedGreen16 = {
    {1.0F - 0x0 * (1.0F / 15), 0x0 * (1.0F / 15), 0.0F},
    {1.0F - 0x1 * (1.0F / 15), 0x1 * (1.0F / 15), 0.0F},
    {1.0F - 0x2 * (1.0F / 15), 0x2 * (1.0F / 15), 0.0F},
    {1.0F - 0x3 * (1.0F / 15), 0x3 * (1.0F / 15), 0.0F},
    {1.0F - 0x4 * (1.0F / 15), 0x4 * (1.0F / 15), 0.0F},
    {1.0F - 0x5 * (1.0F / 15), 0x5 * (1.0F / 15), 0.0F},
    {1.0F - 0x6 * (1.0F / 15), 0x6 * (1.0F / 15), 0.0F},
    {1.0F - 0x7 * (1.0F / 15), 0x7 * (1.0F / 15), 0.0F},
    {1.0F - 0x8 * (1.0F / 15), 0x8 * (1.0F / 15), 0.0F},
    {1.0F - 0x9 * (1.0F / 15), 0x9 * (1.0F / 15), 0.0F},
    {1.0F - 0xa * (1.0F / 15), 0xa * (1.0F / 15), 0.0F},
    {1.0F - 0xb * (1.0F / 15), 0xb * (1.0F / 15), 0.0F},
    {1.0F - 0xc * (1.0F / 15), 0xc * (1.0F / 15), 0.0F},
    {1.0F - 0xd * (1.0F / 15), 0xd * (1.0F / 15), 0.0F},
    {1.0F - 0xe * (1.0F / 15), 0xe * (1.0F / 15), 0.0F},
    {1.0F - 0xf * (1.0F / 15), 0xf * (1.0F / 15), 0.0F},
};

const std::vector<Vector3<float>> ColorPalette::RedYellow16 = {
    {1.0F, 0x0 * (1.0F / 15), 0.0F},
    {1.0F, 0x1 * (1.0F / 15), 0.0F},
    {1.0F, 0x2 * (1.0F / 15), 0.0F},
    {1.0F, 0x3 * (1.0F / 15), 0.0F},
    {1.0F, 0x4 * (1.0F / 15), 0.0F},
    {1.0F, 0x5 * (1.0F / 15), 0.0F},
    {1.0F, 0x6 * (1.0F / 15), 0.0F},
    {1.0F, 0x7 * (1.0F / 15), 0.0F},
    {1.0F, 0x8 * (1.0F / 15), 0.0F},
    {1.0F, 0x9 * (1.0F / 15), 0.0F},
    {1.0F, 0xa * (1.0F / 15), 0.0F},
    {1.0F, 0xb * (1.0F / 15), 0.0F},
    {1.0F, 0xc * (1.0F / 15), 0.0F},
    {1.0F, 0xd * (1.0F / 15), 0.0F},
    {1.0F, 0xe * (1.0F / 15), 0.0F},
    {1.0F, 0xf * (1.0F / 15), 0.0F},
};

const std::vector<Vector3<float>> ColorPalette::blueCyanGreenYellowRed16 = {
    {0.000000F, 0.000000F, 1.000000F},
    {0.000000F, 0.333333F, 1.000000F},
    {0.000000F, 0.666667F, 1.000000F},
    {0.000000F, 1.000000F, 1.000000F},
    {0.000000F, 1.000000F, 0.666667F},
    {0.000000F, 1.000000F, 0.333333F},
    {0.000000F, 1.000000F, 0.000000F},
    {0.333333F, 1.000000F, 0.000000F},
    {0.666667F, 1.000000F, 0.000000F},
    {1.000000F, 1.000000F, 0.000000F},
    {1.000000F, 0.833333F, 0.000000F},
    {1.000000F, 0.666667F, 0.000000F},
    {1.000000F, 0.500000F, 0.000000F},
    {1.000000F, 0.333333F, 0.000000F},
    {1.000000F, 0.166667F, 0.000000F},
    {1.000000F, 0.000000F, 0.000000F},
};

std::vector<Vector3<float>> ColorPalette::blueGreenRed(size_t nColors)
{
    std::vector<Vector3<float>> colormap;
    colormap.resize(nColors);

    size_t n1 = nColors / 3;
    float delta = 1.0F / static_cast<float>(n1);
    for (size_t i = 0; i < n1; i++)
    {
        float v = delta * static_cast<float>(i);
        colormap[i].set(0.0F, v, 1.0F);
    }

    size_t n2 = n1 * 2;
    delta = 1.0F / static_cast<float>(n2 - n1);
    for (size_t i = n1; i < n2; i++)
    {
        float v = delta * static_cast<float>(i - n1);
        colormap[i].set(v, 1.0F, 1.0F - v);
    }

    delta = 1.0F / static_cast<float>(nColors - n2 - 1);
    for (size_t i = n2; i < nColors; i++)
    {
        float v = delta * static_cast<float>(i - n2);
        colormap[i].set(1.0F, 1.0F - v, 0.0F);
    }

    return colormap;
}

std::vector<Vector3<float>> ColorPalette::blueCyanGreenYellowRed(size_t nColors)
{
    std::vector<Vector3<float>> colormap;
    colormap.resize(nColors);

    size_t n1 = nColors / 5;
    float delta = 1.0F / static_cast<float>(n1);
    for (size_t i = 0; i < n1; i++)
    {
        float v = delta * static_cast<float>(i);
        colormap[i].set(0.0F, v, 1.0F);
    }

    size_t n2 = n1 * 2;
    delta = 1.0F / static_cast<float>(n2 - n1);
    for (size_t i = n1; i < n2; i++)
    {
        float v = delta * static_cast<float>(i - n1);
        colormap[i].set(0.0F, 1.0F, 1.0F - v);
    }

    size_t n3 = n1 * 3;
    delta = 1.0F / static_cast<float>(n3 - n2);
    for (size_t i = n2; i < n3; i++)
    {
        float v = delta * static_cast<float>(i - n2);
        colormap[i].set(v, 1.0F, 0.0F);
    }

    delta = 1.0F / static_cast<float>(nColors - n3 - 1);
    for (size_t i = n3; i < nColors; i++)
    {
        float v = delta * static_cast<float>(i - n3);
        colormap[i].set(1.0F, 1.0F - v, 0.0F);
    }

    return colormap;
}

std::vector<Vector3<float>> ColorPalette::gray(size_t nColors)
{
    std::vector<Vector3<float>> colormap;
    colormap.resize(nColors);

    float delta = 1.0F / static_cast<float>(nColors - 1);
    for (size_t i = 0; i < nColors; i++)
    {
        float v = delta * static_cast<float>(i);
        colormap[i].set(v, v, v);
    }

    return colormap;
}
