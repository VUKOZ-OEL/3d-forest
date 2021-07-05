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

/** @file EditorTile.cpp */

#include <ColorPalette.hpp>
#include <EditorBase.hpp>
#include <EditorTile.hpp>
#include <File.hpp>
#include <FileIndexBuilder.hpp>
#include <FileLas.hpp>

EditorTile::EditorTile()
    : dataSetId(0),
      tileId(0),
      loaded(false),
      transformed(false),
      filtered(false),
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
    const FileIndex::Node *node = dataSet.index.at(tileId);

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
    indices.resize(n);

    xyz.resize(n * 3);
    xyzBase.resize(n * 3);

    intensity.resize(n);
    rgb.resize(n * 3);
    rgbOutput.resize(n * 3);
    attrib.resize(n);
    gpsTime.resize(n);
    layer.resize(n);

    view.xyz.resize(n * 3);
    view.rgb.resize(n * 3);

    // Covert buffer to point data
    uint8_t *ptr = buffer.data();
    FileLas::Point point;
    const float scaleU16 =
        1.0F / 65535.0F; /**< @todo Normalize during conversion. */
    // const float scaleU16 = 1.0F / 255.0F;
    bool rgbFlag = las.header.hasRgb();

    for (size_t i = 0; i < n; i++)
    {
        indices[i] = static_cast<unsigned int>(i);

        las.readPoint(point, ptr + (pointSize * i), fmt);

        // xyz
        xyzBase[3 * i + 0] = static_cast<double>(point.x);
        xyzBase[3 * i + 1] = static_cast<double>(point.y);
        xyzBase[3 * i + 2] = static_cast<double>(point.z);

        // intensity and color
        intensity[i] = static_cast<float>(point.intensity) * scaleU16;

        if (rgbFlag)
        {
            rgb[3 * i + 0] = point.red * scaleU16;
            rgb[3 * i + 1] = point.green * scaleU16;
            rgb[3 * i + 2] = point.blue * scaleU16;
        }
        else
        {
            rgb[3 * i + 0] = 1.0F;
            rgb[3 * i + 1] = 1.0F;
            rgb[3 * i + 2] = 1.0F;
        }

        rgbOutput[3 * i + 0] = point.user_red * scaleU16;
        rgbOutput[3 * i + 1] = point.user_green * scaleU16;
        rgbOutput[3 * i + 2] = point.user_blue * scaleU16;

        // Attributes
        Attributes &attribute = attrib[i];
        attribute.returnNumber = point.return_number;
        attribute.numberOfReturns = point.number_of_returns;
        attribute.classification = point.classification;

        // GPS
        gpsTime[i] = point.gps_time;

        // Layer
        layer[i] = point.user_layer;
    }

    // Loaded
    loaded = true;

    // Apply
    transform(editor);
    filter(editor);
}

void EditorTile::transform(const EditorBase *editor)
{
    const EditorDataSet &dataSet = editor->dataSet(dataSetId);
    size_t n = xyzBase.size() / 3;
    double x;
    double y;
    double z;
    double tx = dataSet.translation[0];
    double ty = dataSet.translation[1];
    double tz = dataSet.translation[2];

    for (size_t i = 0; i < n; i++)
    {
        x = xyzBase[3 * i + 0] + tx;
        y = xyzBase[3 * i + 1] + ty;
        z = xyzBase[3 * i + 2] + tz;

        xyz[3 * i + 0] = x;
        xyz[3 * i + 1] = y;
        xyz[3 * i + 2] = z;

        view.xyz[3 * i + 0] = static_cast<float>(x);
        view.xyz[3 * i + 1] = static_cast<float>(y);
        view.xyz[3 * i + 2] = static_cast<float>(z);
    }

    boundary.set(xyz);
    view.boundary.set(view.xyz);

    transformed = true;
}

void EditorTile::filter(const EditorBase *editor)
{
    select(editor);
    setPointColor(editor);

    filtered = true;
}

bool EditorTile::renderMore() const
{
    return loaded && transformed && filtered && !view.isFinished();
}

void EditorTile::select(const EditorBase *editor)
{
    const EditorDataSet &dataSet = editor->dataSet(dataSetId);
    const FileIndex::Node *node = dataSet.index.at(tileId);

    if (editor->clipFilter().enabled)
    {
        // Read L2 index
        if (index.empty())
        {
            std::string pathIndex = FileIndexBuilder::extension(dataSet.path);
            index.read(pathIndex, node->offset);
            index.translate(dataSet.translation);
        }

        // Select octants
        std::vector<FileIndex::Selection> selection;
        Aabb<double> clipBox = editor->clipFilter().box;

        index.selectLeaves(selection, clipBox, dataSet.id);

        // Compute upper limit of the number of selected points
        size_t nSelected = 0;

        for (size_t i = 0; i < selection.size(); i++)
        {
            const FileIndex::Node *nodeL2 = index.at(selection[i].idx);
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
            const FileIndex::Node *nodeL2 = index.at(selection[i].idx);
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

void EditorTile::setPointColor(const EditorBase *editor)
{
    const EditorSettings::View &opt = editor->settings().view();
    float r = opt.pointColorRed();
    float g = opt.pointColorGreen();
    float b = opt.pointColorBlue();

    size_t n = intensity.size();

    for (size_t i = 0; i < n; i++)
    {
        view.rgb[i * 3 + 0] = r;
        view.rgb[i * 3 + 1] = g;
        view.rgb[i * 3 + 2] = b;
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_COLOR))
    {
        for (size_t i = 0; i < (n * 3); i++)
        {
            view.rgb[i] *= rgb[i];
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_INTENSITY))
    {
        // for (size_t i = 0; i < n; i++)
        // {
        //     view.rgb[i * 3 + 0] *= intensity[i];
        //     view.rgb[i * 3 + 1] *= intensity[i];
        //     view.rgb[i * 3 + 2] *= intensity[i];
        // }

        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     static_cast<size_t>(intensity[i] * 255.0F),
                     255,
                     ColorPalette::BlueCyanYellowRed256);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_RETURN_NUMBER))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     attrib[i].returnNumber,
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_NUMBER_OF_RETURNS))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     attrib[i].numberOfReturns,
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_CLASSIFICATION))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     attrib[i].classification,
                     15,
                     ColorPalette::Classification);
        }
    }
}

void EditorTile::setColor(size_t idx,
                          size_t value,
                          size_t max,
                          const std::vector<Vector3<float>> &pal)
{
    if (value > max)
    {
        value = max;
    }

    view.rgb[idx * 3 + 0] *= pal[value][0];
    view.rgb[idx * 3 + 1] *= pal[value][1];
    view.rgb[idx * 3 + 2] *= pal[value][2];
}
