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

/** @file TestUtil.cpp */

// Include 3D Forest.
#include <Test.hpp>
#include <Util.hpp>

TEST_CASE(TestUtilSetEmpty)
{
    std::vector<int> v;

    set(v, 1);
    TEST(v.empty());
}

TEST_CASE(TestUtilSet1)
{
    std::vector<int> v{1};

    set(v, 2);
    TEST(v.size() == 1);
    TEST(v.at(0) == 2);
}

TEST_CASE(TestUtilInsert)
{
    std::vector<int> v;

    insert(v, 1);
    TEST(v.size() == 1);
    TEST(v.at(0) == 1);

    insert(v, 2);
    TEST(v.size() == 2);
    TEST(v.at(0) == 1);
    TEST(v.at(1) == 2);

    insert(v, 0);
    TEST(v.size() == 3);
    TEST(v.at(0) == 0);
    TEST(v.at(1) == 1);
    TEST(v.at(2) == 2);
}

TEST_CASE(TestUtilInsertAt)
{
    std::vector<int> v;

    insertAt(v, 0, 1);
    TEST(v.size() == 1);
    TEST(v.at(0) == 1);

    insertAt(v, 0, 2);
    TEST(v.size() == 2);
    TEST(v.at(0) == 2);
    TEST(v.at(1) == 1);

    insertAt(v, 1, 3);
    TEST(v.size() == 3);
    TEST(v.at(0) == 2);
    TEST(v.at(1) == 3);
    TEST(v.at(2) == 1);
}

TEST_CASE(TestUtilRemove)
{
    std::vector<int> v{1, 2, 3};

    remove(v, 2);
    TEST(v.size() == 2);
    TEST(v.at(0) == 1);
    TEST(v.at(1) == 3);

    remove(v, 1);
    TEST(v.size() == 1);
    TEST(v.at(0) == 3);

    remove(v, 3);
    TEST(v.size() == 0);
}

TEST_CASE(TestUtilContains)
{
    std::vector<int> v0;
    TEST(!contains(v0, 1));

    std::vector<int> v1{1};
    TEST(contains(v1, 1));
    TEST(!contains(v1, 2));

    std::vector<int> v2{1, 2};
    TEST(contains(v2, 2));
    TEST(!contains(v2, 3));
}

TEST_CASE(TestUtilBinarySearch)
{
    std::vector<int> v0;
    TEST(binarySearch(v0, 1) == SIZE_MAX);

    std::vector<int> v1{1};
    TEST(binarySearch(v1, 1) == 0);
    TEST(binarySearch(v1, 2) == SIZE_MAX);

    std::vector<int> v3{1, 2, 3};
    TEST(binarySearch(v3, 1) == 0);
    TEST(binarySearch(v3, 2) == 1);
    TEST(binarySearch(v3, 3) == 2);
    TEST(binarySearch(v3, 4) == SIZE_MAX);
}
