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

/** @file exampleImage.cpp @brief Mesh distance example. */

// Include 3rd party.
#include <Eigen/Core>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <igl/png/writePNG.h>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "exampleImage"
#include <Log.hpp>

static void exampleImageRGB()
{
    Eigen::Index width = 3;
    Eigen::Index height = 2;

    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R;
    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> G;
    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> B;
    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> A;

    R.resize(width, height);
    G.resize(width, height);
    B.resize(width, height);
    A.resize(width, height);

    for (Eigen::Index y = 0; y < height; y++)
    {
        for (Eigen::Index x = 0; x < width; x++)
        {
            R(x, y) = 0U;
            G(x, y) = 0U;
            B(x, y) = 0U;
            A(x, y) = 255U;
        }
    }

    // y
    // 1 red,   green, blue
    // 0 black, gray,  white
    //   0      1      2     x

    R(0, 1) = 255U;
    G(1, 1) = 255U;
    B(2, 1) = 255U;

    R(1, 0) = 128U;
    G(1, 0) = 128U;
    B(1, 0) = 128U;
    R(2, 0) = 255U;
    G(2, 0) = 255U;
    B(2, 0) = 255U;

    igl::png::writePNG(R, G, B, A, "outputRGB.png");
}

static void exampleImageGrayscale()
{
    Eigen::Index width = 3;
    Eigen::Index height = 1;

    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> I;

    I.resize(width, height);

    I(0, 0) = 0;
    I(1, 0) = 128U;
    I(2, 0) = 255U;

    igl::png::writePNG(I, "outputGrayscale.png");
}

static void exampleImage()
{
    exampleImageRGB();
    exampleImageGrayscale();
}

int main()
{
    try
    {
        exampleImage();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
