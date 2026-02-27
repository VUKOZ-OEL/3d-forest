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
#include <Box.hpp>
#include <Json.hpp>
#include <Mesh.hpp>
#include <Range.hpp>
#include <Region.hpp>
#include <Test.hpp>
#include <Util.hpp>
#include <Vector2.hpp>
#include <Vector3.hpp>

TEST_CASE(TestJsonString)
{
    Json obj;
    obj["name"] = "John";

    TEST(obj.contains("name"));
    TEST(obj["name"].typeString());
    TEST(obj["name"].string() == "John");
}

TEST_CASE(TestJsonStringOpt)
{
    Json obj;
    obj["name"] = "John";

    std::string a;
    fromJson(a, obj, "name");
    TEST(a == "John");

    std::string b;
    fromJson(b, obj, "surname");
    TEST(b.empty());

    std::string c;
    fromJson(c, obj, "surname", "Doe");
    TEST(c == "Doe");
}

TEST_CASE(TestJsonNumber)
{
    Json obj;
    obj["width"] = 5;

    TEST(obj.contains("width"));
    TEST(obj["width"].typeNumber());
    TEST(equal(obj["width"].number(), 5.0));
}

TEST_CASE(TestJsonNumberOpt)
{
    Json obj;
    obj["width"] = 5;

    double a;
    fromJson(a, obj, "width");
    TEST(equal(a, 5.0));

    double b;
    fromJson(b, obj, "height");
    TEST(equal(b, 0.0));

    double c;
    fromJson(c, obj, "height", 3.14);
    TEST(equal(c, 3.14));
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

TEST_CASE(TestJsonVectorDoubleOpt)
{
    std::vector<double> in{0, 1.5, 3.14, 7.9};

    Json obj;
    obj["vec"] = in;

    std::vector<double> a;
    fromJson(a, obj, "vec");
    TEST(a.size() == in.size());
    TEST(equal(a, in));

    std::vector<double> b;
    fromJson(b, obj, "foo");
    TEST(equal(b, {}));

    std::vector<double> c;
    fromJson(c, obj, "foo", {1.2, 0.5});
    TEST(equal(c, {1.2, 0.5}));
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

TEST_CASE(TestJsonVector2)
{
    Vector2<double> in{0, 1.5};

    Json obj;
    toJson(obj["vec"], in);

    Vector2<double> out;
    fromJson(out, obj["vec"]);
    TEST(out == in);

    Vector2<double> a;
    fromJson(a, obj, "vec");
    TEST(a == in);

    Vector2<double> b;
    fromJson(b, obj, "foo");
    TEST(b == Vector2<double>());

    Vector2<double> c;
    fromJson(c, obj, "foo", {1.2, 0.5});
    TEST(c == Vector2<double>(1.2, 0.5));
}

TEST_CASE(TestJsonVector3)
{
    Vector3<double> in{0, 1.5, 3.14};

    Json obj;
    toJson(obj["vec"], in);

    Vector3<double> out;
    fromJson(out, obj["vec"]);
    TEST(out == in);

    Vector3<double> a;
    fromJson(a, obj, "vec");
    TEST(a == in);

    Vector3<double> b;
    fromJson(b, obj, "foo");
    TEST(b == Vector3<double>());

    Vector3<double> c;
    fromJson(c, obj, "foo", {1.2, 0.5, 2.0});
    TEST(c == Vector3<double>(1.2, 0.5, 2.0));
}

TEST_CASE(TestJsonBox)
{
    Box<double> in(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

    Json obj;
    toJson(obj["box"], in);

    Box<double> out;
    fromJson(out, obj["box"]);
    TEST(out == in);

    Box<double> a;
    fromJson(a, obj, "box");
    TEST(a == in);

    Box<double> b;
    fromJson(b, obj, "foo");
    TEST(b == Box<double>());

    Box<double> c;
    fromJson(c, obj, "foo", Box<double>(0.5, 1.5));
    TEST(c == Box<double>(0.5, 1.5));
}

TEST_CASE(TestJsonMesh)
{
    Mesh in;
    in.name = "hull";
    in.mode = Mesh::Mode::MODE_POINTS;

    Json obj;
    toJson(obj["mesh"], in);

    Mesh out;
    fromJson(out, obj["mesh"]);
    TEST(out.name == in.name);
    TEST(out.mode == in.mode);
}

TEST_CASE(TestJsonRange)
{
    Range<double> in(0.0, 0.1, 0.8, 1.0);

    Json obj;
    toJson(obj["range"], in);

    Range<double> out;
    fromJson(out, obj["range"]);
    TEST(out == in);

    Range<double> a;
    fromJson(a, obj, "range");
    TEST(a == in);

    Range<double> b;
    fromJson(b, obj, "foo");
    TEST(b == Range<double>());

    Range<double> c;
    fromJson(c, obj, "foo", Range<double>(0.5, 1.5));
    TEST(c == Range<double>(0.5, 1.5));
}

TEST_CASE(TestJsonRegion)
{
    Region in;
    in.shape = Region::Shape::BOX;
    in.box.set(0.0, 1.0);

    Json obj;
    toJson(obj["region"], in);

    Region out;
    fromJson(out, obj["region"]);
    TEST(out == in);

    Region a;
    fromJson(a, obj, "region");
    TEST(a == in);

    Region b;
    fromJson(b, obj, "foo");
    TEST(b == Region());

    Region c;
    fromJson(c, obj, "foo", in);
    TEST(c == in);
}
