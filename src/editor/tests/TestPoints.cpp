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

/** @file TestPoints.cpp */

// Include 3D Forest.
#include <Points.hpp>
#include <Test.hpp>
#include <Util.hpp>

TEST_CASE(TestPointsCreate)
{
    Points p;
    const Points &pc = p;

    p.push_back({1.0, 0.0, 0.0});

    TEST(pc.size() == 1);
    TEST(isEqual(pc[0].x, 1.0));

    std::vector<size_t> result;
    p.createIndex();
    p.findRadius(0.0, 0.0, 0.0, 1.1, result);
    TEST(result.size() == 1 && isEqual(pc[result[0]].x, 1.0));
}

TEST_CASE(TestPointsFindNN)
{
    Points p;
    p.push_back({0.0, 0.0, 0.0});
    p.createIndex();

    TEST(p.findNN(0.0, 0.0, 1.0) == 0);
}

class TestPoint3f
{
public:
    typedef float Type;
    TestPoint3f(Type x_, Type y_, Type z_) : x(x_), y(y_), z(z_) {}
    Type x, y, z;
};

TEST_CASE(TestPointsOctreeRadiusFloat)
{
    std::vector<TestPoint3f> points;
    points.push_back({0.0F, 0.0F, 0.0F});
    points.push_back({0.3F, 0.0F, 0.0F});
    points.push_back({0.1F, 0.0F, 0.0F});

    unibn::Octree<TestPoint3f> octree;
    octree.initialize(points);

    std::vector<size_t> results;
    octree.radiusNeighbors<unibn::L2Distance<TestPoint3f>>({0.0F, 0.0F, 0.0F},
                                                           0.2F,
                                                           results);
    TEST(results.size() == 2 && isEqual(points[results[0]].x, 0.0F));
    TEST(results.size() == 2 && isEqual(points[results[1]].x, 0.1F));
}

class TestPoint3d
{
public:
    typedef double Type;
    TestPoint3d(Type x_, Type y_, Type z_) : x(x_), y(y_), z(z_) {}
    Type x, y, z;
};

TEST_CASE(TestPointsOctreeRadiusDouble)
{
    std::vector<TestPoint3d> points;
    points.push_back({0.0, 0.0, 0.0});
    points.push_back({0.3, 0.0, 0.0});
    points.push_back({0.1, 0.0, 0.0});

    unibn::Octree<TestPoint3d> octree;
    octree.initialize(points);

    std::vector<size_t> results;
    octree.radiusNeighbors<unibn::L2Distance<TestPoint3d>>({0.0, 0.0, 0.0},
                                                           0.2,
                                                           results);
    TEST(results.size() == 2 && isEqual(points[results[0]].x, 0.0));
    TEST(results.size() == 2 && isEqual(points[results[1]].x, 0.1));
}
