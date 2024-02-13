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

/** @file TestProgressCounter.cpp */

// Include 3D Forest.
#include <ProgressCounter.hpp>
#include <Test.hpp>

TEST_CASE(TestProgressCounterEmpty)
{
    ProgressCounter pc;

    TEST(pc.end());
    TEST(pc.percentToInt() == 100);
}

TEST_CASE(TestProgressCounterSingle10)
{
    ProgressCounter pc;

    pc.setMaximumStep(20);
    pc.setValueStep(2);

    TEST(!pc.end());
    TEST(pc.percentToInt() == 10);
}

TEST_CASE(TestProgressCounterSingle100)
{
    ProgressCounter pc;

    pc.setMaximumStep(20);
    pc.setValueStep(20);

    TEST(pc.end());
    TEST(pc.percentToInt() == 100);
}

TEST_CASE(TestProgressCounterMulti)
{
    ProgressCounter pc;

    pc.setMaximumSteps(2);
    pc.setValueSteps(1);
    pc.setMaximumStep(20);

    pc.setValueStep(2);
    TEST(!pc.end());
    TEST(pc.percentToInt() == 55);

    pc.setValueStep(20);
    TEST(pc.percentToInt() == 100);

    pc.setValueSteps(pc.maximumSteps());
    TEST(pc.end());
}

TEST_CASE(TestProgressCounterMultiPercent)
{
    ProgressCounter pc;

    pc.setMaximumSteps({5.0, 80.0, 15.0});
    pc.setMaximumStep(20);

    pc.setValueStep(16);
    TEST(!pc.end());
    TEST(pc.percentToInt() == 4);

    pc.setValueSteps(1);
    pc.setValueStep(5);
    TEST(!pc.end());
    TEST(pc.percentToInt() == 25);

    pc.setValueStep(20);
    TEST(!pc.end());
    TEST(pc.percentToInt() == 85);

    pc.setValueSteps(2);
    pc.setValueStep(0);
    TEST(!pc.end());
    TEST(pc.percentToInt() == 85);

    pc.setValueStep(20);
    TEST(pc.percentToInt() == 100);
}
