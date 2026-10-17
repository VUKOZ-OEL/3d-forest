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
#include "scriptgrid.h"
#include "globalsettings.h"
#include "helper.h"
#include "expression.h"
#include "modelcontroller.h"
#include "mapgrid.h"
#include "gisgrid.h"
#include "expressionwrapper.h"
#include "model.h"
#include "tree.h"


#include <QJSEngine>
#include <QJSValueIterator>

int ScriptGrid::mDeleted = 0;
int ScriptGrid::mCreated = 0;

ScriptGrid::ScriptGrid(QObject *parent) : QObject(parent)
{
    mGrid = nullptr;
    mVariableName = "x"; // default name
    mCreated++;
    mOwner = true;
}

ScriptGrid::~ScriptGrid()
{
    if (mGrid && mOwner)
        delete mGrid;
    mDeleted++;
    qDebug() << "ScriptGrid::balance: created:" << mCreated << "deleted:" << mDeleted;
}

// create a ScriptGrid-Wrapper around "grid". Note: destructing the 'grid' is done via the JS-garbage-collector.
QJSValue ScriptGrid::createGrid(Grid<double> *grid, QString name)
{
    ScriptGrid *g = new ScriptGrid(grid);
    if (!name.isEmpty())
        g->setName(name);
    QJSValue jsgrid = GlobalSettings::instance()->scriptEngine()->newQObject(g);
    return jsgrid;
}

void ScriptGrid::addToScriptEngine(QJSEngine *engine)
{
    qRegisterMetaType<ScriptGrid*>("ScriptGrid*"); // register type, required to have that type as property
    // allow for "new Grid"
    QJSValue jsMetaObject = engine->newQMetaObject(&ScriptGrid::staticQtMetaObject);
    engine->globalObject().setProperty("Grid", jsMetaObject);

}

bool ScriptGrid::create(int awidth, int aheight, int acellsize)
{
    if (mGrid)
        delete mGrid;
    mGrid = new Grid<double>(acellsize, awidth, aheight);
    mGrid->wipe();
    return true;
}

void ScriptGrid::setOrigin(double x, double y)
{
    if (!mGrid)
        return;
    QRectF rect = mGrid->metricRect();
    rect.moveTo(x,y);
    mGrid->setMetricRect( rect );
}

QJSValue ScriptGrid::copy()
{
    if (!mGrid)
        return QJSValue();

    ScriptGrid *newgrid = new ScriptGrid();
    // copy the data
    Grid<double> *copy_grid = new Grid<double>(*mGrid);
    newgrid->setGrid(copy_grid);

    QJSValue jsgrid = GlobalSettings::instance()->scriptEngine()->newQObject(newgrid);
    return jsgrid;
}

void ScriptGrid::clear()
{
    if (mGrid && !mGrid->isEmpty())
        mGrid->wipe();
}

void ScriptGrid::paint(double min_val, double max_val)
{
    // TODO: implement
    //if (GlobalSettings::instance()->controller())
    //    GlobalSettings::instance()->controller()->addGrid(mGrid, mVariableName, GridViewRainbow, min_val, max_val);
}

QString ScriptGrid::info()
{
    if (!mGrid || mGrid->isEmpty())
        return QString("not valid / empty.");
    return QString("grid-dimensions: %1/%2 (cellsize: %5, N cells: %3), grid-name='%4'").arg(mGrid->sizeX()).arg(mGrid->sizeY()).arg(mGrid->count()).arg(mVariableName).arg(mGrid->cellsize());
}

void ScriptGrid::save(QString fileName)
{
    if (!mGrid || mGrid->isEmpty())
        return;
    fileName = GlobalSettings::instance()->path(fileName);
    QString result = gridToESRIRaster(*mGrid);
    Helper::saveToTextFile(fileName, result);
    qDebug() << "saved grid " << name() << " to " << fileName;
}

bool ScriptGrid::load(QString fileName)
{
    fileName = GlobalSettings::instance()->path(fileName);
    // load the grid from file
//    MapGrid mg(fileName, false);
//    if (!mg.isValid()) {
//        qDebug() << "ScriptGrid::load(): load not successful of file:" << fileName;
//        return false;
//    }
//    GisGrid gisgrid;
//    if (!gisgrid.loadFromFile(fileName))
//        return false;
//    if (mGrid) {
//        delete mGrid;
//    }

    mGrid = new Grid<double>();
    if (!mGrid->loadGridFromFile(fileName))
        return false;
    QRectF rect = mGrid->metricRect();
    double wx = GlobalSettings::instance()->settings().valueDouble("model.world.location.x");
    double wy = GlobalSettings::instance()->settings().valueDouble("model.world.location.y");
    // move the grid to match the origin (0/0) of the model
    rect.moveTo( rect.left() - wx, rect.top() - wy );
    mGrid->setMetricRect(rect);
    //mGrid = mg.grid().toDouble(); // create a copy of the mapgrid-grid
    mVariableName = QFileInfo(fileName).baseName();
    return !mGrid->isEmpty();

}

void ScriptGrid::apply(QString expression)
{
    if (!mGrid || mGrid->isEmpty())
        return;

    Expression expr;
    double *varptr = expr.addVar(mVariableName);
    try {
        expr.setExpression(expression);
        expr.parse();
    } catch(const IException &e) {
        qDebug() << "JS - grid:apply(): ERROR: " << e.message();
        return;
    }

    // now apply function on grid
    for (double *p = mGrid->begin(); p!=mGrid->end(); ++p) {
        *varptr = *p;
        *p = expr.execute();
    }

}

void ScriptGrid::combine(QString expression, QJSValue grid_object)
{
    if (!grid_object.isObject()) {
        qDebug() << "ERROR: ScriptGrid::combine(): no valid grids object" << grid_object.toString();
        return;
    }
    QVector< Grid<double>* > grids;
    QStringList names;
    QJSValueIterator it(grid_object);
    while (it.hasNext()) {
        it.next();
        names.push_back(it.name());
        QObject *o = it.value().toQObject();
        if (o && qobject_cast<ScriptGrid*>(o)) {
            grids.push_back(qobject_cast<ScriptGrid*>(o)->grid());
            if (grids.last()->isEmpty() || grids.last()->cellsize() != mGrid->cellsize() || grids.last()->rectangle()!=mGrid->rectangle()) {
                qDebug() << "ERROR: ScriptGrid::combine(): the grid " << it.name() << "is empty or has different dimensions:" << qobject_cast<ScriptGrid*>(o)->info();
                return;
            }
        } else {
            qDebug() << "ERROR: ScriptGrid::combine(): no valid grid object with name:" << it.name();
            return;
        }
    }
    // now add names
    Expression expr;
    QVector<double *> vars;
    for (int i=0;i<names.count();++i)
        vars.push_back( expr.addVar(names[i]) );
    try {
        expr.setExpression(expression);
        expr.parse();
    } catch(const IException &e) {
        qDebug() << "JS - grid:combine(): expression ERROR: " << e.message();
        return;
    }

    // now apply function on grid
    for (int i=0;i<mGrid->count();++i) {
        // set variable values in the expression object
        for (int v=0;v<names.count();++v)
            *vars[v] = grids[v]->valueAtIndex(i);
        double result = expr.execute();
        mGrid->valueAtIndex(i) = result; // write back value
    }
}

QJSValue ScriptGrid::resample(QJSValue grid_object)
{
    if (!mGrid) {
        qDebug() << "ERROR in ScriptGrid::resample(): not a valid grid!";
        return QJSValue();
    }
    // crop the grid to the extent given by the grid 'grid_object'
    QObject *o = grid_object.toQObject();
    if (o && qobject_cast<ScriptGrid*>(o)) {
        Grid<double> *src = qobject_cast<ScriptGrid*>(o)->grid();
        Grid<double> *new_grid = new Grid<double>(src->metricRect(), src->cellsize());
        // now copy content for all cells of the new grid:
        QPointF p;
        for (int i=0;i<new_grid->count();++i) {
            p=new_grid->cellCenterPoint(i);
            if (mGrid->coordValid(p))
                (*new_grid)[i] = (*mGrid)[p];
            else
                (*new_grid)[i] = 0.; // should be NA
        }
        // free the original grid...
        delete mGrid;
        mGrid = new_grid;


    } else {
        qDebug() << "ERROR in ScriptGrid::resample(): grid_object is not a valid grid!";
        return QJSValue();
    }

    QJSValue jsgrid = GlobalSettings::instance()->scriptEngine()->newQObject(this);
    return jsgrid;


}

void ScriptGrid::aggregate(int factor)
{
    if (!mGrid) {
        throw IException("ERROR in ScriptGrid::aggregate(): not a valid grid!");
    }
    Grid<double> res = mGrid->averaged(factor);
    // make a copy on the heap
    Grid<double> *new_grid = new Grid<double>(res);
    // delete the old data, and use the new data instead
    delete mGrid;
    mGrid = new_grid;


}

double ScriptGrid::sum(QString expression)
{
    if (!mGrid || mGrid->isEmpty())
        return -1.;

    Expression expr;
    double *varptr = expr.addVar(mVariableName);
    try {
        expr.setExpression(expression);
        expr.parse();
    } catch(const IException &e) {
        qDebug() << "JS - grid:apply(): ERROR: " << e.message();
        return -1.;
    }

    // now apply function on grid
    double sum = 0.;
    for (double *p = mGrid->begin(); p!=mGrid->end(); ++p) {
        *varptr = *p;
        sum += expr.execute();
    }
    return sum;
}

void ScriptGrid::sumTrees(QString expression, QString filter)
{
    if (!mGrid || mGrid->isEmpty())
        return;

    mGrid->wipe();

    try {

        TreeWrapper tw;
        Expression custom_expr;
        custom_expr.setExpression(expression);
        custom_expr.setModelObject(&tw);

        Expression filterexpr;
        bool do_filter = !filter.isEmpty();
        filterexpr.setExpression(filter);
        filterexpr.setModelObject(&tw);

        AllTreeIterator ati(GlobalSettings::instance()->model());
        while (Tree *t = ati.next()) {

            // only trees on the grid area:
            if (!mGrid->coordValid(t->position()))
                continue;

            // apply filter
            tw.setTree(t);
            if (do_filter && !filterexpr.execute())
                continue;

            // calculate
            double val=custom_expr.execute();
            mGrid->valueAt(t->position()) += val;
        }

    } catch(const IException &e) {
        qDebug() << "ScriptGrid::sumTrees: an error occured." << e.message();
    }


}

