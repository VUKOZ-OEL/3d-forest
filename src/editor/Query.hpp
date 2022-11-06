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

/** @file Query.hpp */

#ifndef QUERY_HPP
#define QUERY_HPP

#include <unordered_set>

#include <Camera.hpp>
#include <Cone.hpp>
#include <ExportEditor.hpp>
#include <Page.hpp>
#include <Range.hpp>
#include <Sphere.hpp>

class Editor;

/** Data Query.

    A database query is a request for a database’s data so we can retrieve or
    manipulate it.

    A query may be used to access up to quintillions of point records that meet
    certain criteria.
*/
class EXPORT_EDITOR Query
{
public:
    Query(Editor *editor);
    ~Query();

    void selectBox(const Box<double> &box);
    void selectCone(double x, double y, double z, double z2, double angle);
    void selectSphere(double x, double y, double z, double radius);
    void selectElevationRange(const Range<double> &elevationRange);
    void selectDescriptorRange(const Range<float> &descriptorRange);
    void selectClassifications(const std::unordered_set<size_t> &list);
    void selectLayers(const std::unordered_set<size_t> &list);
    void selectCamera(const Camera &camera);

    const Box<double> &selectedBox() const { return selectBox_; }
    const Cone<double> &selectedCone() const { return selectCone_; }
    const Sphere<double> &selectedSphere() const { return selectedSphere_; }
    const Range<double> &selectedElevationRange() const
    {
        return elevationRange_;
    }
    const Range<float> &selectedDescriptorRange() const
    {
        return descriptorRange_;
    }
    const std::vector<int> &selectedClassifications() const
    {
        return selectClassifications_;
    }
    const std::unordered_set<size_t> &selectedLayers() const
    {
        return selectLayers_;
    }

    void setMaximumResults(size_t nPoints);
    size_t maximumResults() const { return maximumResults_; }

    void exec();
    void exec(const std::vector<IndexFile::Selection> &selectedPages);
    void reset();
    void clear();

    const std::vector<IndexFile::Selection> &selectedPages() const
    {
        return selectedPages_;
    }

    bool next()
    {
        if (pagePointIndex_ == pagePointIndexMax_)
        {
            return nextPage();
        }

        pagePointIndex_++;

        return true;
    }

    /** @name Point data available after next() */
    /**@{*/
    double &x() { return position_[3 * selection_[pagePointIndex_] + 0]; }
    double &y() { return position_[3 * selection_[pagePointIndex_] + 1]; }
    double &z() { return position_[3 * selection_[pagePointIndex_] + 2]; }

    float &intensity() { return intensity_[selection_[pagePointIndex_]]; }

    uint8_t &returnNumber()
    {
        return returnNumber_[selection_[pagePointIndex_]];
    }

    uint8_t &numberOfReturns()
    {
        return numberOfReturns_[selection_[pagePointIndex_]];
    }

    uint8_t &classification()
    {
        return classification_[selection_[pagePointIndex_]];
    }

    uint8_t &userData() { return userData_[selection_[pagePointIndex_]]; }

    double &gpsTime() { return gpsTime_[selection_[pagePointIndex_]]; }

    float &red() { return color_[3 * selection_[pagePointIndex_] + 0]; }
    float &green() { return color_[3 * selection_[pagePointIndex_] + 1]; }
    float &blue() { return color_[3 * selection_[pagePointIndex_] + 2]; }

    uint32_t &layer() { return layer_[selection_[pagePointIndex_]]; }
    double &elevation() { return elevation_[selection_[pagePointIndex_]]; }
    float &customRed()
    {
        return customColor_[3 * selection_[pagePointIndex_] + 0];
    }
    float &customGreen()
    {
        return customColor_[3 * selection_[pagePointIndex_] + 1];
    }
    float &customBlue()
    {
        return customColor_[3 * selection_[pagePointIndex_] + 2];
    }
    float &descriptor() { return descriptor_[selection_[pagePointIndex_]]; }
    float &density() { return density_[selection_[pagePointIndex_]]; }
    float &normalX() { return normal_[3 * selection_[pagePointIndex_] + 0]; }
    float &normalY() { return normal_[3 * selection_[pagePointIndex_] + 1]; }
    float &normalZ() { return normal_[3 * selection_[pagePointIndex_] + 2]; }
    size_t &value() { return value_[selection_[pagePointIndex_]]; }
    /**@}*/

    bool nextPage();
    Page *page() { return page_.get(); }
    size_t pageSizeEstimate() const;

    void setGrid(size_t pointsPerCell = 100000, double cellLengthMinPct = 1.0);
    bool nextGrid();
    const Box<double> &gridCell() const { return gridCell_; }
    size_t gridSize() const { return grid_.size(); }

    void setState(Page::State state);
    bool nextState();

    void setModified();
    void flush();

    size_t resultSize() const { return nResults_; }
    void addResults(size_t n);

    size_t cacheSize() const { return lru_.size(); }
    Page &cache(size_t index) { return *lru_[index]; }

protected:
    // Parent
    Editor *editor_;

    // Query
    Box<double> selectBox_;
    Cone<double> selectCone_;
    Sphere<double> selectedSphere_;
    Range<double> elevationRange_;
    Range<float> descriptorRange_;
    std::vector<int> selectClassifications_;
    std::unordered_set<size_t> selectLayers_;
    size_t maximumResults_;
    size_t nResults_;

    // Grid
    Box<double> gridCell_;
    Box<double> gridCellBase_;
    Box<double> gridBoundary_;
    size_t gridXSize_;
    size_t gridYSize_;
    size_t gridIndex_;
    std::vector<uint64_t> grid_;

    // Current page
    std::shared_ptr<Page> page_;

    double *position_;
    float *intensity_;
    uint8_t *returnNumber_;
    uint8_t *numberOfReturns_;
    uint8_t *classification_;
    uint8_t *userData_;
    double *gpsTime_;
    float *color_;

    uint32_t *layer_;
    double *elevation_;
    float *customColor_;
    float *descriptor_;
    float *density_;
    float *normal_;
    size_t *value_;

    uint32_t *selection_;

    // Iterator
    size_t pageIndex_;
    size_t pagePointIndex_;
    size_t pagePointIndexMax_;
    std::vector<IndexFile::Selection> selectedPages_;

    // Cache
    struct Key
    {
        size_t datasetId;
        size_t pageId;

        bool operator<(const Key &rhs) const;
    };
    size_t cacheSizeMax_;
    std::map<Key, std::shared_ptr<Page>> cache_;

    // Last Recently Used (LRU) for Cache
    std::vector<std::shared_ptr<Page>> lru_;

    std::shared_ptr<Page> read(size_t dataset, size_t index);
};

#endif /* QUERY_HPP */
