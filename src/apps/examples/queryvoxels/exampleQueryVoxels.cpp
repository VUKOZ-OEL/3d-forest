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

/** @file exampleQueryVoxels.cpp @brief Query voxels example. */

#include <cstring>

#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <Time.hpp>

static void exampleQueryVoxels(const std::string &path, double voxelSize)
{
    std::unordered_set<size_t> classifications;
    classifications.insert(LasFile::Classification::CLASS_UNASSIGNED);

    Editor db;
    db.open(path);

    Query query(&db);
    query.setVoxels(voxelSize, db.clipBoundary());

    while (query.nextVoxel())
    {
        query.where().setBox(query.voxelBox());
        query.where().setClassification(classifications);
        query.exec();
        while (query.next())
        {
            // clang-format off
            std::cout << "voxel <" << query.numberOfVisitedVoxels()
                      << "> [" << query.x() << ", "
                               << query.y() << ", "
                               << query.z()
                      << "]" << std::endl;
            // clang-format on
        }
        query.where().setClassification({});
    }
}

static void setPoint(int32_t x,
                     int32_t y,
                     int32_t z,
                     LasFile::Classification classification,
                     LasFile::Point *point)
{
    point->x = x;
    point->y = y;
    point->z = z;
    point->classification = classification;
}

static void createDataSet(const std::string &path, double scale)
{
    // Points
    std::vector<LasFile::Point> points;

    points.resize(3);
    std::memset(points.data(), 0, sizeof(LasFile::Point) * points.size());

    setPoint(0, 0, 0, LasFile::Classification::CLASS_GROUND, &points[0]);
    setPoint(1, 1, 1, LasFile::Classification::CLASS_UNASSIGNED, &points[1]);
    setPoint(3, 3, 3, LasFile::Classification::CLASS_UNASSIGNED, &points[2]);

    LasFile::create(path, points, {scale, scale, scale}, {0, 0, 0});

    // Index
    IndexFileBuilder::Settings settings;
    settings.maxSize1 = 100;

    IndexFileBuilder::index(path, path, settings);
}

int main()
{
    try
    {
        std::string path = "exampledataset.las";

        createDataSet(path, 0.001);
        exampleQueryVoxels(path, 1.5);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
