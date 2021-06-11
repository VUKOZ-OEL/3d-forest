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

/**
    @file EditorTile.cpp
*/

#include <EditorBase.hpp>
#include <EditorTile.hpp>
#include <File.hpp>
#include <FileLas.hpp>
#include <LasIndexBuilder.hpp>

EditorTile::EditorTile()
    : dataSetId(0),
      tileId(0),
      loaded(false),
      modified(false)
{
}

EditorTile::~EditorTile()
{
}

EditorTile::View::View() : renderStep(1), renderStepCount(1)
{
}

EditorTile::View::~View()
{
}

void EditorTile::View::resetFrame()
{
    renderStep = 1;
}

void EditorTile::View::nextFrame()
{
    renderStep++;
}

bool EditorTile::View::isStarted() const
{
    return renderStep == 1;
}

bool EditorTile::View::isFinished() const
{
    return renderStep > renderStepCount;
}

void EditorTile::read(const EditorBase *editor)
{
    const EditorDataSet &dataSet = editor->dataSet(dataSetId);
    const OctreeIndex::Node *node = dataSet.index.at(tileId);

    // Read tile buffer from LAS file
    FileLas las;
    las.open(dataSet.path);
    las.readHeader();

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    std::vector<uint8_t> buffer;
    size_t n = static_cast<size_t>(node->size);
    size_t bufferSize = pointSize * n;
    uint8_t fmt = las.header.point_data_record_format;
    buffer.resize(bufferSize);
    las.file().read(buffer.data(), bufferSize);

    // Create point data
    xyz.resize(n * 3);
    view.xyz.resize(n * 3);
    bool rgbFlag = las.header.hasRgb();
    if (rgbFlag)
    {
        rgb.resize(n * 3);
    }
    rgbOutput.resize(n * 3);
    indices.resize(n);

    // Covert buffer to point data
    uint8_t *ptr = buffer.data();
    FileLas::Point point;
    double x;
    double y;
    double z;
    const float scaleU16 = 1.0F / 65535.0F;

    for (size_t i = 0; i < n; i++)
    {
        indices[i] = static_cast<unsigned int>(i);

        las.readPoint(point, ptr + (pointSize * i), fmt);
        las.transform(x, y, z, point);

        xyz[3 * i + 0] = x;
        xyz[3 * i + 1] = y;
        xyz[3 * i + 2] = z;

        x = static_cast<double>(point.x) + las.header.x_offset;
        y = static_cast<double>(point.y) + las.header.y_offset;
        z = static_cast<double>(point.z) + las.header.z_offset;
        view.xyz[3 * i + 0] = static_cast<float>(x);
        view.xyz[3 * i + 1] = static_cast<float>(y);
        view.xyz[3 * i + 2] = static_cast<float>(z);

        if (rgbFlag)
        {
            rgb[3 * i + 0] = point.red * scaleU16;
            rgb[3 * i + 1] = point.green * scaleU16;
            rgb[3 * i + 2] = point.blue * scaleU16;
        }

        rgbOutput[3 * i + 0] = point.user_red * scaleU16;
        rgbOutput[3 * i + 1] = point.user_green * scaleU16;
        rgbOutput[3 * i + 2] = point.user_blue * scaleU16;
    }

    view.rgb = rgb;

    boundary.set(xyz);
    view.boundary.set(view.xyz);

    // Apply
    readFilter(editor);

    // Loaded
    loaded = true;
}

void EditorTile::readFilter(const EditorBase *editor)
{
    const EditorDataSet &dataSet = editor->dataSet(dataSetId);
    const OctreeIndex::Node *node = dataSet.index.at(tileId);

    if (editor->clipFilter().enabled)
    {
        // Read L2 index
        const std::string path = LasIndexBuilder::extensionL2(dataSet.path);
        index.read(path, node->reserved);

        // Select octants
        std::vector<OctreeIndex::Selection> selection;
        Aabb<double> clipBox = editor->clipFilter().box;
        index.selectLeaves(selection, clipBox, dataSet.id);

        // Compute upper limit of the number of selected points
        size_t nSelected = 0;

        for (size_t i = 0; i < selection.size(); i++)
        {
            const OctreeIndex::Node *nodeL2 = index.at(selection[i].idx);
            if (!nodeL2)
            {
                continue;
            }

            nSelected += static_cast<size_t>(nodeL2->size);
        }

        indices.resize(nSelected);

        // Select points
        nSelected = 0;

        for (size_t i = 0; i < selection.size(); i++)
        {
            const OctreeIndex::Node *nodeL2 = index.at(selection[i].idx);
            if (!nodeL2)
            {
                continue;
            }

            unsigned int nPoints = static_cast<unsigned int>(nodeL2->size);
            unsigned int from = static_cast<unsigned int>(nodeL2->from);

            if (selection[i].partial)
            {
                // Partial selection, apply clip filter
                for (unsigned int j = 0; j < nPoints; j++)
                {
                    unsigned int idx = from + j;
                    double x = xyz[3 * idx + 0];
                    double y = xyz[3 * idx + 1];
                    double z = xyz[3 * idx + 2];

                    if (clipBox.isInside(x, y, z))
                    {
                        indices[nSelected++] = idx;
                    }
                }
            }
            else
            {
                // Everything
                for (unsigned int j = 0; j < nPoints; j++)
                {
                    indices[nSelected++] = from + j;
                }
            }
        }

        indices.resize(nSelected);
    }
}
