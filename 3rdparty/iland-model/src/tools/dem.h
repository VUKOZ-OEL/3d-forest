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

#ifndef DEM_H
#define DEM_H
#include "grid.h"
/** DEM is a digital elevation model class.
  @ingroup tools
   It uses a float grid internally.
   slope is calculated in "%", i.e. a value of 1 is 45deg (90deg -> inf)

   The aspect angles are defined as follows (like ArcGIS, values in degrees):
          0
          N
   270 W x E 90
          S
         180

   Values for height of -1 indicate "out of scope", "invalid" values

  */

class DEM: public FloatGrid
{
public:
    DEM(const QString &fileName) { loadFromFile(fileName); }
    bool loadFromFile(const QString &fileName);
    // create and fill grids for aspect/slope
    void createSlopeGrid() const;
    /// grid with aspect, i.e. slope direction in degrees (0: North, 90: east, 180: south, 270: west)
    const FloatGrid *aspectGrid() const { createSlopeGrid(); return &aspect_grid; }
    /// grid with slope, given as slope angle as percentage (i.e: 1:=45 degrees)
    const FloatGrid *slopeGrid() const { createSlopeGrid(); return &slope_grid; }
    const FloatGrid *viewGrid() const { createSlopeGrid(); return &view_grid; }
    // special functions for DEM
    /// get the elevation (m) at point (x/y)
    float elevation(const float x, const float y) const { return constValueAt(x,y); }
    float elevation(const QPointF p) const { return constValueAt(p.x(),p.y()); }
    /// get the direction of the slope at point (x/y)
    /// if the slope at the point is 0, "north" (0) is returned.
    float direction(const float x, const float y);
    float slope(const float x, const float y);
    /// get orientation at specific point (x,y) and height
    float orientation(const QPointF &point, float &rslope_angle, float &rslope_aspect) const;
    float orientation(const float x, const float y, float &rslope_angle, float &rslope_aspect)
                        { return orientation(QPointF(x,y), rslope_angle, rslope_aspect); }
    /// topographic position index
    /// TPI measures the difference between elevation at the central point
    ///  and the average elevation (z) around it within a predetermined radius (radius in m)
    float topographicPositionIndex(const QPointF &point, float radius) const;


private:
    mutable FloatGrid aspect_grid;
    mutable FloatGrid slope_grid;
    mutable FloatGrid view_grid;
};

#endif // DEM_H
