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

#ifndef LAYEREDGRID_H
#define LAYEREDGRID_H

#include "grid.h"

/** \class LayeredGrid
    @ingroup tools
    This is the base class for multi-layer grids in iLand. Use the LayeredGrid-template class
    for creating actual multi layer grids. The LayeredGridBase can be used for specializations.
  */

class LayeredGridBase
{
public:
    // layer description element
    class LayerElement {
    public:
        LayerElement() {}
        LayerElement(QString aname, QString adesc, GridViewType type): name(aname), description(adesc), view_type(type) {}
        QString name;
        QString description;
        GridViewType view_type;
    };
    virtual ~LayeredGridBase() {}

    // access to properties
    virtual int sizeX() const=0;
    virtual int sizeY() const=0;
    virtual QRectF metricRect() const=0;
    virtual QRectF cellRect(const QPoint &p) const=0;
    virtual bool onClick(const QPointF &world_coord) const { Q_UNUSED(world_coord); return false; /*false: not handled*/ }
    // available variables
    /// list of stored layers
    virtual const QVector<LayeredGridBase::LayerElement> &names()=0;
    /// get layer index by name of the layer. returns -1 if layer is not available.
    virtual int indexOf(const QString &layer_name)
    {
        for(int i=0;i<names().count();++i)
            if (names().at(i).name == layer_name)
                return i;
        return -1;
    }
    virtual QStringList layerNames() {
        QStringList l;
         for(int i=0;i<names().count();++i)
             l.append(names().at(i).name);
         return l;
    }

    // statistics
    /// retrieve min and max of variable 'index'
    virtual void range(double &rMin, double &rMax, const int index) const=0;

    // data access functions
    virtual double value(const float x, const float y, const int index) const = 0;
    virtual double value(const QPointF &world_coord, const int index) const = 0;
    virtual double value(const int ix, const int iy, const int index) const = 0;
    virtual double value(const int grid_index, const int index) const = 0;
    // for classified values
    virtual const QString labelvalue(const int value, const int index) const
    {
        Q_UNUSED(value)
        Q_UNUSED(index)
        return QLatin1Literal("-");
    }

};

/** \class LayeredGrid is a template for multi-layered grids in iLand.
 * Use your cell-class for T and provide at minium a value() and a names() function.
 * The names() provide the names of the individual layers (used e.g. in the GUI), the value() function
 * returns a cell-specific value for a specific layer (given by 'index' parameter).
 * */
template <class T>
class LayeredGrid: public LayeredGridBase
{
public:
    LayeredGrid(const Grid<T>& grid) { mGrid = &grid; }
    LayeredGrid() { mGrid = 0;  }
    bool isValid() const { return mGrid != 0; }
    QRectF cellRect(const QPoint &p) const { return mGrid->cellRect(p); }
    QRectF metricRect() const { return mGrid->metricRect(); }
    float cellsize() const { return mGrid->cellsize(); }
    int sizeX() const { return mGrid->sizeX(); }
    int sizeY() const { return mGrid->sizeY();}

    virtual double value(const T& data, const int index) const = 0;
    double value(const T* ptr, const int index) const { return value(mGrid->constValueAtIndex(mGrid->indexOf(ptr)), index);  }
    double value(const int grid_index, const int index) const { return value(mGrid->constValueAtIndex(grid_index), index); }
    double value(const float x, const float y, const int index) const { return value(mGrid->constValueAt(x,y), index); }
    double value(const QPointF &world_coord, const int index) const { return mGrid->coordValid(world_coord)?value(mGrid->constValueAt(world_coord), index) : 0.; }
    double value(const int ix, const int iy, const int index) const { return value(mGrid->constValueAtIndex(ix, iy), index); }
    void range(double &rMin, double &rMax, const int index) const { rMin=9999999999.; rMax=-99999999999.;
                                                              for (int i=0;i<mGrid->count(); ++i) {
                                                                  rMin=qMin(rMin, value(i, index));
                                                                  rMax=qMax(rMax, value(i,index));}}

    /// extract a (newly created) grid filled with the value of the variable given by 'index'
    /// caller need to free memory!
    Grid<double> *copyGrid(const int index) const
    {
        Grid<double> *data_grid= new Grid<double>(mGrid->metricRect(), mGrid->cellsize());
        double *p = data_grid->begin();
        for (int i=0;i<mGrid->count();++i)
            *p++ = value(i, index);
        return data_grid;
    }


protected:
    const Grid<T> *mGrid;
};

void modelToWorld(const Vector3D &From, Vector3D &To);

/** translate

  */
template <class T>
    QString gridToESRIRaster(const LayeredGrid<T> &grid, const QString name)
{
        int index = const_cast<LayeredGrid<T> &>(grid).indexOf(name);
        if (index<0)
            return QString();
        Vector3D model(grid.metricRect().left(), grid.metricRect().top(), 0.);
        Vector3D world;
        modelToWorld(model, world);
        QString result = QString("ncols %1\r\nnrows %2\r\nxllcorner %3\r\nyllcorner %4\r\ncellsize %5\r\nNODATA_value %6\r\n")
                                .arg(grid.sizeX())
                                .arg(grid.sizeY())
                                .arg(world.x(),0,'f').arg(world.y(),0,'f')
                                .arg(grid.cellsize()).arg(-9999);

        QString res;
        QTextStream ts(&res);
        QChar sep = QChar(' ');
        for (int y=grid.sizeY()-1;y>=0;--y){
            for (int x=0;x<grid.sizeX();x++){
                ts << grid.value(x,y,index) << sep;
            }
            ts << "\r\n";
        }

        return result + res;
}



#endif // LAYEREDGRID_H





