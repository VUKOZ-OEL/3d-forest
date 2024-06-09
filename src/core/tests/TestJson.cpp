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

/** @file TestJson.cpp */

// Include 3D Forest.
#include <Json.hpp>
#include <Test.hpp>
#include <Util.hpp>

TEST_CASE(TestJsonString)
{
    Json obj;
    obj["name"] = "John";

    TEST(obj.contains("name"));
    TEST(obj["name"].isString());
    TEST(obj["name"].string() == "John");
}

TEST_CASE(TestJsonNumber)
{
    Json obj;
    obj["width"] = 5;

    TEST(obj.contains("width"));
    TEST(obj["width"].isNumber());
    TEST(isEqual(obj["width"].number(), 5.0));
}

TEST_CASE(TestJsonSerialize)
{
    Json a;
    a.deserialize("{\"width\":5}");

    TEST(a.contains("width"));
    TEST(a["width"].isNumber());
    TEST(isEqual(a["width"].number(), 5.0));

    Json b;
    b.deserialize(a.serialize());

    TEST(b.contains("width"));
    TEST(b["width"].isNumber());
    TEST(isEqual(b["width"].number(), 5.0));
}
