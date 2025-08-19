/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#ifndef MAPGRID_H
#define MAPGRID_H
#include <QtCore/QHash>
#include <QRectF>
#include "grid.h"
#include "gisgrid.h"
class ResourceUnit; // forward
class Tree; // forward
class DeadTree; // forward
class SaplingTreeOld; // forward
class ResourceUnitSpecies; // forward

class MapGrid
{
public:
    MapGrid();
    /// create a MapGrid. the optional parameter "create_index" indicates if a spatial index (e.g. to query all pixels with a given value) should be built.
    MapGrid(const GisGrid &source_grid) { loadFromGrid(source_grid); }
    MapGrid(const QString &fileName, const bool create_index=true) { loadFromFile(fileName, create_index); }
    bool loadFromFile(const QString &fileName, const bool create_index=true); ///< load ESRI style text file
    bool loadFromGrid(const GisGrid &source_grid, const bool create_index=true); ///< load from an already present GisGrid
    void createEmptyGrid(); ///< create an empty grid with the size of the height grid of iLand (all values are 0, no index is created)
    void createIndex(); ///< (re-)creates the internal index (mRUIndex, mRectIndex, ...)

    // access
    const QString &name() const { return mName; }
    bool isValid() const { return !mGrid.isEmpty(); }
    const Grid<int> &grid() const { return mGrid; }
    /// number of stands stored in the index
    int count() const { return mRectIndex.size(); }
    /// get a list of all stored polygons in the map
    QList<int> mapIds() const { return mRectIndex.keys(); }
    // access
    /// returns true, if 'id' is a valid id in the grid, false otherwise.
    bool isValid(const int id) const { return mRectIndex.contains(id); }
    /// returns the bounding box of a polygon (metric coordinates)
    QRectF boundingBox(const int id) const { return isValid(id)?mRectIndex[id].first: QRectF(); }
    /// return the area (m2) covered by the polygon
    double area(const int id) const {return isValid(id)?mRectIndex[id].second : 0.;}
    /// returns the list of resource units with at least one pixel within the area designated by 'id'
    QList<ResourceUnit*> resourceUnits(const int id) const;
    /// returns a list with resource units and area factors per 'id'.
    /// the area is '1' if the resource unit is fully covered by the grid-value. (Less efficient that the function returning directly an iterator)
    QList<QPair<ResourceUnit*, double> > resourceUnitAreas(const int id) const { return mRUIndex.values(id); }
    /// returns a pair of iterators to the intenal hash with resource units and area factors for stand 'id'.
    /// the area is '1' if the resource unit is fully covered by the grid-value.
    QPair<QMultiHash<int, QPair<ResourceUnit*, double> >::const_iterator,
          QMultiHash<int, QPair<ResourceUnit*, double> >::const_iterator>   resourceUnitAreasIterator(const int id) const {
        return QPair<QMultiHash<int, QPair<ResourceUnit*, double> >::const_iterator,
                     QMultiHash<int, QPair<ResourceUnit*, double> >::const_iterator>(mRUIndex.constFind(id), mRUIndex.cend()); }
    /// return a list of all living trees on the area 'id'
    QList<Tree*> trees(const int id) const;
    /// load trees and store in list 'rList'. If 'filter'<>"", then the filter criterion is applied
    int loadTrees(const int id,  QVector<QPair<Tree *, double> > &rList, const QString filter=QString(), int n_estimate=0) const;

    /// load dead trees and store in list 'rList'. If 'filter'<>"", then the filter criterion is applied
    int loadDeadTrees(const int id,  QVector<DeadTree *> &rList, const QString filter=QString(), int n_estimate=0) const;

    /// return a list of grid-indices of a given stand-id
    QList<int> gridIndices(const int id) const;
    /// extract a list of neighborhood relationships between all the polygons of the grid
    const QMultiHash<int, int> neighborList() const { return mNeighborList; }
    void updateNeighborList(); ///< scan the map and fill the mNeighborList
    QList<int> neighborsOf(const int index) const;
    /// return true, if the point 'lif_grid_coords' (x/y integer key within the LIF-Grid)
    inline bool hasValue(const int id, const QPoint &lif_grid_coords) const { return mGrid.constValueAtIndex(lif_grid_coords.x()/cPxPerHeight, lif_grid_coords.y()/cPxPerHeight) == id; }
    /// return the stand-ID at the coordinates *from* the LIF-Grid (i.e., 2m grid).
    inline int standIDFromLIFCoord(const QPoint &lif_grid_coords) const  { return mGrid.constValueAtIndex(lif_grid_coords.x()/cPxPerHeight, lif_grid_coords.y()/cPxPerHeight); }

private:
    QString mName; ///< file name of the grid
    Grid<int> mGrid;
    QHash<int, QPair<QRectF,double> > mRectIndex; ///< holds the extent and area for each map-id
    QMultiHash<int, QPair<ResourceUnit*, double> > mRUIndex; ///< holds a list of resource units + areas per map-id
    QMultiHash<int, int> mNeighborList; ///< a list of neighboring polygons; for each ID all neighboring IDs are stored.
};

#endif // MAPGRID_H
