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
#ifndef SCRIPTGRID_H
#define SCRIPTGRID_H


#include <QObject>
#include <QJSValue>

#include "grid.h"

class ScriptGrid : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(int cellsize READ cellsize)
    Q_PROPERTY(bool isValid READ isValid)
public:
    Q_INVOKABLE ScriptGrid(QObject *parent = nullptr);
    Q_INVOKABLE ScriptGrid(QString fileName): ScriptGrid() {  load(fileName); }

    explicit ScriptGrid(Grid<double> *grid) { mVariableName="x"; mGrid = grid; mCreated++; mOwner=true; }

    void setGrid(Grid<double> *grid) { mGrid = grid; mOwner=true; }
    void setOwnership(bool should_delete) { mOwner = should_delete; }
    ~ScriptGrid();
    static QJSValue createGrid(Grid<double> *grid, QString name=QString());

    QString name() const {return mVariableName;}
    Grid<double> *grid() { return mGrid; }

    int width() const { return mGrid?mGrid->sizeX():-1; }
    int height() const { return mGrid?mGrid->sizeY():-1; }
    int count() const { return mGrid?mGrid->count():-1; }
    int cellsize() const { return mGrid?mGrid->cellsize():-1; }
    bool isValid() const { return mGrid?!mGrid->isEmpty():false; }

    static void addToScriptEngine(QJSEngine *engine);
signals:

public slots:
    /// creates a grid with cellsize / width / height
    /// the offset of the grid (lower left corner) is given by coordx/coordy
    bool create(int awidth, int aheight, int acellsize);

    void setName(QString arg) { mVariableName = arg; }
    /// set the origin of the grid (iLand coordiantes).
    /// the width/height of the grid are not altered, the grid is just moved to the new coordiantes.
    void setOrigin(double x, double y);
    /// create a copy of the current grid and return a new script object
    QJSValue copy();
    /// fill the grid with 0-values
    void clear();

    /// draw the map
    void paint(double min_val, double max_val);

    QString info();

    /// save to a file as ESRI ASC raster grid (relative to project file)
    void save(QString fileName);

    /// load from a file (ESRI ASC raster grid), relative to project root.
    /// return true on success.
    bool load(QString fileName);

    /// apply a function on the values of the grid, thus modifiying the grid (see the copy() function).
    /// The function is given as an Expression and is run for each cell of the grid.
    void apply(QString expression);

    /// combine multiple grids, and calculate the result of 'expression'
    void combine(QString expression, QJSValue grid_object);

    /// resamples the grid to the extent/cellsize given by the grid 'grid_object'
    /// Resampling is "brute-force", every cell of the new grid gets the cell value of the cell center
    /// returns the object itself.
    QJSValue resample(QJSValue grid_object);

    void aggregate(int factor);


    /// apply the expression "expression" on all pixels of the grid and return the sum of the values
    double sum(QString expression);

    /// loop over all trees and create a sum of 'expression' for each cell. Filter trees with 'filter'
    void sumTrees(QString expression, QString filter);

    /// access values of the grid
    double value(int x, int y) const {return (isValid() && mGrid->isIndexValid(x,y)) ? mGrid->valueAtIndex(x,y) : -1.;}
    /// write values to the grid
    void setValue(int x, int y, double value) const { if(isValid() && mGrid->isIndexValid(x,y)) mGrid->valueAtIndex(x,y)=value;}

    /// access value of the grid in metric coordinates
    double valueAt(double x, double y) const { return (isValid() && mGrid->coordValid(x,y)) ? mGrid->valueAt(x,y) : -1; }
    /// write values to the grid at metric coordinates x and y.
    void setValueAt(double x, double y, double value) const { if (isValid() && mGrid->coordValid(x,y))  mGrid->valueAt(x,y)=value; }

    // coordinate functions
    /// convert a cellindex in x-direction to a metric value (x) representing the center of the cell
    double metricX(int indexx) const { return isValid() ?  mGrid->cellCenterPoint(QPoint(indexx, 0)).x() : 0.; }
    /// convert a cellindex in y-direction to a metric value (y) representing the center of the cell
    double metricY(int indexy) const { return isValid() ?  mGrid->cellCenterPoint(QPoint(0, indexy)).y() : 0.; }
    /// convert a metric value (x-axis) to an index for the x-axis (see also isIndexValid(), isCoordValid() )
    int indexX(double meterx) const { return isValid() ? mGrid->indexAt(QPointF(meterx, 0)).x() : -1; }
    /// convert a metric value (y-axis) to an index for the y-axis (see also isIndexValid(), isCoordValid() )
    int indexY(double metery) const { return isValid() ? mGrid->indexAt(QPointF(0., metery)).y() : -1; }
    /// check if a given pair of indices (x/y) is valid for the grid
    bool isIndexValid(int x, int y) const {return isValid() ? mGrid->isIndexValid(x,y): false; }
    /// check if a given pair of metric coordinates (x/y) is valid for the grid
    bool isCoordValid(double x, double y) const {return isValid() ? mGrid->coordValid(x, y) : false; }

private:
    Grid<double> *mGrid;
    QString mVariableName;
    bool mOwner; // true if grid should be deleted
    static int mCreated;
    static int mDeleted;
};

#endif // SCRIPTGRID_H
