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

/** @file exampleVector3.cpp @brief Vector3 example. */

#include <Vector3.hpp>

#define LOG_MODULE_NAME "exampleVector3"
#include <Log.hpp>

int main()
{
    Vector3<double> v1{1, 1, 0};

    std::cout << "v1: " << v1 << "\n";
    std::cout << "v1.length: " << v1.length() << "\n";
    std::cout << "v1.max: " << v1.max() << "\n";
    std::cout << "v1.min: " << v1.min() << "\n";
    std::cout << "v1.normalized: " << v1.normalized() << "\n";
    std::cout << "v1 + v1: " << v1 + v1 << "\n";

    return 0;
}
