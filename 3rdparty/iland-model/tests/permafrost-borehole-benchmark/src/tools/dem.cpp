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

#include "global.h"
#include "dem.h"

#include "globalsettings.h"
#include "model.h"

#include "gisgrid.h"

// from here: http://www.scratchapixel.com/lessons/3d-advanced-lessons/interpolation/bilinear-interpolation/
template<typename T>
T bilinear(
   const T &tx,
   const T &ty,
   const T &c00,
   const T &c10,
   const T &c01,
   const T &c11)
{
#if 1
    T a = c00 * (T(1) - tx) + c10 * tx;
    T b = c01 * (T(1) - tx) + c11 * tx;
    return a * (T(1) - ty) + b * ty;
#else
    return (T(1) - tx) * (T(1) - ty) * c00 +
        tx * (T(1) - ty) * c10 +
        (T(1) - tx) * ty * c01 +
        tx * ty * c11;
#endif
}

/// loads a DEM from a ESRI style text file.
/// internally, the DEM has always a resolution of 10m
bool DEM::loadFromFile(const QString &fileName)
{
    if (!GlobalSettings::instance()->model())
        throw IException("DEM::create10mGrid: no valid model to retrieve height grid.");

    HeightGrid *h_grid = GlobalSettings::instance()->model()->heightGrid();
    if (!h_grid || h_grid->isEmpty())
        throw IException("GisGrid::create10mGrid: no valid height grid to copy grid size.");

    GisGrid gis_grid;
    if (!gis_grid.loadFromFile(fileName))
        throw IException(QString("Unable to load DEM file %1").arg(fileName));
    // create a grid with the same size as the height grid
    // (height-grid: 10m size, covering the full extent)
    clear();
    aspect_grid.clear();
    slope_grid.clear();
    view_grid.clear();

    setup(h_grid->metricRect(),h_grid->cellsize());

    const QRectF &world = GlobalSettings::instance()->model()->extent();

    if (gis_grid.cellSize() <= cellsize()) {
        QPointF p;
        // simple copy of the data
        for (int i=0;i<count();i++) {
            p = cellCenterPoint(indexOf(i));
            if (gis_grid.value(p) != gis_grid.noDataValue() && world.contains(p) )
                valueAtIndex(i) = gis_grid.value(p);
            else
                valueAtIndex(i) = -1;
        }
    } else {
        // bilinear approximation approach
        if (fmod(gis_grid.cellSize(), cellsize()) != 0.f )
            throw IException("DEM: bilinear approximation: this requires a DEM with a resolution of a multiple of 10.");

        qDebug() << "DEM: built-in bilinear interpolation from cell size" << gis_grid.cellSize();
        int f = gis_grid.cellSize() / cellsize(); // size-factor
        initialize(-1.f);
        int ixmin = 10000000, iymin = 1000000, ixmax = -1, iymax = -1;
        for (int y=0;y<gis_grid.rows();++y)
            for (int x=0;x<gis_grid.cols(); ++x){
                Vector3D p3d = gis_grid.coord(x,y);
                if (world.contains(p3d.x(), p3d.y())) {
                    QPoint pt=indexAt(QPointF(p3d.x(), p3d.y()));
                    valueAt(p3d.x(), p3d.y()) = gis_grid.value(x,y);
                    ixmin = std::min(ixmin, pt.x()); ixmax=std::max(ixmax, pt.x());
                    iymin = std::min(iymin, pt.y()); iymax=std::max(iymax, pt.y());
                }
            }
        for (int y=iymin;y<=iymax-f;y+=f)
            for (int x=ixmin;x<=ixmax-f;x+=f){
                float c00 = valueAtIndex(x, y);
                float c10 = valueAtIndex(x+f, y);
                float c01 = valueAtIndex(x, y+f);
                float c11 = valueAtIndex(x+f, y+f);
                for (int my=0;my<f;++my)
                    for (int mx=0;mx<f;++mx)
                        valueAtIndex(x+mx, y+my) = bilinear<float>(mx/float(f), my/float(f), c00, c10, c01, c11);
            }
    }
    qDebug() << "Loaded DEM from " << fileName;
    return true;
}

/// calculate slope and aspect at a given point.
/// results are params per reference.
/// returns the height at point (x/y)
/// calculation follows: Burrough, P. A. and McDonell, R.A., 1998.Principles of Geographical Information Systems.(Oxford University Press, New York), p. 190.
/// http://uqu.edu.sa/files2/tiny_mce/plugins/filemanager/files/4280125/Principles%20of%20Geographical%20Information%20Systems.pdf
/// @param point metric coordinates of point to derive orientation
/// @param rslope_angle RESULTING (passed by reference) slope angle as percentage (i.e: 1:=45 degrees)
/// @param rslope_aspect RESULTING slope direction in degrees (0: North, 90: east, 180: south, 270: west)
float DEM::orientation(const QPointF &point, float &rslope_angle, float &rslope_aspect) const
{
    QPoint pt = indexAt(point);
    if (pt.x()>0 && pt.x()<sizeX()+1 && pt.y()>0 && pt.y()<sizeY()-1) {
        float *p = const_cast<DEM*>(this)->ptr(pt.x(), pt.y());
        float z2 = *(p-sizeX());
        float z4 = *(p-1);
        float z6 = *(p+1);
        float z8 = *(p+sizeX());
        float g = (-z4 + z6) / (2*cellsize());
        float h = (z2 - z8) / (2*cellsize());

        if (z2<=0. || z4<=0. || z6<=0. || z8<=0) {
            rslope_angle = 0.;
            rslope_aspect = 0.;
            return *p;
        }
        rslope_angle = sqrt(g*g + h*h);
        // atan2: returns -pi : +pi
        // North: -pi/2, east: 0, south: +pi/2, west: -pi/+pi
        float aspect = atan2(-h, -g);
        // transform to degree:
        // north: 0, east: 90, south: 180, west: 270
        aspect = aspect * 180. / M_PI + 360. + 90.;
        aspect = fmod(aspect, 360.f);

        rslope_aspect = aspect;
        return *p;
    } else {
        rslope_angle = 0.;
        rslope_aspect = 0.;
        return 0.;
    }
}

void DEM::createSlopeGrid() const
{
    if (slope_grid.isEmpty()) {
        // setup custom grids with the same size as this DEM
        slope_grid.setup(*this);
        view_grid.setup(*this);
        aspect_grid.setup(*this);
    } else {
        return;
    }
    float *slope = slope_grid.begin();
    float *view = view_grid.begin();
    float *aspect = aspect_grid.begin();

    // use fixed values for azimuth (315) and angle (45 deg) and calculate
    // norm vectors
    float sun_x = cos(315. * M_PI/180.) * cos(45.*M_PI/180.);
    float sun_y = sin(315. * M_PI/180.) * cos(45.*M_PI/180.);
    float sun_z = sin(45.*M_PI/180.);

    float a_x, a_y, a_z;
    for (float *p = begin(); p!=end(); ++p, ++slope, ++view, ++aspect) {
        QPointF pt = cellCenterPoint(indexOf(p));
        float height = orientation(pt, *slope, *aspect);
        // calculate the view value:
        if (height>0) {
            float h = atan(*slope);
            a_x = cos(*aspect * M_PI/180.) * cos(h);
            a_y = sin(*aspect * M_PI/180.) * cos(h);
            a_z = sin(h);

            // use the scalar product to calculate the angle, and then
            // transform from [-1,1] to [0,1]
            *view = (a_x*sun_x + a_y*sun_y + a_z*sun_z + 1.)/2.;
        } else {
            *view = 0.;
        }
    }

}
