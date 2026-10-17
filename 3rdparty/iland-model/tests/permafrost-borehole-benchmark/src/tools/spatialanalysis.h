#ifndef SPATIALANALYSIS_H
#define SPATIALANALYSIS_H

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
#include "grid.h"
#include "layeredgrid.h"

#include <QObject>
#include <QJSValue>

class RumpleIndex; // forward
class SpatialLayeredGrid; // forward
/**
 * @brief The SpatialAnalysis class is the scripting class related to extra spatial analysis functions.
 * rumpleIndex: ratio crown surface area / ground area for the whole project area.
 * saveRumpleGrid(): save RU-based grid of rumple indices to an ASCII grid file.
 */
class SpatialAnalysis: public QObject
{
    Q_OBJECT
    Q_PROPERTY(double rumpleIndex READ rumpleIndexFullArea)
    Q_PROPERTY(QList<int> patchsizes READ patchsizes)

public:
    SpatialAnalysis(QObject *parent=0): QObject(parent), mRumple(0) {}
    ~SpatialAnalysis();
    static void addToScriptEngine();

    double rumpleIndexFullArea(); ///< retrieve the rumple index for the full landscape (one value)
    /// extract patches ('clumps') and save the resulting grid to 'fileName' (if not empty). Returns a vector with
    /// the number of pixels for each patch-id
    QList<int> extractPatches(Grid<double> &src, int min_size, QString fileName);
    QList<int> patchsizes() const { return mLastPatches; }

    static void runCrownProjection2m(FloatGrid *agrid=nullptr); ///< internal function that prepares crown cover for the whole landscape

public slots:
    // API for Javascript
    void saveRumpleGrid(QString fileName); ///< save a grid of rumple index values (resource unit level) to a ESRI grid file (ascii)
    void saveCrownCoverGrid(QString fileName); ///< save a grid if crown cover percentages (RU level) to a ESRI grid file (ascii)
    void saveCrownCoverGrid(QString fileName, QJSValue grid); ///< save a grid of crown cover with the extent/resolution given by 'grid'

    QJSValue patches(QJSValue grid, int min_size);

private:
    void calculateCrownCoverRU(); ///< calculate resource-unit level crown cover
    RumpleIndex *mRumple;
    SpatialLayeredGrid *mLayers;
    FloatGrid mCrownCoverGrid;
    Grid<int> mClumpGrid;
    QList<int> mLastPatches;
    friend class SpatialLayeredGrid;

};

/** The RumpleIndex is a spatial index relating surface area to ground area.
 *  In forestry, it is a indicator of vertical heterogeneity. In iLand, the Rumple Index is
 *  the variability of the maximum tree height on 10m level (i.e. the "Height"-Grid).
 *  The RumpleIndex is calculated for each resource unit and also for the full project area.
 **/
class RumpleIndex
{
public:
    RumpleIndex(): mLastYear(-1) {}
    void setup(); ///< sets up the grid
    void calculate(); ///< calculates (or re-calculates) index values
    double value(const bool force_recalculate=false); ///< calculates rumple index for the full project area
    const FloatGrid &rumpleGrid()  { value(); /* calculate if necessary */ return mRumpleGrid; } ///< return the rumple index for the full project area
    //
    double test_triangle_area();
private:
    double calculateSurfaceArea(const float *heights, const float cellsize);
    FloatGrid mRumpleGrid;
    double mRumpleIndex;
    int mLastYear;
};



class SpatialLayeredGrid: public LayeredGridBase
{
public:
    SpatialLayeredGrid() { setup(); }
    void setup(); ///< initial setup of the grid
    int addGrid(const QString name, FloatGrid *grid); ///< adds a 'grid' named 'name'. returns index of the newly added grid.
    const QStringList &gridNames() { return mGridNames; }

    double value(const float x, const float y, const int index) const { checkGrid(index); return mGrids[index]->constValueAt(x,y); }
    double value(const QPointF &world_coord, const int index) const { checkGrid(index); return mGrids[index]->constValueAt(world_coord); }
    double value(const int ix, const int iy, const int index) const { checkGrid(index); return mGrids[index]->constValueAtIndex(ix,iy);}
    double value(const int grid_index, const int index) const {  checkGrid(index); return mGrids[index]->constValueAtIndex(grid_index);}
    void range(double &rMin, double &rMax, const int index) const { rMin=9999999999.; rMax=-99999999999.;
                                                              for (int i=0;i<mGrids[index]->count(); ++i) {
                                                                  rMin=qMin(rMin, value(i, index));
                                                                  rMax=qMax(rMax, value(i,index));}}

private:
    inline void checkGrid(const int grid_index) const { if (mGrids[grid_index]==0) const_cast<SpatialLayeredGrid*>(this)->createGrid(grid_index); } ///< helper function that checks if grids are to be created
    void createGrid(const int grid_index); ///< create (if necessary) the actual grid
    QStringList mGridNames; ///< the list of grid names
    QVector<FloatGrid*> mGrids; ///< the grid


};
#endif // SPATIALANALYSIS_H
