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

/** @file TestRange.cpp */

// Include 3D Forest.
#include <Range.hpp>
#include <Test.hpp>
#include <Util.hpp>

TEST_CASE(TestRangeDefault)
{
    Range<double> r;

    TEST(r.empty());
    TEST(r.full());
    TEST(r.enabled());
}

TEST_CASE(TestRangeConstructor2)
{
    Range<double> r{0.0, 1.0};

    TEST(equal(r.minimum(), 0.0));
    TEST(equal(r.minimumValue(), 0.0));
    TEST(equal(r.maximumValue(), 1.0));
    TEST(equal(r.maximum(), 1.0));
    TEST(!r.contains(-0.1));
    TEST(r.contains(0.5));
    TEST(!r.contains(1.1));
    TEST(!r.empty());
    TEST(r.full());
    TEST(r.enabled());
}

TEST_CASE(TestRangeLow)
{
    Range<double> r{0.3, 0.5, 1.0, 1.0};

    TEST(equal(r.minimum(), 0.3));
    TEST(equal(r.minimumValue(), 0.5));
    TEST(equal(r.maximumValue(), 1.0));
    TEST(equal(r.maximum(), 1.0));
    TEST(!r.contains(0.4));
    TEST(r.contains(0.5));
    TEST(!r.contains(1.1));
    TEST(!r.empty());
    TEST(!r.full());
    TEST(r.enabled());
}

TEST_CASE(TestRangeHigh)
{
    Range<double> r{0.3, 0.3, 0.8, 1.0};

    TEST(equal(r.minimum(), 0.3));
    TEST(equal(r.minimumValue(), 0.3));
    TEST(equal(r.maximumValue(), 0.8));
    TEST(equal(r.maximum(), 1.0));
    TEST(!r.contains(0.2));
    TEST(r.contains(0.8));
    TEST(!r.contains(0.9));
    TEST(!r.empty());
    TEST(!r.full());
    TEST(r.enabled());
}
