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

#define LOG_MODULE_NAME "ColorPalette"
#include <Log.hpp>

#if !defined(EXPORT_CORE_IMPORT)
const std::vector<Vector3<double>> ColorPalette::Classification{
    0xffffffU, //  0 : Created, Never classified
    0xb2b2b2U, //  1 : Unassigned - Unclassified
    0xa87102U, //  2 : Ground
    0x407d00U, //  3 : Low Vegetation
    0x5cb200U, //  4 : Medium Vegetation
    0x85ff00U, //  5 : High Vegetation
    0xe64d43U, //  6 : Building
    0xe60101U, //  7 : Low Point (Noise)
    0xff55ffU, //  8 :   Reserved
    0x035ee6U, //  9 : Water
    0x8504a9U, // 10 : Rail
    0xffff03U, // 11 : Road Surface
    0xee4feeU, // 12 :   Reserved
    0xaaffffU, // 13 : Wire - Guard (Shield)
    0x00ffffU, // 14 : Wire - Conductor (Phase)
    0xb3b300U, // 15 : Transmission Tower
    0xaaaaffU, // 16 : Wire-Structure Connector (Insulator)
    0xe6e6e6U, // 17 : Bridge Deck
    0xff0000U, // 18 : High Noise
               // 19 - 63 : Reserved
               // 19 : Overhead Structure (mining equipment, traffic lights)
               // 20 : Ignored Ground (breakline proximity)
               // 21 : Snow
               // 22 : Temporal Exclusion (water levels, landslides, permafrost)
               // 64 - 255 : User Definable
};

const std::vector<Vector3<double>> ColorPalette::BlackRed16 = {
    {0x0 * (1.0 / 15), 0.0, 0.0},
    {0x1 * (1.0 / 15), 0.0, 0.0},
    {0x2 * (1.0 / 15), 0.0, 0.0},
    {0x3 * (1.0 / 15), 0.0, 0.0},
    {0x4 * (1.0 / 15), 0.0, 0.0},
    {0x5 * (1.0 / 15), 0.0, 0.0},
    {0x6 * (1.0 / 15), 0.0, 0.0},
    {0x7 * (1.0 / 15), 0.0, 0.0},
    {0x8 * (1.0 / 15), 0.0, 0.0},
    {0x9 * (1.0 / 15), 0.0, 0.0},
    {0xa * (1.0 / 15), 0.0, 0.0},
    {0xb * (1.0 / 15), 0.0, 0.0},
    {0xc * (1.0 / 15), 0.0, 0.0},
    {0xd * (1.0 / 15), 0.0, 0.0},
    {0xe * (1.0 / 15), 0.0, 0.0},
    {0xf * (1.0 / 15), 0.0, 0.0},
};

const std::vector<Vector3<double>> ColorPalette::BlackGreen16 = {
    {0.0, 0x0 * (1.0 / 15), 0.0},
    {0.0, 0x1 * (1.0 / 15), 0.0},
    {0.0, 0x2 * (1.0 / 15), 0.0},
    {0.0, 0x3 * (1.0 / 15), 0.0},
    {0.0, 0x4 * (1.0 / 15), 0.0},
    {0.0, 0x5 * (1.0 / 15), 0.0},
    {0.0, 0x6 * (1.0 / 15), 0.0},
    {0.0, 0x7 * (1.0 / 15), 0.0},
    {0.0, 0x8 * (1.0 / 15), 0.0},
    {0.0, 0x9 * (1.0 / 15), 0.0},
    {0.0, 0xa * (1.0 / 15), 0.0},
    {0.0, 0xb * (1.0 / 15), 0.0},
    {0.0, 0xc * (1.0 / 15), 0.0},
    {0.0, 0xd * (1.0 / 15), 0.0},
    {0.0, 0xe * (1.0 / 15), 0.0},
    {0.0, 0xf * (1.0 / 15), 0.0},
};

const std::vector<Vector3<double>> ColorPalette::RedBlack16 = {
    {1.0 - 0x0 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x1 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x2 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x3 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x4 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x5 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x6 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x7 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x8 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0x9 * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0xa * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0xb * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0xc * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0xd * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0xe * (1.0 / 15), 0.0, 0.0},
    {1.0 - 0xf * (1.0 / 15), 0.0, 0.0},
};

const std::vector<Vector3<double>> ColorPalette::RedGreen16 = {
    {1.0 - 0x0 * (1.0 / 15), 0x0 * (1.0 / 15), 0.0},
    {1.0 - 0x1 * (1.0 / 15), 0x1 * (1.0 / 15), 0.0},
    {1.0 - 0x2 * (1.0 / 15), 0x2 * (1.0 / 15), 0.0},
    {1.0 - 0x3 * (1.0 / 15), 0x3 * (1.0 / 15), 0.0},
    {1.0 - 0x4 * (1.0 / 15), 0x4 * (1.0 / 15), 0.0},
    {1.0 - 0x5 * (1.0 / 15), 0x5 * (1.0 / 15), 0.0},
    {1.0 - 0x6 * (1.0 / 15), 0x6 * (1.0 / 15), 0.0},
    {1.0 - 0x7 * (1.0 / 15), 0x7 * (1.0 / 15), 0.0},
    {1.0 - 0x8 * (1.0 / 15), 0x8 * (1.0 / 15), 0.0},
    {1.0 - 0x9 * (1.0 / 15), 0x9 * (1.0 / 15), 0.0},
    {1.0 - 0xa * (1.0 / 15), 0xa * (1.0 / 15), 0.0},
    {1.0 - 0xb * (1.0 / 15), 0xb * (1.0 / 15), 0.0},
    {1.0 - 0xc * (1.0 / 15), 0xc * (1.0 / 15), 0.0},
    {1.0 - 0xd * (1.0 / 15), 0xd * (1.0 / 15), 0.0},
    {1.0 - 0xe * (1.0 / 15), 0xe * (1.0 / 15), 0.0},
    {1.0 - 0xf * (1.0 / 15), 0xf * (1.0 / 15), 0.0},
};

const std::vector<Vector3<double>> ColorPalette::RedYellow16 = {
    {1.0, 0x0 * (1.0 / 15), 0.0},
    {1.0, 0x1 * (1.0 / 15), 0.0},
    {1.0, 0x2 * (1.0 / 15), 0.0},
    {1.0, 0x3 * (1.0 / 15), 0.0},
    {1.0, 0x4 * (1.0 / 15), 0.0},
    {1.0, 0x5 * (1.0 / 15), 0.0},
    {1.0, 0x6 * (1.0 / 15), 0.0},
    {1.0, 0x7 * (1.0 / 15), 0.0},
    {1.0, 0x8 * (1.0 / 15), 0.0},
    {1.0, 0x9 * (1.0 / 15), 0.0},
    {1.0, 0xa * (1.0 / 15), 0.0},
    {1.0, 0xb * (1.0 / 15), 0.0},
    {1.0, 0xc * (1.0 / 15), 0.0},
    {1.0, 0xd * (1.0 / 15), 0.0},
    {1.0, 0xe * (1.0 / 15), 0.0},
    {1.0, 0xf * (1.0 / 15), 0.0},
};

const std::vector<Vector3<double>> ColorPalette::BlueYellow16 = {
    {0x0 * (1.0 / 15), 1.0, 1.0 - 0x0 * (1.0 / 15)},
    {0x1 * (1.0 / 15), 1.0, 1.0 - 0x1 * (1.0 / 15)},
    {0x2 * (1.0 / 15), 1.0, 1.0 - 0x2 * (1.0 / 15)},
    {0x3 * (1.0 / 15), 1.0, 1.0 - 0x3 * (1.0 / 15)},
    {0x4 * (1.0 / 15), 1.0, 1.0 - 0x4 * (1.0 / 15)},
    {0x5 * (1.0 / 15), 1.0, 1.0 - 0x5 * (1.0 / 15)},
    {0x6 * (1.0 / 15), 1.0, 1.0 - 0x6 * (1.0 / 15)},
    {0x7 * (1.0 / 15), 1.0, 1.0 - 0x7 * (1.0 / 15)},
    {0x8 * (1.0 / 15), 1.0, 1.0 - 0x8 * (1.0 / 15)},
    {0x9 * (1.0 / 15), 1.0, 1.0 - 0x9 * (1.0 / 15)},
    {0xa * (1.0 / 15), 1.0, 1.0 - 0xa * (1.0 / 15)},
    {0xb * (1.0 / 15), 1.0, 1.0 - 0xb * (1.0 / 15)},
    {0xc * (1.0 / 15), 1.0, 1.0 - 0xc * (1.0 / 15)},
    {0xd * (1.0 / 15), 1.0, 1.0 - 0xd * (1.0 / 15)},
    {0xe * (1.0 / 15), 1.0, 1.0 - 0xe * (1.0 / 15)},
    {0xf * (1.0 / 15), 1.0, 1.0 - 0xf * (1.0 / 15)},
};

const std::vector<Vector3<double>> ColorPalette::BlueCyanYellowRed16 = {
    {0.000000, 0.000000, 0.500000},
    {0.000000, 0.000000, 0.750000},
    {0.000000, 0.000000, 1.000000},
    {0.000000, 0.250000, 1.000000},
    {0.000000, 0.500000, 1.000000},
    {0.000000, 0.750000, 1.000000},
    {0.000000, 1.000000, 1.000000},
    {0.250000, 1.000000, 0.750000},
    {0.500000, 1.000000, 0.500000},
    {0.750000, 1.000000, 0.250000},
    {1.000000, 1.000000, 0.000000},
    {1.000000, 0.750000, 0.000000},
    {1.000000, 0.500000, 0.000000},
    {1.000000, 0.250000, 0.000000},
    {1.000000, 0.000000, 0.000000},
    {0.500000, 0.000000, 0.000000},
};

const std::vector<Vector3<double>> ColorPalette::BlueCyanYellowRed256 = {
    0x00007fU, 0x000083U, 0x000087U, 0x00008bU, 0x00008fU, 0x000093U, 0x000097U,
    0x00009bU, 0x00009fU, 0x0000a3U, 0x0000a7U, 0x0000abU, 0x0000afU, 0x0000b3U,
    0x0000b7U, 0x0000bbU, 0x0000bfU, 0x0000c3U, 0x0000c7U, 0x0000cbU, 0x0000cfU,
    0x0000d3U, 0x0000d7U, 0x0000dbU, 0x0000dfU, 0x0000e3U, 0x0000e7U, 0x0000ebU,
    0x0000efU, 0x0000f3U, 0x0000f7U, 0x0000fbU, 0x0000ffU, 0x0003ffU, 0x0007ffU,
    0x000bffU, 0x000fffU, 0x0013ffU, 0x0017ffU, 0x001bffU, 0x001fffU, 0x0023ffU,
    0x0027ffU, 0x002bffU, 0x002fffU, 0x0033ffU, 0x0037ffU, 0x003bffU, 0x003fffU,
    0x0043ffU, 0x0047ffU, 0x004bffU, 0x004fffU, 0x0053ffU, 0x0057ffU, 0x005bffU,
    0x005fffU, 0x0063ffU, 0x0067ffU, 0x006bffU, 0x006fffU, 0x0073ffU, 0x0077ffU,
    0x007bffU, 0x007fffU, 0x0083ffU, 0x0087ffU, 0x008bffU, 0x008fffU, 0x0093ffU,
    0x0097ffU, 0x009bffU, 0x009fffU, 0x00a3ffU, 0x00a7ffU, 0x00abffU, 0x00afffU,
    0x00b3ffU, 0x00b7ffU, 0x00bbffU, 0x00bfffU, 0x00c3ffU, 0x00c7ffU, 0x00cbffU,
    0x00cfffU, 0x00d3ffU, 0x00d7ffU, 0x00dbffU, 0x00dfffU, 0x00e3ffU, 0x00e7ffU,
    0x00ebffU, 0x00efffU, 0x00f3ffU, 0x00f7ffU, 0x00fbffU, 0x00ffffU, 0x03fffbU,
    0x07fff7U, 0x0bfff3U, 0x0fffefU, 0x13ffebU, 0x17ffe7U, 0x1bffe3U, 0x1fffdfU,
    0x23ffdbU, 0x27ffd7U, 0x2bffd3U, 0x2fffcfU, 0x33ffcbU, 0x37ffc7U, 0x3bffc3U,
    0x3fffbfU, 0x43ffbbU, 0x47ffb7U, 0x4bffb3U, 0x4fffafU, 0x53ffabU, 0x57ffa7U,
    0x5bffa3U, 0x5fff9fU, 0x63ff9bU, 0x67ff97U, 0x6bff93U, 0x6fff8fU, 0x73ff8bU,
    0x77ff87U, 0x7bff83U, 0x7fff7fU, 0x83ff7bU, 0x87ff77U, 0x8bff73U, 0x8fff6fU,
    0x93ff6bU, 0x97ff67U, 0x9bff63U, 0x9fff5fU, 0xa3ff5bU, 0xa7ff57U, 0xabff53U,
    0xafff4fU, 0xb3ff4bU, 0xb7ff47U, 0xbbff43U, 0xbfff3fU, 0xc3ff3bU, 0xc7ff37U,
    0xcbff33U, 0xcfff2fU, 0xd3ff2bU, 0xd7ff27U, 0xdbff23U, 0xdfff1fU, 0xe3ff1bU,
    0xe7ff17U, 0xebff13U, 0xefff0fU, 0xf3ff0bU, 0xf7ff07U, 0xfbff03U, 0xffff00U,
    0xfffb00U, 0xfff700U, 0xfff300U, 0xffef00U, 0xffeb00U, 0xffe700U, 0xffe300U,
    0xffdf00U, 0xffdb00U, 0xffd700U, 0xffd300U, 0xffcf00U, 0xffcb00U, 0xffc700U,
    0xffc300U, 0xffbf00U, 0xffbb00U, 0xffb700U, 0xffb300U, 0xffaf00U, 0xffab00U,
    0xffa700U, 0xffa300U, 0xff9f00U, 0xff9b00U, 0xff9700U, 0xff9300U, 0xff8f00U,
    0xff8b00U, 0xff8700U, 0xff8300U, 0xff7f00U, 0xff7b00U, 0xff7700U, 0xff7300U,
    0xff6f00U, 0xff6b00U, 0xff6700U, 0xff6300U, 0xff5f00U, 0xff5b00U, 0xff5700U,
    0xff5300U, 0xff4f00U, 0xff4b00U, 0xff4700U, 0xff4300U, 0xff3f00U, 0xff3b00U,
    0xff3700U, 0xff3300U, 0xff2f00U, 0xff2b00U, 0xff2700U, 0xff2300U, 0xff1f00U,
    0xff1b00U, 0xff1700U, 0xff1300U, 0xff0f00U, 0xff0b00U, 0xff0700U, 0xff0300U,
    0xff0000U, 0xfa0000U, 0xf60000U, 0xf20000U, 0xee0000U, 0xea0000U, 0xe60000U,
    0xe20000U, 0xde0000U, 0xd90000U, 0xd50000U, 0xd10000U, 0xcd0000U, 0xc90000U,
    0xc50000U, 0xc10000U, 0xbd0000U, 0xb90000U, 0xb40000U, 0xb00000U, 0xac0000U,
    0xa80000U, 0xa40000U, 0xa00000U, 0x9c0000U, 0x980000U, 0x940000U, 0x8f0000U,
    0x8b0000U, 0x870000U, 0x830000U, 0x7f0000U,
};

const std::vector<Vector3<double>> ColorPalette::BlueCyanGreenYellowRed16 = {
    {0.000000, 0.000000, 1.000000},
    {0.000000, 0.333333, 1.000000},
    {0.000000, 0.666667, 1.000000},
    {0.000000, 1.000000, 1.000000},
    {0.000000, 1.000000, 0.666667},
    {0.000000, 1.000000, 0.333333},
    {0.000000, 1.000000, 0.000000},
    {0.333333, 1.000000, 0.000000},
    {0.666667, 1.000000, 0.000000},
    {1.000000, 1.000000, 0.000000},
    {1.000000, 0.833333, 0.000000},
    {1.000000, 0.666667, 0.000000},
    {1.000000, 0.500000, 0.000000},
    {1.000000, 0.333333, 0.000000},
    {1.000000, 0.166667, 0.000000},
    {1.000000, 0.000000, 0.000000},
};

const std::vector<Vector3<double>> ColorPalette::WindowsXp32 = {
    0x996600U, 0xcc9900U, 0xffcc00U, 0xffff00U, 0xffff99U, 0xffdb9dU,
    0xffcc66U, 0xff9933U, 0xff794bU, 0xff3300U, 0x990000U, 0x333366U,
    0x003399U, 0x0066ccU, 0x0083d7U, 0x0099ffU,

    0x3e9adeU, 0x99ccffU, 0xb4e2ffU, 0xdeffffU, 0xffccffU, 0xccccffU,
    0x9999ffU, 0x6666ccU, 0x9999ccU, 0x666699U, 0x006600U, 0x009900U,
    0x66cc33U, 0x99ff66U, 0xccf4ccU, 0xffffffU};
#endif /* EXPORT_CORE_IMPORT */

std::vector<Vector3<double>> ColorPalette::blueCyanYellowRed(size_t nColors)
{
    std::vector<Vector3<double>> colormap;
    colormap.resize(nColors);

    // Blue
    size_t i1 = nColors / 8;
    double delta = 0.5 / static_cast<double>(i1);
    for (size_t i = 0; i < i1; i++)
    {
        double v = delta * static_cast<double>(i);
        colormap[i].set(0.0, 0.0, 0.5 + v);
    }

    // Blue to Cyan
    size_t i2 = i1 + nColors / 4;
    delta = 1.0 / static_cast<double>(i2 - i1);
    for (size_t i = i1; i < i2; i++)
    {
        double v = delta * static_cast<double>(i - i1);
        colormap[i].set(0.0, v, 1.0);
    }

    // Cyan to Yellow
    size_t i3 = i2 + nColors / 4;
    delta = 1.0 / static_cast<double>(i3 - i2);
    for (size_t i = i2; i < i3; i++)
    {
        double v = delta * static_cast<double>(i - i2);
        colormap[i].set(v, 1.0, 1.0 - v);
    }

    // Yellow to Red
    size_t i4 = i3 + nColors / 4;
    delta = 1.0 / static_cast<double>(i4 - i3);
    for (size_t i = i3; i < i4; i++)
    {
        double v = delta * static_cast<double>(i - i3);
        colormap[i].set(1.0, 1.0 - v, 0.0);
    }

    // Red
    delta = 0.5 / static_cast<double>(nColors - i4 - 1);
    for (size_t i = i4; i < nColors; i++)
    {
        double v = delta * static_cast<double>(i - i4);
        colormap[i].set(1.0 - v, 0.0, 0.0);
    }

    return colormap;
}

std::vector<Vector3<double>> ColorPalette::blueCyanGreenYellowRed(
    size_t nColors)
{
    std::vector<Vector3<double>> colormap;
    colormap.resize(nColors);

    // Blue to Cyan
    size_t i1 = nColors / 5;
    double delta = 1.0 / static_cast<double>(i1);
    for (size_t i = 0; i < i1; i++)
    {
        double v = delta * static_cast<double>(i);
        colormap[i].set(0.0, v, 1.0);
    }

    // Cyan to Green
    size_t i2 = i1 + nColors / 5;
    delta = 1.0 / static_cast<double>(i2 - i1);
    for (size_t i = i1; i < i2; i++)
    {
        double v = delta * static_cast<double>(i - i1);
        colormap[i].set(0.0, 1.0, 1.0 - v);
    }

    // Green to Yellow
    size_t i3 = i2 + nColors / 7;
    delta = 1.0 / static_cast<double>(i3 - i2);
    for (size_t i = i2; i < i3; i++)
    {
        double v = delta * static_cast<double>(i - i2);
        colormap[i].set(v, 1.0, 0.0);
    }

    // Yellow to Red
    delta = 1.0 / static_cast<double>(nColors - i3 - 1);
    for (size_t i = i3; i < nColors; i++)
    {
        double v = delta * static_cast<double>(i - i3);
        colormap[i].set(1.0, 1.0 - v, 0.0);
    }

    return colormap;
}

std::vector<Vector3<double>> ColorPalette::gray(size_t nColors)
{
    std::vector<Vector3<double>> colormap;
    colormap.resize(nColors);

    double delta = 1.0 / static_cast<double>(nColors - 1);
    for (size_t i = 0; i < nColors; i++)
    {
        double v = delta * static_cast<double>(i);
        colormap[i].set(v, v, v);
    }

    return colormap;
}
