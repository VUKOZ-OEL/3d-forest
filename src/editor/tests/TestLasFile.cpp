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

/** @file TestLasFile.cpp */

// Include std.
#include <cstring>

// Include 3D Forest.
#include <Editor.hpp>
#include <IndexFileBuilder.hpp>
#include <LasFile.hpp>
#include <Test.hpp>
#include <Util.hpp>

#define TEST_LAS_FILE_PATH "test.las"

TEST_CASE(TestLasFileCreateV10)
{
    // Create new file with test data.
    std::vector<LasFile::Point> points;
    points.resize(3);
    std::memset(points.data(), 0, sizeof(LasFile::Point) * points.size());

    points[0].format = 0;
    points[0].x = 0;
    points[0].y = 0;
    points[0].z = 0;
    points[0].intensity = 0;
    points[0].classification = LasFile::CLASS_NEVER_CLASSIFIED;
    points[0].segment = 0;
    points[0].elevation = 0;
    points[0].descriptor = 0;

    points[1].format = 0;
    points[1].x = 200;
    points[1].y = 0;
    points[1].z = 10;
    points[1].intensity = 32767;
    points[1].classification = LasFile::CLASS_GROUND;
    points[1].segment = 0;
    points[1].elevation = 0;
    points[1].descriptor = 0;

    points[2].format = 0;
    points[2].x = 0;
    points[2].y = 300;
    points[2].z = 100;
    points[2].intensity = 65535;
    points[2].classification = LasFile::CLASS_LOW_VEGETATION;
    points[2].segment = 1;
    points[2].elevation = 90;
    points[2].descriptor = 0.25;

    LasFile::create(TEST_LAS_FILE_PATH, points, {1, 1, 1}, {0, 0, 0}, 0);

    // Create LAS file index.
    SettingsImport settings;
    IndexFileBuilder::index(TEST_LAS_FILE_PATH, TEST_LAS_FILE_PATH, settings);

    // Read the test file.
    // Expected : obtain the same values which were used to create the file.
    {
        Editor db;
        db.open(TEST_LAS_FILE_PATH);

        Query query(&db);
        query.where().setBox(Box<double>(-500., 500.));
        query.exec();

        TEST(query.next() && query.classification() == 0);
        TEST(query.next() && query.classification() == 2);
        TEST(query.next() && query.classification() == 3);
    }

    // Modify the test file.
    // Expected : it is possible to modify the file.
    {
        Editor db;
        db.open(TEST_LAS_FILE_PATH);

        Query query(&db);
        query.where().setBox(Box<double>(-500., 500.));
        query.exec();

        TEST(query.next() && (query.classification() = 6));
        TEST(query.next());
        TEST(query.next() && (query.classification() = 5));

        query.setModified();
        query.flush();
    }

    // Read modified values.
    // Expected : it is possible to update the file with new values.
    {
        Editor db;
        db.open(TEST_LAS_FILE_PATH);

        Query query(&db);
        query.where().setBox(Box<double>(-500., 500.));
        query.exec();

        TEST(query.next() && query.classification() == 6);
        TEST(query.next() && query.classification() == 2);
        TEST(query.next() && query.classification() == 5);
    }
}
