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

/** @file Page.cpp */

#include <ColorPalette.hpp>
#include <Dataset.hpp>
#include <Editor.hpp>
#include <Endian.hpp>
#include <File.hpp>
#include <IndexFileBuilder.hpp>
#include <LasFile.hpp>
#include <Log.hpp>
#include <Page.hpp>
#include <Query.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("Page", msg)

Page::Page(Editor *editor, Query *query, uint32_t datasetId, uint32_t pageId)
    : selectionSize(0),
      editor_(editor),
      query_(query),
      datasetId_(datasetId),
      pageId_(pageId),
      state_(Page::STATE_READ),
      modified_(false)
{
}

Page::~Page()
{
}

void Page::clear()
{
    position.clear();
    elevation.clear();
    intensity.clear();
    returnNumber.clear();
    numberOfReturns.clear();
    classification.clear();
    userData.clear();
    gpsTime.clear();
    color.clear();
    userColor.clear();
    layer.clear();

    renderPosition.clear();
    renderColor.clear();

    selection.clear();
    selectionSize = 0;

    box.clear();
    octree.clear();
}

void Page::resize(size_t n)
{
    position.resize(n * 3);
    elevation.resize(n);
    intensity.resize(n);
    returnNumber.resize(n);
    numberOfReturns.resize(n);
    classification.resize(n);
    userData.resize(n);
    gpsTime.resize(n);
    color.resize(n * 3);
    userColor.resize(n * 3);
    layer.resize(n);

    renderPosition.resize(n * 3);
    renderColor.resize(n * 3);

    selection.resize(n);
    selectionSize = n;

    positionBase_.resize(n * 3);

    selectedNodes_.reserve(64);
}

void Page::read()
{
    const Dataset &dataset = editor_->datasets().key(datasetId_);
    const IndexFile::Node *node = dataset.index().at(pageId_);

    // Read page buffer from LAS file
    LasFile las;
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
    LasFile::Point point;
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

        position[3 * i + 0] = positionBase_[3 * i + 0];
        position[3 * i + 1] = positionBase_[3 * i + 1];
        position[3 * i + 2] = positionBase_[3 * i + 2];

        // elevation
        elevation[i] = static_cast<double>(point.user_elevation);

        // intensity and color
        intensity[i] = static_cast<float>(point.intensity) * scaleU16;

        if (rgbFlag)
        {
            color[3 * i + 0] = point.red * scaleU16;
            color[3 * i + 1] = point.green * scaleU16;
            color[3 * i + 2] = point.blue * scaleU16;
        }
        else
        {
            color[3 * i + 0] = 1.0F;
            color[3 * i + 1] = 1.0F;
            color[3 * i + 2] = 1.0F;
        }

        userColor[3 * i + 0] = point.user_red * scaleU16;
        userColor[3 * i + 1] = point.user_green * scaleU16;
        userColor[3 * i + 2] = point.user_blue * scaleU16;

        // attributes
        returnNumber[i] = point.return_number;
        numberOfReturns[i] = point.number_of_returns;
        classification[i] = point.classification;
        userData[i] = point.user_data;

        // gps
        gpsTime[i] = point.gps_time;

        // layer
        layer[i] = point.user_layer;
    }

    // Index
    std::string pathIndex;
    pathIndex = IndexFileBuilder::extension(dataset.path());
    octree.read(pathIndex, node->offset);
    octree.translate(dataset.translation());

    // Loaded
    state_ = Page::STATE_TRANSFORM;
    modified_ = false;

    // Apply
    transform();
    select();
    runModifiers();
}

#define PAGE_FORMAT_COUNT 11

static const size_t PAGE_FORMAT_USER[PAGE_FORMAT_COUNT] =
    {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};

void Page::toPoint(uint8_t *ptr, size_t i, uint8_t fmt)
{
    const float s16 = 65535.0F;
    size_t pos = PAGE_FORMAT_USER[fmt];

    // Classification
    if (fmt > 5)
    {
        ptr[16] = classification[i];
    }

    // Layer
    htol32(ptr + pos, layer[i]);

    // User color
    htol16(ptr + pos + 4, static_cast<uint16_t>(renderColor[3 * i + 0] * s16));
    htol16(ptr + pos + 6, static_cast<uint16_t>(renderColor[3 * i + 1] * s16));
    htol16(ptr + pos + 8, static_cast<uint16_t>(renderColor[3 * i + 2] * s16));

    // User intensity

    // Elevation
    htol32(ptr + pos + 12, static_cast<uint32_t>(elevation[i]));
}

void Page::write()
{
    const Dataset &dataset = editor_->datasets().key(datasetId_);
    const IndexFile::Node *node = dataset.index().at(pageId_);

    LasFile las;
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

    modified_ = false;
}

void Page::transform()
{
    const Dataset &dataset = editor_->datasets().key(datasetId_);
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

        position[3 * i + 0] = x;
        position[3 * i + 1] = y;
        position[3 * i + 2] = z;
    }

    box.set(position);

    state_ = Page::STATE_SELECT;
}

void Page::select()
{
    LOG_DEBUG_LOCAL("");

    const Box<double> &clipBox = query_->selectedBox();
    const Cone<double> &clipCone = query_->selectedCone();
    const Sphere<double> &selectedSphere = query_->selectedSphere();


    if (clipBox.empty() && clipCone.empty() && selectedSphere.empty())
    {
        // Reset selection to mark all points as selected.
        LOG_DEBUG_LOCAL("Reset selection");
        uint32_t n = static_cast<uint32_t>(position.size() / 3);
        selection.resize(n);
        selectionSize = n;
        for (uint32_t i = 0; i < n; i++)
        {
            selection[i] = i;
        }
    }

    // Apply new selection.
    selectBox();
    selectCone();
    selectSphere();
    selectElevation();
    selectClassification();
    selectLayer();

    state_ = Page::STATE_RUN_MODIFIERS;
}

void Page::runModifiers()
{
    runColorModifier();
    editor_->runModifiers(this);

    state_ = Page::STATE_RENDER;
}

void Page::setModified()
{
    modified_ = true;
}

void Page::setState(Page::State state)
{
    if ((state < state_) || (state == Page::STATE_RENDERED))
    {
        state_ = state;
    }

    if (state == Page::STATE_READ)
    {
        modified_ = false;
    }
}

bool Page::nextState()
{
    if (state_ == Page::STATE_READ)
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

        return false;
    }

    if (state_ == Page::STATE_TRANSFORM)
    {
        transform();
        return false;
    }

    if (state_ == Page::STATE_SELECT)
    {
        select();
        return false;
    }

    if (state_ == Page::STATE_RUN_MODIFIERS)
    {
        runModifiers();
        return false;
    }

    if (state_ == Page::STATE_RENDER)
    {
        return false;
    }

    return true;
}

void Page::selectBox()
{
    const Box<double> &clipBox = query_->selectedBox();
    if (clipBox.empty())
    {
        return;
    }

    // Select octants
    selectedNodes_.resize(0);
    octree.selectLeaves(selectedNodes_, clipBox, datasetId_);

    // Compute upper limit of the number of selected points
    size_t nSelected = 0;

    for (size_t i = 0; i < selectedNodes_.size(); i++)
    {
        const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        nSelected += static_cast<size_t>(nodeL2->size);
    }

    selectionSize = nSelected;
    if (selection.size() < selectionSize)
    {
        selection.resize(selectionSize);
    }

    // Select points
    nSelected = 0;

    size_t max = query_->maximumResults();

    if (max == 0)
    {
        // Unlimited number of results
        for (size_t i = 0; i < selectedNodes_.size(); i++)
        {
            const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
            if (!nodeL2)
            {
                continue;
            }

            uint32_t nNodePoints = static_cast<uint32_t>(nodeL2->size);
            uint32_t from = static_cast<uint32_t>(nodeL2->from);

            if (selectedNodes_[i].partial)
            {
                // Partial selection, apply clip filter
                for (uint32_t j = 0; j < nNodePoints; j++)
                {
                    uint32_t idx = from + j;
                    double x = position[3 * idx + 0];
                    double y = position[3 * idx + 1];
                    double z = position[3 * idx + 2];

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
    }
    else
    {
        // Limited number of results
        max = max - query_->resultSize();
        bool maxReached = false;

        for (size_t i = 0; i < selectedNodes_.size(); i++)
        {
            const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
            if (!nodeL2)
            {
                continue;
            }

            uint32_t nNodePoints = static_cast<uint32_t>(nodeL2->size);
            uint32_t from = static_cast<uint32_t>(nodeL2->from);

            if (selectedNodes_[i].partial)
            {
                // Partial selection, apply clip filter
                for (uint32_t j = 0; j < nNodePoints; j++)
                {
                    uint32_t idx = from + j;
                    double x = position[3 * idx + 0];
                    double y = position[3 * idx + 1];
                    double z = position[3 * idx + 2];

                    if (clipBox.isInside(x, y, z))
                    {
                        selection[nSelected++] = idx;
                        if (nSelected == max)
                        {
                            maxReached = true;
                            break;
                        }
                    }
                }
            }
            else
            {
                // Everything
                if (nNodePoints > max)
                {
                    nNodePoints = static_cast<uint32_t>(max);
                    maxReached = true;
                }

                for (uint32_t j = 0; j < nNodePoints; j++)
                {
                    selection[nSelected++] = from + j;
                }
            }

            if (maxReached)
            {
                break;
            }
        }

        query_->addResults(nSelected);
    }

    selectionSize = nSelected;
}

void Page::selectCone()
{
    const Cone<double> &clipCone = query_->selectedCone();
    if (clipCone.empty())
    {
        return;
    }

    // Select octants
    selectedNodes_.resize(0);
    octree.selectLeaves(selectedNodes_, clipCone.box(), datasetId_);

    // Compute upper limit of the number of selected points
    size_t nSelected = 0;

    for (size_t i = 0; i < selectedNodes_.size(); i++)
    {
        const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        nSelected += static_cast<size_t>(nodeL2->size);
    }

    selectionSize = nSelected;
    if (selection.size() < selectionSize)
    {
        selection.resize(selectionSize);
    }

    // Select points
    nSelected = 0;

    size_t max = query_->maximumResults() - query_->resultSize();
    bool maxReached = false;

    for (size_t i = 0; i < selectedNodes_.size(); i++)
    {
        const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        uint32_t nNodePoints = static_cast<uint32_t>(nodeL2->size);
        uint32_t from = static_cast<uint32_t>(nodeL2->from);

        // Partial/Whole selection, apply clip filter
        for (uint32_t j = 0; j < nNodePoints; j++)
        {
            uint32_t idx = from + j;
            double x = position[3 * idx + 0];
            double y = position[3 * idx + 1];
            double z = position[3 * idx + 2];

            if (clipCone.isInside(x, y, z))
            {
                selection[nSelected++] = idx;
                if (nSelected == max)
                {
                    maxReached = true;
                    break;
                }
            }
        }

        if (maxReached)
        {
            break;
        }
    }

    selectionSize = nSelected;

    query_->addResults(nSelected);
}

void Page::selectSphere()
{
    const Sphere<double> &selectedSphere = query_->selectedSphere();
    if (selectedSphere.empty())
    {
        return;
    }

    // Select octants
    selectedNodes_.resize(0);
    octree.selectLeaves(selectedNodes_, selectedSphere.box(), datasetId_);

    // Compute upper limit of the number of selected points
    size_t nSelected = 0;

    for (size_t i = 0; i < selectedNodes_.size(); i++)
    {
        const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        nSelected += static_cast<size_t>(nodeL2->size);
    }

    selectionSize = nSelected;
    if (selection.size() < selectionSize)
    {
        selection.resize(selectionSize);
    }

    // Select points
    nSelected = 0;

    size_t max = query_->maximumResults() - query_->resultSize();
    bool maxReached = false;

    for (size_t i = 0; i < selectedNodes_.size(); i++)
    {
        const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
        if (!nodeL2)
        {
            continue;
        }

        uint32_t nNodePoints = static_cast<uint32_t>(nodeL2->size);
        uint32_t from = static_cast<uint32_t>(nodeL2->from);

        // Partial/Whole selection, apply clip filter
        for (uint32_t j = 0; j < nNodePoints; j++)
        {
            uint32_t idx = from + j;
            double x = position[3 * idx + 0];
            double y = position[3 * idx + 1];
            double z = position[3 * idx + 2];

            if (selectedSphere.isInside(x, y, z))
            {
                selection[nSelected++] = idx;
                if (nSelected == max)
                {
                    maxReached = true;
                    break;
                }
            }
        }

        if (maxReached)
        {
            break;
        }
    }

    selectionSize = nSelected;

    query_->addResults(nSelected);
}

void Page::selectElevation()
{
    const Range<double> &elevationRange = query_->selectedElevationRange();

    if (elevationRange.hasBoundaryValues())
    {
        return;
    }

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        double elev = static_cast<double>(elevation[selection[i]]);

        if (!(elev < elevationRange.minimumValue() ||
              elev > elevationRange.maximumValue()))
        {
            if (nSelectedNew != i)
            {
                selection[nSelectedNew] = selection[i];
            }

            nSelectedNew++;
        }
    }

    LOG_DEBUG_LOCAL("selectionSize <" << selectionSize << "> nSelectedNew <"
                                      << nSelectedNew << ">");
    selectionSize = nSelectedNew;
}

void Page::selectClassification()
{
    const std::vector<int> &classifications = query_->selectedClassifications();

    if (classifications.empty())
    {
        return;
    }

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        uint32_t id = classification[selection[i]];

        if (classifications[id])
        {
            if (nSelectedNew != i)
            {
                selection[nSelectedNew] = selection[i];
            }

            nSelectedNew++;
        }
    }

    selectionSize = nSelectedNew;
}

void Page::selectLayer()
{
    const std::unordered_set<size_t> &layers = query_->selectedLayers();

    if (layers.empty())
    {
        return;
    }

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        uint32_t id = layer[selection[i]];

        if (layers.find(id) != layers.end())
        {
            if (nSelectedNew != i)
            {
                selection[nSelectedNew] = selection[i];
            }

            nSelectedNew++;
        }
    }

    selectionSize = nSelectedNew;
}

void Page::runColorModifier()
{
    const SettingsView &opt = editor_->settings().view();
    float r = opt.pointColor()[0];
    float g = opt.pointColor()[1];
    float b = opt.pointColor()[2];

    size_t n = position.size() / 3;

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
            renderColor[i * 3 + 0] *= color[i * 3 + 0];
            renderColor[i * 3 + 1] *= color[i * 3 + 1];
            renderColor[i * 3 + 2] *= color[i * 3 + 2];
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_USER_COLOR))
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] *= userColor[i * 3 + 0];
            renderColor[i * 3 + 1] *= userColor[i * 3 + 1];
            renderColor[i * 3 + 2] *= userColor[i * 3 + 2];
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
                     returnNumber[i],
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_NUMBER_OF_RETURNS))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     numberOfReturns[i],
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_CLASSIFICATION))
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i, classification[i], 15, ColorPalette::Classification);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_ELEVATION))
    {
        const Dataset &dataset = editor_->datasets().key(datasetId_);
        double zlen = dataset.boundary().length(2);

        if (zlen > 0.)
        {
            for (size_t i = 0; i < n; i++)
            {
                double v = 1. - (elevation[i] / zlen);
                renderColor[i * 3 + 0] *= static_cast<float>(v);
                renderColor[i * 3 + 1] *= static_cast<float>(v);
                renderColor[i * 3 + 2] *= static_cast<float>(v);
            }
        }
    }
}

void Page::setColor(size_t idx,
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
