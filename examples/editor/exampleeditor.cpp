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

/** @file exampleeditor.cpp @brief Editor example. */

#include <EditorDatabase.hpp>
#include <FileIndexBuilder.hpp>
#include <Time.hpp>
#include <Error.hpp>
#include <cstring>

#define PATH_1 "dataset1.las"

static void createDataSet()
{
    FileIndexBuilder::Settings settings;
    settings.maxSize1 = 2;

    std::vector<FileLas::Point> points;

    points.resize(3);
    std::memset(points.data(), 0, sizeof(FileLas::Point) * points.size());

    points[0].x = 0;
    points[0].y = 0;
    points[0].z = 0;

    points[1].x = 1;
    points[1].y = 1;
    points[1].z = 0;

    points[2].x = 0;
    points[2].y = 1;
    points[2].z = 0;

    FileLas::create(PATH_1, points, {1,1,1}, {0,0,0});
    FileIndexBuilder::index(PATH_1, PATH_1, settings);
}

static void edit()
{
    EditorDatabase db;
    db.openDataset(PATH_1);

    double zmin = db.clipBoundary().max(2);

    EditorQuery query(&db);
    query.selectBox(Box<double>(0., 0., 0., 1.0, 1.0, 2.));
    query.exec();

    while (query.nextPoint())
    {
        if (query.z() < zmin)
        {
            zmin = query.z();
        }
    }

    std::cout << "z min is " << zmin << std::endl;
}

int main()
{
    try
    {
        createDataSet();
        edit();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
