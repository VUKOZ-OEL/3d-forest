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

/** @file TestBox.cpp */

// Include 3D Forest.
#include <Box.hpp>
#include <Test.hpp>
#include <Util.hpp>

TEST_CASE(TestBoxDefault)
{
    Box<double> box;

    TEST(box.empty());
    TEST(isEqual(box.radius(), 0.0));
    TEST(isEqual(box.distance(0.0, 0.0, 1.0), 1.0));
}

TEST_CASE(TestBoxOnePointFromVector)
{
    std::vector<double> xyz{0.0, 0.0, 1.0};

    Box<double> box;
    box.set(xyz);

    TEST(!box.empty());
    TEST(isEqual(box.radius(), 0.0));
    TEST(isEqual(box.distance(0.0, 0.0, 1.0), 0.0));
    TEST(isEqual(box.distance(0.0, 0.0, 0.0), 1.0));
}

TEST_CASE(TestBoxOnePointExtend)
{
    Box<double> box;
    box.extend(0.0, 0.0, 1.0);

    TEST(!box.empty());
    TEST(isEqual(box.radius(), 0.0));
    TEST(isEqual(box.distance(0.0, 0.0, 1.0), 0.0));
    TEST(isEqual(box.distance(0.0, 0.0, 0.0), 1.0));
}

TEST_CASE(TestBoxTwoPointsExtend)
{
    Box<double> box;
    box.extend(0.0, 0.0, 1.0);
    box.extend(0.0, 0.0, 2.0);

    TEST(!box.empty());
    TEST(isEqual(box.radius(), 0.5));
    TEST(isEqual(box.distance(0.0, 0.0, 1.0), 0.5));
    TEST(isEqual(box.distance(0.0, 0.0, 0.0), 1.5));
}
