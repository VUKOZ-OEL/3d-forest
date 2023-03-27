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
#include <Math.hpp>
#include <Page.hpp>
#include <Query.hpp>

#define LOG_MODULE_NAME "Page"
#include <Log.hpp>

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
    intensity.clear();
    returnNumber.clear();
    numberOfReturns.clear();
    classification.clear();
    userData.clear();
    gpsTime.clear();
    color.clear();

    layer.clear();
    elevation.clear();
    customColor.clear();
    descriptor.clear();
    value.clear();

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
    intensity.resize(n);
    returnNumber.resize(n);
    numberOfReturns.resize(n);
    classification.resize(n);
    userData.resize(n);
    gpsTime.resize(n);
    color.resize(n * 3);

    layer.resize(n);
    elevation.resize(n);
    customColor.resize(n * 3);
    descriptor.resize(n);
    value.resize(n);

    renderPosition.resize(n * 3);
    renderColor.resize(n * 3);

    selection.resize(n);
    selectionSize = n;

    positionBase_.resize(n * 3);

    selectedNodes_.reserve(64);
}

void Page::readPage()
{
    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

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
    const double s16 = 1.0 / 65535.0;
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

        // intensity and color
        intensity[i] = static_cast<double>(point.intensity) * s16;

        if (rgbFlag)
        {
            color[3 * i + 0] = point.red * s16;
            color[3 * i + 1] = point.green * s16;
            color[3 * i + 2] = point.blue * s16;
        }
        else
        {
            color[3 * i + 0] = 1.0;
            color[3 * i + 1] = 1.0;
            color[3 * i + 2] = 1.0;
        }

        // attributes
        returnNumber[i] = point.return_number;
        numberOfReturns[i] = point.number_of_returns;
        classification[i] = point.classification;
        userData[i] = point.user_data;

        // gps
        gpsTime[i] = point.gps_time;

        // User extra
        layer[i] = point.user_layer;
        elevation[i] = static_cast<double>(point.user_elevation);
        customColor[3 * i + 0] = static_cast<double>(point.user_red) * s16;
        customColor[3 * i + 1] = static_cast<double>(point.user_green) * s16;
        customColor[3 * i + 2] = static_cast<double>(point.user_blue) * s16;
        descriptor[i] = point.user_descriptor;
        value[i] = static_cast<size_t>(point.user_value);
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
    queryWhere();
    runModifiers();
}

#define PAGE_FORMAT_COUNT 11

static const size_t PAGE_FORMAT_USER[PAGE_FORMAT_COUNT] =
    {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};

void Page::toPoint(uint8_t *ptr, size_t i, uint8_t fmt)
{
    const double s16 = 65535.0;
    size_t pos = PAGE_FORMAT_USER[fmt];

    // Do not overwrite the other values for now
    // - return number
    // - gps time
    // - etc.

    // Classification
    if (fmt > 5)
    {
        ptr[16] = classification[i];
    }

    // Layer
    htol32(ptr + pos, static_cast<uint32_t>(layer[i]));

    // Elevation
    htol32(ptr + pos + 4, static_cast<uint32_t>(elevation[i]));

    // Custom color
    ptr[pos + 8] = static_cast<uint16_t>(customColor[3 * i + 0] * s16);
    ptr[pos + 10] = static_cast<uint16_t>(customColor[3 * i + 1] * s16);
    ptr[pos + 12] = static_cast<uint16_t>(customColor[3 * i + 2] * s16);

    // Descriptor
    htold(ptr + pos + 16, descriptor[i]);

    // Value
    htol64(ptr + pos + 24, static_cast<uint64_t>(value[i]));
}

void Page::writePage()
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

void Page::queryWhere()
{
    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    const Region &region = query_->where().region();

    if (region.enabled == Region::TYPE_NONE)
    {
        // Reset selection to mark all points as selected.
        LOG_DEBUG(<< "Reset selection.");
        uint32_t n = static_cast<uint32_t>(position.size() / 3);
        selection.resize(n);
        selectionSize = n;
        for (uint32_t i = 0; i < n; i++)
        {
            selection[i] = i;
        }
    }

    // Apply new selection.
    queryWhereBox();
    queryWhereCone();
    queryWhereCylinder();
    queryWhereSphere();
    queryWhereElevation();
    queryWhereDescriptor();
    queryWhereClassification();
    queryWhereLayer();

    state_ = Page::STATE_RUN_MODIFIERS;
}

void Page::runModifiers()
{
    LOG_TRACE_UNKNOWN(<< "Page pageId <" << pageId_ << ">.");

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
    LOG_TRACE_UNKNOWN(<< "Page pageId <" << pageId_ << "> to state <" << state
                      << ">.");

    if ((state < state_) || (state == Page::STATE_RENDERED))
    {
        state_ = state;
    }

    if (state == Page::STATE_READ)
    {
        modified_ = false;
    }
}

std::string Page::stateToString(Page::State state)
{
    const char *stateNames[] =
        {"read", "transform", "select", "modifiers", "render", "rendered"};
    if (state < static_cast<int>(sizeof(stateNames)))
    {
        return std::string(stateNames[state]);
    }

    return std::to_string(state);
}

bool Page::nextState()
{
    LOG_TRACE_UPDATE_VIEW(<< "Compute state <" << Page::stateToString(state_)
                          << ">.");

    if (state_ == Page::STATE_READ)
    {
        try
        {
            LOG_DEBUG(<< "Page pageId <" << pageId_ << "> state <STATE_READ>.");
            readPage();
        }
        catch (std::exception &e)
        {
            // std::cout << e.what() << "\n";
        }
        catch (...)
        {
            // std::cout << "unknown error\n";
        }

        return true;
    }

    if (state_ == Page::STATE_TRANSFORM)
    {
        transform();
        return true;
    }

    if (state_ == Page::STATE_SELECT)
    {
        LOG_DEBUG(<< "Page pageId <" << pageId_ << "> state <STATE_SELECT>.");
        queryWhere();
        return true;
    }

    if (state_ == Page::STATE_RUN_MODIFIERS)
    {
        runModifiers();
        return true;
    }

    if (state_ == Page::STATE_RENDER)
    {
        return true;
    }

    return false;
}

void Page::queryWhereBox()
{
    const Region &region = query_->where().region();
    const Box<double> &clipBox = region.box;
    if (region.enabled != Region::TYPE_BOX || clipBox.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

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

void Page::queryWhereCone()
{
    const Region &region = query_->where().region();
    const Cone<double> &clipCone = region.cone;
    if (region.enabled != Region::TYPE_CONE || clipCone.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

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

void Page::queryWhereCylinder()
{
    const Region &region = query_->where().region();
    const Cylinder<double> &clipCylinder = region.cylinder;
    if (region.enabled != Region::TYPE_CYLINDER || clipCylinder.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    // Select octants
    selectedNodes_.resize(0);
    octree.selectLeaves(selectedNodes_, clipCylinder.box(), datasetId_);

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

            if (clipCylinder.isInside(x, y, z))
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

void Page::queryWhereSphere()
{
    const Region &region = query_->where().region();
    const Sphere<double> &clipSphere = region.sphere;
    if (region.enabled != Region::TYPE_SPHERE || clipSphere.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    // Select octants
    selectedNodes_.resize(0);
    octree.selectLeaves(selectedNodes_, clipSphere.box(), datasetId_);

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

            if (clipSphere.isInside(x, y, z))
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

void Page::queryWhereElevation()
{
    const Range<double> &elevationRange = query_->where().elevation();

    if (elevationRange.isEnabled() == false ||
        elevationRange.hasBoundaryValues())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

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

    selectionSize = nSelectedNew;
}

void Page::queryWhereDescriptor()
{
    const Range<double> &descriptorRange = query_->where().descriptor();

    if (descriptorRange.isEnabled() == false ||
        descriptorRange.hasBoundaryValues())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        double v = descriptor[selection[i]];

        if (!(v < descriptorRange.minimumValue() ||
              v > descriptorRange.maximumValue()))
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

void Page::queryWhereClassification()
{
    if (!query_->where().classification().isFilterEnabled())
    {
        return;
    }

    const std::vector<int> &classifications =
        query_->where().classificationArray();

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");
    LOG_DEBUG(<< "Query classifications <" << classifications << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        uint32_t id = classification[selection[i]];
        LOG_DEBUG(<< "Query classification <" << id << "> at <" << i << ">.");

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

void Page::queryWhereLayer()
{
    if (!query_->where().layer().isFilterEnabled())
    {
        return;
    }

    const std::unordered_set<size_t> &layers = query_->where().layer().filter();

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");
    LOG_DEBUG(<< "Number of query layers <" << layers.size() << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        size_t id = layer[selection[i]];

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
    double r = opt.pointColor()[0];
    double g = opt.pointColor()[1];
    double b = opt.pointColor()[2];

    size_t n = position.size() / 3;

    for (size_t i = 0; i < n; i++)
    {
        renderColor[i * 3 + 0] = static_cast<float>(r);
        renderColor[i * 3 + 1] = static_cast<float>(g);
        renderColor[i * 3 + 2] = static_cast<float>(b);
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_COLOR))
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] *= static_cast<float>(color[i * 3 + 0]);
            renderColor[i * 3 + 1] *= static_cast<float>(color[i * 3 + 1]);
            renderColor[i * 3 + 2] *= static_cast<float>(color[i * 3 + 2]);
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
                     static_cast<size_t>(intensity[i] * 255.0),
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

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_LAYER))
    {
        const Layers &layers = editor_->layers();
        const size_t max = layers.size();
        LOG_TRACE_UNKNOWN(<< "Maximum layers <" << max << ">.");

        for (size_t i = 0; i < n; i++)
        {
            if (layer[i] < max)
            {
                const Vector3<double> &c = layers.color(layer[i]);
                renderColor[i * 3 + 0] *= static_cast<float>(c[0]);
                renderColor[i * 3 + 1] *= static_cast<float>(c[1]);
                renderColor[i * 3 + 2] *= static_cast<float>(c[2]);
            }
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_ELEVATION))
    {
        const Dataset &dataset = editor_->datasets().key(datasetId_);
        double zlen = dataset.boundary().length(2);

        if (zlen > 1e-6)
        {
            for (size_t i = 0; i < n; i++)
            {
                const float v = static_cast<float>(1. - (elevation[i] / zlen));
                renderColor[i * 3 + 0] *= v;
                renderColor[i * 3 + 1] *= v;
                renderColor[i * 3 + 2] *= v;
            }
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_CUSTOM_COLOR))
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] *=
                static_cast<float>(customColor[i * 3 + 0]);
            renderColor[i * 3 + 1] *=
                static_cast<float>(customColor[i * 3 + 1]);
            renderColor[i * 3 + 2] *=
                static_cast<float>(customColor[i * 3 + 2]);
        }
    }

    if (opt.isColorSourceEnabled(opt.COLOR_SOURCE_DESCRIPTOR))
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] *= static_cast<float>(descriptor[i]);
            renderColor[i * 3 + 1] *= static_cast<float>(descriptor[i]);
            renderColor[i * 3 + 2] *= static_cast<float>(descriptor[i]);
        }
    }
}

void Page::setColor(size_t idx,
                    size_t colorValue,
                    size_t colorMax,
                    const std::vector<Vector3<double>> &pal)
{
    if (colorValue > colorMax)
    {
        colorValue = colorMax;
    }

    renderColor[idx * 3 + 0] *= static_cast<float>(pal[colorValue][0]);
    renderColor[idx * 3 + 1] *= static_cast<float>(pal[colorValue][1]);
    renderColor[idx * 3 + 2] *= static_cast<float>(pal[colorValue][2]);
}
