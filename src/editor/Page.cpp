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

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <Dataset.hpp>
#include <Editor.hpp>
#include <Endian.hpp>
#include <File.hpp>
#include <LasFile.hpp>
#include <Page.hpp>
#include <Query.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "Page"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

Page::Page(Editor *editor, Query *query, uint32_t datasetId, uint32_t pageId)
    : position(nullptr),
      intensity(nullptr),
      returnNumber(nullptr),
      numberOfReturns(nullptr),
      classification(nullptr),
      userData(nullptr),
      gpsTime(nullptr),
      color(nullptr),
      segment(nullptr),
      elevation(nullptr),
      descriptor(nullptr),
      voxel(nullptr),
      renderPosition(nullptr),
      selectionSize(0),
      editor_(editor),
      query_(query),
      datasetId_(datasetId),
      pageId_(pageId),
      state_(Page::STATE_READ)
{
}

Page::~Page()
{
    if (editor_ && pageData_)
    {
        pageData_.reset();
        editor_->erasePage(datasetId_, pageId_);
    }
}

size_t Page::size() const
{
    if (pageData_)
    {
        return pageData_->size();
    }

    return 0;
}

void Page::setModified()
{
    if (pageData_)
    {
        pageData_->setModified();
    }
}

bool Page::modified() const
{
    if (pageData_)
    {
        return pageData_->modified();
    }

    return false;
}

void Page::resize(size_t n)
{
    position = pageData_->position.data();
    intensity = pageData_->intensity.data();
    returnNumber = pageData_->returnNumber.data();
    numberOfReturns = pageData_->numberOfReturns.data();
    classification = pageData_->classification.data();
    userData = pageData_->userData.data();
    gpsTime = pageData_->gpsTime.data();
    color = pageData_->color.data();
    segment = pageData_->segment.data();
    elevation = pageData_->elevation.data();
    descriptor = pageData_->descriptor.data();
    voxel = pageData_->voxel.data();
    renderPosition = pageData_->renderPosition.data();

    renderColor.resize(n * 3);

    selection.resize(n);
    selectionSize = n;
    for (size_t i = 0; i < n; i++)
    {
        selection[i] = static_cast<uint32_t>(i);
    }

    selectedNodes_.reserve(64);
}

void Page::readPage()
{
    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    pageData_ = editor_->readPage(datasetId_, pageId_);

    resize(pageData_->size());

    // Loaded.
    state_ = Page::STATE_TRANSFORM;

    // Apply.
    transform();
    queryWhere();
    runModifiers();
}

void Page::writePage()
{
    if (pageData_ && pageData_->modified())
    {
        pageData_->writePage(editor_);
    }
}

void Page::setState(Page::State state)
{
    // LOG_TRACE_UNKNOWN(<< "Page pageId <" << pageId_ << "> to state <" <<
    // state
    //                   << ">.");

    if ((state < state_) || (state == Page::STATE_RENDERED))
    {
        state_ = state;
    }

    if (state == Page::STATE_READ)
    {
        // modified_ = false;
    }
}

bool Page::nextState()
{
    LOG_DEBUG(<< "Compute state <" << Page::stateToString(state_) << ">.");

    if (state_ == Page::STATE_READ)
    {
        try
        {
            readPage();
        }
        catch (...)
        {
            // std::cout << "unknown error\n";
        }

        return true;
    }

    if (state_ == Page::STATE_TRANSFORM)
    {
        if (pageData_)
        {
            transform();
        }
        return true;
    }

    if (state_ == Page::STATE_SELECT)
    {
        if (pageData_)
        {
            queryWhere();
        }
        return true;
    }

    if (state_ == Page::STATE_RUN_MODIFIERS)
    {
        if (pageData_)
        {
            runModifiers();
        }
        return true;
    }

    if (state_ == Page::STATE_RENDER)
    {
        return true;
    }

    return false;
}

std::string Page::stateToString(Page::State state)
{
    const int stateNamesCount = 6;
    const char *stateNames[stateNamesCount] =
        {"read", "transform", "select", "modifiers", "render", "rendered"};
    if (state >= 0 && state < stateNamesCount)
    {
        return std::string(stateNames[state]);
    }

    return std::to_string(state);
}

void Page::transform()
{
    state_ = Page::STATE_SELECT;
}

void Page::queryWhere()
{
    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    const Region &region = query_->where().region();

    if (region.shape == Region::Shape::NONE)
    {
        // Reset selection to mark all points as selected.
        LOG_DEBUG(<< "Reset selection.");
        uint32_t n = static_cast<uint32_t>(size());
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
    queryWhereIntensity();
    queryWhereDescriptor();
    queryWhereClassification();
    queryWhereSegment();
    queryWhereSpecies();
    queryWhereManagementStatus();

    state_ = Page::STATE_RUN_MODIFIERS;
}

void Page::queryWhereBox()
{
    const Region &region = query_->where().region();
    const Box<double> &clipBox = region.box;
    if (region.shape != Region::Shape::BOX || clipBox.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    // Select octants.
    selectedNodes_.resize(0);
    IndexFile &octree = pageData_->octree;
    octree.selectLeaves(selectedNodes_, clipBox, datasetId_);

    // Compute upper limit of the number of selected points.
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

    // Select points.
    nSelected = 0;

    size_t max = query_->maximumResults();

    if (max == 0)
    {
        // Unlimited number of results.
        for (size_t i = 0; i < selectedNodes_.size(); i++)
        {
            const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
            if (!nodeL2)
            {
                continue;
            }

            size_t nNodePoints = static_cast<size_t>(nodeL2->size);
            size_t from = static_cast<size_t>(nodeL2->from);

            if (selectedNodes_[i].partial)
            {
                // Partial selection, apply clip filter.
                for (size_t j = 0; j < nNodePoints; j++)
                {
                    size_t idx = from + j;
                    double x = position[3 * idx + 0];
                    double y = position[3 * idx + 1];
                    double z = position[3 * idx + 2];

                    if (clipBox.contains(x, y, z))
                    {
                        selection[nSelected++] = static_cast<uint32_t>(idx);
                    }
                }
            }
            else
            {
                // Everything.
                for (size_t j = 0; j < nNodePoints; j++)
                {
                    selection[nSelected++] = static_cast<uint32_t>(from + j);
                }
            }
        }
    }
    else
    {
        // Limited number of results.
        max = max - query_->resultSize();
        bool maxReached = false;

        for (size_t i = 0; i < selectedNodes_.size(); i++)
        {
            const IndexFile::Node *nodeL2 = octree.at(selectedNodes_[i].idx);
            if (!nodeL2)
            {
                continue;
            }

            size_t nNodePoints = static_cast<size_t>(nodeL2->size);
            size_t from = static_cast<size_t>(nodeL2->from);

            if (selectedNodes_[i].partial)
            {
                // Partial selection, apply clip filter.
                for (size_t j = 0; j < nNodePoints; j++)
                {
                    size_t idx = from + j;
                    double x = position[3 * idx + 0];
                    double y = position[3 * idx + 1];
                    double z = position[3 * idx + 2];

                    if (clipBox.contains(x, y, z))
                    {
                        selection[nSelected++] = static_cast<uint32_t>(idx);
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
                // Everything.
                if (nNodePoints > max)
                {
                    nNodePoints = max;
                    maxReached = true;
                }

                for (size_t j = 0; j < nNodePoints; j++)
                {
                    selection[nSelected++] = static_cast<uint32_t>(from + j);
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
    if (region.shape != Region::Shape::CONE || clipCone.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    // Select octants.
    selectedNodes_.resize(0);
    IndexFile &octree = pageData_->octree;
    octree.selectLeaves(selectedNodes_, clipCone.box(), datasetId_);

    // Compute upper limit of the number of selected points.
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

    // Select points.
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

        size_t nNodePoints = static_cast<size_t>(nodeL2->size);
        size_t from = static_cast<size_t>(nodeL2->from);

        // Partial/Whole selection, apply clip filter.
        for (size_t j = 0; j < nNodePoints; j++)
        {
            size_t idx = from + j;
            double x = position[3 * idx + 0];
            double y = position[3 * idx + 1];
            double z = position[3 * idx + 2];

            if (clipCone.contains(x, y, z))
            {
                selection[nSelected++] = static_cast<uint32_t>(idx);
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
    if (region.shape != Region::Shape::CYLINDER || clipCylinder.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    // Select octants.
    selectedNodes_.resize(0);
    IndexFile &octree = pageData_->octree;
    octree.selectLeaves(selectedNodes_, clipCylinder.box(), datasetId_);

    // Compute upper limit of the number of selected points.
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

    // Select points.
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

        size_t nNodePoints = static_cast<size_t>(nodeL2->size);
        size_t from = static_cast<size_t>(nodeL2->from);

        // Partial/Whole selection, apply clip filter.
        for (size_t j = 0; j < nNodePoints; j++)
        {
            size_t idx = from + j;
            double x = position[3 * idx + 0];
            double y = position[3 * idx + 1];
            double z = position[3 * idx + 2];

            if (clipCylinder.contains(x, y, z))
            {
                selection[nSelected++] = static_cast<uint32_t>(idx);
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
    if (region.shape != Region::Shape::SPHERE || clipSphere.empty())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    // Select octants.
    selectedNodes_.resize(0);
    IndexFile &octree = pageData_->octree;
    octree.selectLeaves(selectedNodes_, clipSphere.box(), datasetId_);

    // Compute upper limit of the number of selected points.
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

    // Select points.
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

        size_t nNodePoints = static_cast<size_t>(nodeL2->size);
        size_t from = static_cast<size_t>(nodeL2->from);

        // Partial/Whole selection, apply clip filter.
        for (size_t j = 0; j < nNodePoints; j++)
        {
            size_t idx = from + j;
            double x = position[3 * idx + 0];
            double y = position[3 * idx + 1];
            double z = position[3 * idx + 2];

            if (clipSphere.contains(x, y, z))
            {
                selection[nSelected++] = static_cast<uint32_t>(idx);
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

    if (elevationRange.enabled() == false || elevationRange.full())
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

    if (descriptorRange.enabled() == false || descriptorRange.full())
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

void Page::queryWhereIntensity()
{
    const Range<double> &intensityRange = query_->where().intensity();

    if (intensityRange.enabled() == false || intensityRange.full())
    {
        return;
    }

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        double v = intensity[selection[i]];

        if (!(v < intensityRange.minimumValue() ||
              v > intensityRange.maximumValue()))
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
    if (!query_->where().classification().enabled())
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
        // LOG_DEBUG(<< "Query classification <" << id << "> at <" << i <<
        // ">.");

        if (id < classifications.size() && classifications[id])
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

void Page::queryWhereSegment()
{
    if (!query_->where().segment().enabled())
    {
        return;
    }

    const std::unordered_set<size_t> &segmentFilter =
        query_->where().segment().filter();
    const Segments &segments = editor_->segments();

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");
    LOG_DEBUG(<< "Number of query segments <" << segmentFilter.size() << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        size_t id = segment[selection[i]];

        if (segmentFilter.find(id) != segmentFilter.end() ||
            !segments.contains(id))
        {
            if (nSelectedNew != i)
            {
                selection[nSelectedNew] = selection[i];
            }

            nSelectedNew++;
        }
    }

    LOG_DEBUG(<< "Old selection size <" << selectionSize << ">.");
    LOG_DEBUG(<< "New selection size <" << nSelectedNew << ">.");
    selectionSize = nSelectedNew;
}

void Page::queryWhereSpecies()
{
    if (!query_->where().species().enabled())
    {
        return;
    }

    const QueryWhere &where = query_->where();
    const std::unordered_set<size_t> &speciesFilter = where.species().filter();
    const SpeciesList &speciesList = editor_->speciesList();
    const Segments &segments = editor_->segments();

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");
    LOG_DEBUG(<< "Number of query species <" << speciesFilter.size() << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        size_t segmentId = segment[selection[i]];
        size_t segmentIndex = segments.index(segmentId, false);

        if (segmentIndex != SIZE_MAX)
        {
            size_t speciesId = segments[segmentIndex].speciesId;

            if (speciesFilter.find(speciesId) != speciesFilter.end() ||
                !speciesList.contains(speciesId))
            {
                if (nSelectedNew != i)
                {
                    selection[nSelectedNew] = selection[i];
                }
                nSelectedNew++;
            }
        }
    }

    LOG_DEBUG(<< "Old selection size <" << selectionSize << ">.");
    LOG_DEBUG(<< "New selection size <" << nSelectedNew << ">.");
    selectionSize = nSelectedNew;
}

void Page::queryWhereManagementStatus()
{
    if (!query_->where().managementStatus().enabled())
    {
        return;
    }

    const std::unordered_set<size_t> &statusFilter =
        query_->where().managementStatus().filter();
    const ManagementStatusList &statusList = editor_->managementStatusList();
    const Segments &segments = editor_->segments();

    LOG_DEBUG(<< "Page pageId <" << pageId_ << ">.");
    LOG_DEBUG(<< "Number of query species <" << statusFilter.size() << ">.");

    size_t nSelectedNew = 0;

    for (size_t i = 0; i < selectionSize; i++)
    {
        size_t segmentId = segment[selection[i]];
        size_t segmentIndex = segments.index(segmentId, false);

        if (segmentIndex != SIZE_MAX)
        {
            size_t statusId = segments[segmentIndex].managementStatusId;

            if (statusFilter.find(statusId) != statusFilter.end() ||
                !statusList.contains(statusId))
            {
                if (nSelectedNew != i)
                {
                    selection[nSelectedNew] = selection[i];
                }
                nSelectedNew++;
            }
        }
    }

    LOG_DEBUG(<< "Old selection size <" << selectionSize << ">.");
    LOG_DEBUG(<< "New selection size <" << nSelectedNew << ">.");
    selectionSize = nSelectedNew;
}

void Page::runModifiers()
{
    // LOG_TRACE_UNKNOWN(<< "Page pageId <" << pageId_ << ">.");

    runColorModifier();
    editor_->runModifiers(this);

    state_ = Page::STATE_RENDER;
}

void Page::runColorModifier()
{
    const ViewSettings &opt = editor_->settings().viewSettings();

    size_t n = size();

    if (opt.colorSource() == ViewSettings::ColorSource::COLOR)
    {
        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] = static_cast<float>(color[i * 3 + 0]);
            renderColor[i * 3 + 1] = static_cast<float>(color[i * 3 + 1]);
            renderColor[i * 3 + 2] = static_cast<float>(color[i * 3 + 2]);
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::INTENSITY)
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
    else if (opt.colorSource() == ViewSettings::ColorSource::RETURN_NUMBER)
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     returnNumber[i],
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::NUMBER_OF_RETURNS)
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     numberOfReturns[i],
                     15,
                     ColorPalette::BlueCyanGreenYellowRed16);
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::CLASSIFICATION)
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i, classification[i], 15, ColorPalette::Classification);
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::SEGMENT)
    {
        const Segments &segments = editor_->segments();

        for (size_t i = 0; i < n; i++)
        {
            size_t segmentIndex = segments.index(segment[i], false);
            if (segmentIndex != SIZE_MAX)
            {
                const Vector3<double> &c = segments[segmentIndex].color;
                renderColor[i * 3 + 0] = static_cast<float>(c[0]);
                renderColor[i * 3 + 1] = static_cast<float>(c[1]);
                renderColor[i * 3 + 2] = static_cast<float>(c[2]);
            }
            else
            {
                renderColor[i * 3 + 0] = 0.8F;
                renderColor[i * 3 + 1] = 0.8F;
                renderColor[i * 3 + 2] = 0.8F;
            }
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::SPECIES)
    {
        const Segments &segments = editor_->segments();
        const SpeciesList &species = editor_->speciesList();

        for (size_t i = 0; i < n; i++)
        {
            size_t segmentIndex = segments.index(segment[i], false);
            if (segmentIndex != SIZE_MAX)
            {
                size_t speciesId = segments[segmentIndex].speciesId;
                size_t speciesIndex = species.index(speciesId, false);
                if (speciesIndex != SIZE_MAX)
                {
                    const Vector3<double> &c = species[speciesIndex].color;
                    renderColor[i * 3 + 0] = static_cast<float>(c[0]);
                    renderColor[i * 3 + 1] = static_cast<float>(c[1]);
                    renderColor[i * 3 + 2] = static_cast<float>(c[2]);
                }
                else
                {
                    renderColor[i * 3 + 0] = 0.8F;
                    renderColor[i * 3 + 1] = 0.8F;
                    renderColor[i * 3 + 2] = 0.8F;
                }
            }
            else
            {
                renderColor[i * 3 + 0] = 0.8F;
                renderColor[i * 3 + 1] = 0.8F;
                renderColor[i * 3 + 2] = 0.8F;
            }
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::MANAGEMENT_STATUS)
    {
        const Segments &segments = editor_->segments();
        const ManagementStatusList &statList = editor_->managementStatusList();

        for (size_t i = 0; i < n; i++)
        {
            size_t segmentIndex = segments.index(segment[i], false);
            if (segmentIndex != SIZE_MAX)
            {
                size_t id = segments[segmentIndex].managementStatusId;
                size_t index = statList.index(id, false);
                if (index != SIZE_MAX)
                {
                    const Vector3<double> &c = statList[index].color;
                    renderColor[i * 3 + 0] = static_cast<float>(c[0]);
                    renderColor[i * 3 + 1] = static_cast<float>(c[1]);
                    renderColor[i * 3 + 2] = static_cast<float>(c[2]);
                }
                else
                {
                    renderColor[i * 3 + 0] = 0.8F;
                    renderColor[i * 3 + 1] = 0.8F;
                    renderColor[i * 3 + 2] = 0.8F;
                }
            }
            else
            {
                renderColor[i * 3 + 0] = 0.8F;
                renderColor[i * 3 + 1] = 0.8F;
                renderColor[i * 3 + 2] = 0.8F;
            }
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::ELEVATION)
    {
        const Range<double> &e = editor_->elevationFilter();
        const Dataset &d = editor_->datasets().key(datasetId_);
        double a = static_cast<double>(d.range().elevationMin);
        double len = e.maximum() - e.minimum();

        if (len > 1e-6)
        {
            for (size_t i = 0; i < n; i++)
            {
                float v = static_cast<float>(1. - ((elevation[i] - a) / len));
                setColor(i,
                         static_cast<size_t>(v * 255.0),
                         255,
                         ColorPalette::BlueCyanYellowRed256);
            }
        }
    }
    else if (opt.colorSource() == ViewSettings::ColorSource::DESCRIPTOR)
    {
        for (size_t i = 0; i < n; i++)
        {
            setColor(i,
                     static_cast<size_t>(descriptor[i] * 255.0),
                     255,
                     ColorPalette::BlueCyanYellowRed256);
        }
    }
    else
    {
        double r = 1.0; // opt.pointColor()[0];
        double g = 1.0; // opt.pointColor()[1];
        double b = 1.0; // opt.pointColor()[2];

        for (size_t i = 0; i < n; i++)
        {
            renderColor[i * 3 + 0] = static_cast<float>(r);
            renderColor[i * 3 + 1] = static_cast<float>(g);
            renderColor[i * 3 + 2] = static_cast<float>(b);
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

    renderColor[idx * 3 + 0] = static_cast<float>(pal[colorValue][0]);
    renderColor[idx * 3 + 1] = static_cast<float>(pal[colorValue][1]);
    renderColor[idx * 3 + 2] = static_cast<float>(pal[colorValue][2]);
}
