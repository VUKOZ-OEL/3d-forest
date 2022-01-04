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

/** @file EditorPage.cpp */

#include <ColorPalette.hpp>
#include <EditorDatabase.hpp>
#include <EditorDataset.hpp>
#include <EditorPage.hpp>
#include <EditorQuery.hpp>
#include <Endian.hpp>
#include <File.hpp>
#include <FileIndexBuilder.hpp>
#include <FileLas.hpp>
#include <Log.hpp>

EditorPage::EditorPage(EditorDatabase *editor,
                       EditorQuery *query,
                       uint32_t datasetId,
                       uint32_t pageId)
    : editor_(editor),
      query_(query),
      datasetId_(datasetId),
      pageId_(pageId),
      loaded(false),
      transformed(false),
      selected(false),
      rendered(false),
      modified(false)
{
}

EditorPage::~EditorPage()
{
}

void EditorPage::clear()
{
    points.clear();

    renderPosition.clear();
    renderColor.clear();

    selection.clear();
    box.clear();
    octree.clear();
}

void EditorPage::resize(size_t n)
{
    points.resize(n);

    renderPosition.resize(n * 3);
    renderColor.resize(n * 3);

    selection.resize(n);

    positionBase_.resize(n * 3);
}

void EditorPage::read()
{
    const EditorDataset &dataset = editor_->datasets().key(datasetId_);
    const FileIndex::Node *node = dataset.index().at(pageId_);

    // Read page buffer from LAS file
    FileLas las;
    las.open(dataset.path());
    las.readHeader();

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    size_t nPagePoints = static_cast<size_t>(node->size);
    size_t bufferSize = pointSize * nPagePoints;
    uint8_t fmt = las.header.point_data_record_format;
    buffer_.resize(bufferSize);
    las.file().read(buffer_.data(), bufferSize);

    // Create point data
    resize(nPagePoints);

    // Covert buffer to point data
    uint8_t *ptr = buffer_.data();
    FileLas::Point point;
    const float scaleU16 = 1.0F / 65535.0F;
    bool rgbFlag = las.header.hasRgb();

    for (size_t i = 0; i < nPagePoints; i++)
    {
        selection[i] = static_cast<uint32_t>(i);

        las.readPoint(point, ptr + (pointSize * i), fmt);

        // xyz
        positionBase_[3 * i + 0] = static_cast<double>(point.x);
        positionBase_[3 * i + 1] = static_cast<double>(point.y);
        positionBase_[3 * i + 2] = static_cast<double>(point.z);

        EditorPoint &pt = points[i];
        pt.x = positionBase_[3 * i + 0];
        pt.y = positionBase_[3 * i + 1];
        pt.z = positionBase_[3 * i + 2];

        // intensity and color
        pt.intensity = static_cast<float>(point.intensity) * scaleU16;

        if (rgbFlag)
        {
            pt.red = point.red * scaleU16;
            pt.green = point.green * scaleU16;
            pt.blue = point.blue * scaleU16;
        }
        else
        {
            pt.red = 1.0F;
            pt.green = 1.0F;
            pt.blue = 1.0F;
        }

        pt.userRed = point.user_red * scaleU16;
        pt.userGreen = point.user_green * scaleU16;
        pt.userBlue = point.user_blue * scaleU16;

        // Attributes
        pt.returnNumber = point.return_number;
        pt.numberOfReturns = point.number_of_returns;
        pt.classification = point.classification;
        pt.userData = point.user_data;

        // GPS
        pt.gpsTime = point.gps_time;

        // Layer
        pt.layer = point.user_layer;
    }

    // Index
    std::string pathIndex;
    pathIndex = FileIndexBuilder::extension(dataset.path());
    octree.read(pathIndex, node->offset);
    octree.translate(dataset.translation());

    // Loaded
    loaded = true;
    transformed = false;
    selected = false;
    rendered = false;
    modified = false;

    // Apply
    transform();
    select();
}

#define EDITOR_PAGE_FORMAT_COUNT 11

static const size_t EDITOR_PAGE_FORMAT_USER[EDITOR_PAGE_FORMAT_COUNT] =
    {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};

void EditorPage::toPoint(uint8_t *ptr, size_t i, uint8_t fmt)
{
    const float s16 = 65535.0F;
    size_t pos = EDITOR_PAGE_FORMAT_USER[fmt];

    // Layer
    htol32(ptr + pos, points[i].layer);

    // User color
    htol16(ptr + pos + 4, static_cast<uint16_t>(renderColor[3 * i + 0] * s16));
    htol16(ptr + pos + 6, static_cast<uint16_t>(renderColor[3 * i + 1] * s16));
    htol16(ptr + pos + 8, static_cast<uint16_t>(renderColor[3 * i + 2] * s16));
}

void EditorPage::write()
{
    const EditorDataset &dataset = editor_->datasets().key(datasetId_);
    const FileIndex::Node *node = dataset.index().at(pageId_);

    FileLas las;
    las.open(dataset.path());
    las.readHeader();

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    size_t n = static_cast<size_t>(node->size);
    uint8_t fmt = las.header.point_data_record_format;

    uint8_t *ptr = buffer_.data();

    for (size_t i = 0; i < n; i++)
    {
        toPoint(ptr + (pointSize * i), i, fmt);
    }

    las.file().write(buffer_.data(), buffer_.size());

    modified = false;
}

void EditorPage::transform()
{
    const EditorDataset &dataset = editor_->datasets().key(datasetId_);
    size_t n = positionBase_.size() / 3;
    double x;
    double y;
    double z;
    double tx = dataset.translation()[0];
    double ty = dataset.translation()[1];
    double tz = dataset.translation()[2];

    for (size_t i = 0; i < n; i++)
    {
        x = positionBase_[3 * i + 0] + tx;
        y = positionBase_[3 * i + 1] + ty;
        z = positionBase_[3 * i + 2] + tz;

        renderPosition[3 * i + 0] = static_cast<float>(x);
        renderPosition[3 * i + 1] = static_cast<float>(y);
        renderPosition[3 * i + 2] = static_cast<float>(z);

        EditorPoint &pt = points[i];
        pt.x = x;
        pt.y = y;
        pt.z = z;
    }

    if (n > 0)
    {
        double min_x = points[0].x;
        double min_y = points[0].y;
        double min_z = points[0].z;
        double max_x = points[0].x;
        double max_y = points[0].y;
        double max_z = points[0].z;

        for (size_t i = 1; i < n; i++)
        {
            if (points[i].x < min_x)
                min_x = points[i].x;
            else if (points[i].x > max_x)
                max_x = points[i].x;

            if (points[i].y < min_y)
                min_y = points[i].y;
            else if (points[i].y > max_y)
                max_y = points[i].y;

            if (points[i].z < min_z)
                min_z = points[i].z;
            else if (points[i].z > max_z)
                max_z = points[i].z;
        }

        box.set(min_x, min_y, min_z, max_x, max_y, max_z);
    }
    else
    {
        box.clear();
    }

    transformed = true;
}

void EditorPage::select()
{
    size_t n = points.size();
    selection.resize(n);
    for (size_t i = 0; i < n; i++)
    {
        selection[i] = static_cast<uint32_t>(i);
    }

    selectBox();
    selectClassification();
    selectLayer();
    selectColor();

    selected = true;
}

void EditorPage::setStateRead()
{
    loaded = false;
    transformed = false;
    selected = false;
    rendered = false;
    modified = false;
}

void EditorPage::setStateSelect()
{
    selected = false;
    rendered = false;
}

void EditorPage::setStateRender()
{
    rendered = false;
}

bool EditorPage::nextState()
{
    if (!loaded)
    {
        try
        {
            read();
        }
        catch (std::exception &e)
        {
            // std::cout << e.what() << "\n";
        }
        catch (...)
        {
            // std::cout << "unknown error\n";
        }

        editor_->applyFilters(this);

        return false;
    }

    if (!transformed)
    {
        transform();
        return false;
    }

    if (!selected)
    {
        select();
        editor_->applyFilters(this);
        return false;
    }

    if (!rendered)
    {
        return false;
    }

    return true;
}

bool EditorPage::nextStateRender()
{
    if (loaded && transformed && selected && !rendered)
    {
        rendered = true;
        return true;
    }

    return false;
}

void EditorPage::selectBox()
{
    const Box<double> &clipBox = query_->selectedBox();
    if (clipBox.empty())
    {
        return;
    }

    // Select octants
    std::vector<FileIndex::Selection> selectedNodes;
    octree.selectLeaves(selectedNodes, clipBox, datasetId_);

    // Compute upper limit of the number of selected points
    size_t nSelected = 0;

    for (size_t i = 0; i < selectedNodes.size(); i++)
    {
        const FileIndex::Node *nodeL2 = octree.at(selectedNodes[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        nSelected += static_cast<size_t>(nodeL2->size);
    }

    selection.resize(nSelected);

    // Select points
    nSelected = 0;

    for (size_t i = 0; i < selectedNodes.size(); i++)
    {
        const FileIndex::Node *nodeL2 = octree.at(selectedNodes[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        uint32_t nNodePoints = static_cast<uint32_t>(nodeL2->size);
        uint32_t from = static_cast<uint32_t>(nodeL2->from);

        if (selectedNodes[i].partial)
        {
            // Partial selection, apply clip filter
            for (uint32_t j = 0; j < nNodePoints; j++)
            {
                uint32_t idx = from + j;
                double x = points[idx].x;
                double y = points[idx].y;
                double z = points[idx].z;

                if (clipBox.isInside(x, y, z))
                {
                    selection[nSelected++] = idx;
                }
            }
        }
        else
        {
            // Everything
            for (uint32_t j = 0; j < nNodePoints; j++)
            {
                selection[nSelected++] = from + j;
            }
        }
    }

    selection.resize(nSelected);
}

void EditorPage::selectClassification()
{
    const EditorClassifications &classifications = editor_->classifications();

    if (!classifications.isEnabled())
    {
        return;
    }

    size_t nSelected = selection.size();
    size_t nSelectedNew = 0;

    for (size_t i = 0; i < nSelected; i++)
    {
        uint32_t idx = selection[i];

        if (classifications.isEnabled(points[idx].classification))
        {
            if (nSelectedNew != i)
            {
                selection[nSelectedNew] = selection[i];
            }
            nSelectedNew++;
        }
    }

    selection.resize(nSelectedNew);
}

void EditorPage::selectLayer()
{
    const EditorLayers &layers = editor_->layers();

    if (!layers.isEnabled())
    {
        return;
    }

    size_t nSelected = selection.size();
    size_t nSelectedNew = 0;

    for (size_t i = 0; i < nSelected; i++)
    {
        uint32_t idx = selection[i];

        if (layers.isEnabledId(points[idx].layer))
        {
            if (nSelectedNew != i)
            {
                selection[nSelectedNew] = selection[i];
            }
            nSelectedNew++;
        }
    }

    selection.resize(nSelectedNew);
}

void EditorPage::selectColor()
{
    const EditorSettingsView &opt = editor_->settings().view();
    float r = opt.pointColor()[0];
    float g = opt.pointColor()[1];
    float b = opt.pointColor()[2];

    size_t n = points.size();

    for (size_t i = 0; i < n; i++)
    {
        renderColor[i * 3 + 0] = r;
        renderColor[i * 3 + 1] = g;
        renderColor[i * 3 + 2] = b;
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_COLOR))
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] *= points[i].red;
            renderColor[i * 3 + 1] *= points[i].green;
            renderColor[i * 3 + 2] *= points[i].blue;
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_USER_COLOR))
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] *= points[i].userRed;
            renderColor[i * 3 + 1] *= points[i].userGreen;
            renderColor[i * 3 + 2] *= points[i].userBlue;
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
                     static_cast<size_t>(points[i].intensity * 255.0F),
                     255,
                     ColorPalette::BlueCyanYellowRed256);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_RETURN_NUMBER))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     points[i].returnNumber,
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_NUMBER_OF_RETURNS))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     points[i].numberOfReturns,
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_CLASSIFICATION))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     points[i].classification,
                     15,
                     ColorPalette::Classification);
        }
    }
}

void EditorPage::setColor(size_t idx,
                          size_t value,
                          size_t max,
                          const std::vector<Vector3<float>> &pal)
{
    if (value > max)
    {
        value = max;
    }

    renderColor[idx * 3 + 0] *= pal[value][0];
    renderColor[idx * 3 + 1] *= pal[value][1];
    renderColor[idx * 3 + 2] *= pal[value][2];
}