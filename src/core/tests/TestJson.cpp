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
    TEST(obj["name"].typeString());
    TEST(obj["name"].string() == "John");
}

TEST_CASE(TestJsonNumber)
{
    Json obj;
    obj["width"] = 5;

    TEST(obj.contains("width"));
    TEST(obj["width"].typeNumber());
    TEST(equal(obj["width"].number(), 5.0));
}

TEST_CASE(TestJsonVectorInt)
{
    std::vector<int> in{0, 1, 3, -7};

    Json obj;
    obj["vec"] = in;

    TEST(obj.contains("vec"));
    TEST(obj["vec"].typeArray());
    TEST(equal(static_cast<int>(obj["vec"][0].number()), 0));
    TEST(equal(static_cast<int>(obj["vec"][1].number()), 1));
    TEST(equal(static_cast<int>(obj["vec"][2].number()), 3));
    TEST(equal(static_cast<int>(obj["vec"][3].number()), -7));

    std::vector<int> out;
    fromJson(out, obj["vec"]);

    TEST(out.size() == 4);
    TEST(equal(out[0], 0));
    TEST(equal(out[1], 1));
    TEST(equal(out[2], 3));
    TEST(equal(out[3], -7));
}

TEST_CASE(TestJsonVectorDouble)
{
    std::vector<double> in{0, 1.5, 3.14, 7.9};

    Json obj;
    obj["vec"] = in;

    TEST(obj.contains("vec"));
    TEST(obj["vec"].typeArray());
    TEST(equal(obj["vec"][0].number(), 0.0));
    TEST(equal(obj["vec"][1].number(), 1.5));
    TEST(equal(obj["vec"][2].number(), 3.14));
    TEST(equal(obj["vec"][3].number(), 7.9));

    std::vector<double> out;
    fromJson(out, obj["vec"]);

    TEST(out.size() == 4);
    TEST(equal(out[0], 0.0));
    TEST(equal(out[1], 1.5));
    TEST(equal(out[2], 3.14));
    TEST(equal(out[3], 7.9));
}

TEST_CASE(TestJsonVectorFloat)
{
    std::vector<float> in{0.0F, 1.5F, 3.14F, 7.9F};

    Json obj;
    obj["vec"] = in;

    TEST(obj.contains("vec"));
    TEST(obj["vec"].typeArray());
    TEST(equal(static_cast<float>(obj["vec"][0].number()), 0.0F));
    TEST(equal(static_cast<float>(obj["vec"][1].number()), 1.5F));
    TEST(equal(static_cast<float>(obj["vec"][2].number()), 3.14F));
    TEST(equal(static_cast<float>(obj["vec"][3].number()), 7.9F));

    std::vector<float> out;
    fromJson(out, obj["vec"]);

    TEST(out.size() == 4);
    TEST(equal(out[0], 0.0F));
    TEST(equal(out[1], 1.5F));
    TEST(equal(out[2], 3.14F));
    TEST(equal(out[3], 7.9F));
}

TEST_CASE(TestJsonSerialize)
{
    Json a;
    a.deserialize("{\"width\":5}");

    TEST(a.contains("width"));
    TEST(a["width"].typeNumber());
    TEST(equal(a["width"].number(), 5.0));

    Json b;
    b.deserialize(a.serialize());

    TEST(b.contains("width"));
    TEST(b["width"].typeNumber());
    TEST(equal(b["width"].number(), 5.0));
}
