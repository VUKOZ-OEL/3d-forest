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

/** @file exampleQuery.cpp @brief Query example. */

#include <cstring>

#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <Time.hpp>

#define SCALE 0.001
#define DATA_PATH "exampledataset.las"

// Use the same query to issue multiple data requests.
static void exampleQuery()
{
    Editor db;
    db.open(DATA_PATH);

    Query query(&db);

    // Set all 3 points to layer 0
    // Output:
    //     0, 0, 0 Layer 0 Start
    //     1, 0, 0 Layer 0 Start
    //     0, 1, 0 Layer 0 Start
    query.selectBox(db.clipBoundary());
    query.exec();
    while (query.next())
    {
        query.layer() = 0;   // Update
        query.setModified(); // Mark update
        std::cout << query.x() << ", " << query.y() << ", " << query.z()
                  << " Layer " << query.layer() << " Start" << std::endl;
    }

    // Set first point to layer 1 by using extra region select
    // Output:
    //     0, 0, 0 Layer 1 A
    query.selectBox(Box<double>(-0.5, -0.5, -0.5, 0.5, 0.5, 0.5));
    query.exec();
    while (query.next())
    {
        query.layer() = 1;   // Update
        query.setModified(); // Mark update
        std::cout << query.x() << ", " << query.y() << ", " << query.z()
                  << " Layer " << query.layer() << " A" << std::endl;
    }

    // Set second point to layer 2 by using extra region select
    // Output:
    //     1, 0, 0 Layer 2 B
    query.selectBox(Box<double>(0.5, -0.5, -0.5, 1.5, 0.5, 0.5));
    query.exec();
    while (query.next())
    {
        query.layer() = 2;   // Update
        query.setModified(); // Mark update
        std::cout << query.x() << ", " << query.y() << ", " << query.z()
                  << " Layer " << query.layer() << " B" << std::endl;
    }

    // Skip the third point

    // Flush query cache
    query.flush();

    // Print layers of all points
    // Output:
    //     0, 0, 0 Layer 1 End
    //     1, 0, 0 Layer 2 End
    //     0, 1, 0 Layer 0 End
    query.selectBox(db.clipBoundary());
    query.exec();
    while (query.next())
    {
        std::cout << query.x() << ", " << query.y() << ", " << query.z()
                  << " Layer " << query.layer() << " End" << std::endl;
    }
}

static void createDataSet()
{
    // Points
    std::vector<LasFile::Point> points;

    points.resize(3);
    std::memset(points.data(), 0, sizeof(LasFile::Point) * points.size());

    points[0].x = 0;
    points[0].y = 0;
    points[0].z = 0;

    points[1].x = 1;
    points[1].y = 0;
    points[1].z = 0;

    points[2].x = 0;
    points[2].y = 1;
    points[2].z = 0;

    LasFile::create(DATA_PATH, points, {SCALE, SCALE, SCALE}, {0, 0, 0});

    // Index
    IndexFileBuilder::Settings settings;
    settings.maxSize1 = 1;

    IndexFileBuilder::index(DATA_PATH, DATA_PATH, settings);
}

int main()
{
    try
    {
        createDataSet();
        exampleQuery();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
