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

/** @file exampleSegmentation.cpp @brief Segmentation example. */

#include <Editor.hpp>
#include <Error.hpp>
#include <SegmentationPca.hpp>
#include <SegmentationThread.hpp>

#define PATH "dataset.las"

static void exampleSegmentation()
{
    Editor editor;
    editor.open(PATH);

    SegmentationPca pca;
    SegmentationThread segmentationThread(&editor);

    int voxelSize = 1;
    int threshold = 50;

    segmentationThread.create();
    segmentationThread.start(voxelSize, threshold);
    segmentationThread.wait();
}

static void createDataset()
{
    // Create a dataset.
    std::vector<LasFile::Point> points;

    points.resize(3);
    std::memset(points.data(), 0, sizeof(LasFile::Point) * points.size());

    points[0].x = 0;
    points[0].y = 0;
    points[0].z = 0;

    points[1].x = 1;
    points[1].y = 0;
    points[1].z = 0;

    points[2].x = 2;
    points[2].y = 0;
    points[2].z = 0;

    LasFile::create(PATH, points, {1, 1, 1}, {0, 0, 0});

    // Create index file.
    IndexFileBuilder::Settings settings;
    settings.maxSize1 = 2;

    IndexFileBuilder::index(PATH, PATH, settings);
}

int main()
{
    try
    {
        createDataset();
        exampleSegmentation();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
