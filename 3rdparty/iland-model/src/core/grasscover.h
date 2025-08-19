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
#ifndef GRASSCOVER_H
#define GRASSCOVER_H

#include "expression.h"
#include "grid.h"
#include "layeredgrid.h"
#include "random.h"

class GrassCoverLayers; // forwared

// define the data type that is used to store the grass-levels
// use unsigned char for 1 byte (or quint8), unsigned short int (quint16) for two bytes per pixel
#define grass_grid_type qint16

/**
 * @brief The GrassCover class specifies the limiting effect of ground vegetation (grasses, herbs)
 * on the regeneration success of the tree species.
 * The GrassCover model is very simple and operates on a 2x2m grain.
 */
class GrassCover
{
public:
    GrassCover();
    ~GrassCover();
    void setup();
    // the number of steps used internally
    static const int GRASSCOVERSTEPS = 32000;

    /// set for all the pixels (LIFPixels) the corresponding grass value (in percent: 0-100)
    void setInitialValues(const QVector<float*> &LIFpixels, const int percent);

    /// main function (growth/die-off of grass cover)
    void execute();
    /// function called after the regeneration module
    void executeAfterRegeneration();

    // access
    /// returns 'true' if the module is enabled
    bool enabled() const { return mEnabled; }

    /// used algorithm
    enum GrassAlgorithmType { Invalid, Continuous, Pixel, Simplified };
    GrassAlgorithmType mode() const { return mType; }


    float lifThreshold() const { return mGrassLIFThreshold; }
    // access values of the underlying grid (for visualization)
    double effect(grass_grid_type level) const { return mEffect[level]; }
    double cover(const grass_grid_type &data) const  {return mType == Pixel? data : data/double(GRASSCOVERSTEPS-1);  }
    // access value for simplified mode


    /// main function
    double regenerationInhibition(QPoint &lif_index) const {

        if (mType==Pixel)
            // -1: off, out of project area, 0: off, ready to get grassy again, 1: off (waiting for LIF threshold), >1 on, counting down
            return mGrid.constValueAtIndex(lif_index)>1 ? 1. : 0.;

        // type continuous
        return mEnabled?effect(mGrid.constValueAtIndex(lif_index)) : 0.;
    }

    /// retrieve the grid of current grass cover
    const Grid<grass_grid_type> &grid() { return mGrid; }
private:

    GrassAlgorithmType mType;
    bool mEnabled; ///< is module enabled?
    Expression mGrassPotential; ///< function defining max. grass cover [0..1] as function of the LIF pixel value
    Expression mGrassEffect; ///< equation giving probability of *prohibiting* regeneration as a function of grass level [0..1]
    int mMaxTimeLag; ///< maximum duration (years) from 0 to full cover
    double mEffect[GRASSCOVERSTEPS]; ///< effect lookup table
    Grid<grass_grid_type> mGrid; ///< grid covering state of grass cover (in integer steps)
    int mGrowthRate; ///< max. annual growth rate of herbs and grasses (in 1/256th)
    grass_grid_type mMaxState; ///< potential at lif=1

    RandomCustomPDF mPDF; ///< probability density function defining the life time of grass-pixels
    float mGrassLIFThreshold; ///< if LIF>threshold, then the grass is considered as occupatied by grass
    GrassCoverLayers *mLayers; // visualization
};

/** Helper class manage and visualize data layers.

*/
class GrassCoverLayers: public LayeredGrid<grass_grid_type> {
  public:
    void setGrid(const Grid<grass_grid_type> &grid, const GrassCover *gc) { mGrid = &grid; mGrassCover=gc; }
    double value(const grass_grid_type &data, const int index) const;
    const QVector<LayeredGridBase::LayerElement> &names();
private:
    QVector<LayeredGridBase::LayerElement> mNames;
    const GrassCover *mGrassCover;
};

#endif // GRASSCOVER_H
