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
#include "bitedispersal.h"
#include "biteagent.h"
#include "biteengine.h"
#include "expression.h"
#include "helper.h"

#include "bitelifecycle.h"

namespace BITE {

//BiteDispersal::BiteDispersal()
//{
//    throw IException("Bite dispersal setup without JS object!");
//}


BiteDispersal::BiteDispersal(QJSValue obj): BiteItem(obj)
{
    mScriptGrid=nullptr;
}

void BiteDispersal::setup(BiteAgent *parent_agent)
{
    BiteItem::setup(parent_agent);
    setRunCells(false);

    try {
        qCDebug(biteSetup) << "Bite Dispersal constructor";
        checkProperties(mObj);

        double kernel_size = BiteEngine::valueFromJs(mObj, "maxDistance", "",  "'maxDistance' is a required property!").toNumber();
        QString expr = BiteEngine::valueFromJs(mObj, "kernel", "",  "'kernel' is a required property!").toString();

        QString dbg_file = BiteEngine::valueFromJs(mObj, "debugKernel", "").toString();
        setupKernel(expr, kernel_size, dbg_file);

        // setup of the dispersal grid
        mGrid.setup(agent()->grid().metricRect(), agent()->grid().cellsize());
        mGrid.initialize(0.);

        // Link to the script grid
        mScriptGrid = new ScriptGrid(&mGrid);
        mScriptGrid->setOwnership(false); // scriptgrid should not delete the grid

        mThis = BiteEngine::instance()->scriptEngine()->newQObject(this);
        BiteAgent::setCPPOwnership(this);


        // setup events
        mEvents.setup(mObj, QStringList() << "onBeforeSpread" << "onAfterSpread" << "onSetup", agent());

        agent()->wrapper()->registerGridVar(&mGrid, "dispersalGrid");

        QJSValueList eparam = QJSValueList() << thisJSObj();
        mEvents.run("onSetup", nullptr, &eparam);



    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of BiteDispersal item '%1': %2").arg(name()).arg(e.message());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);

    }

}

QString BiteDispersal::info()
{
    QString res = QString("Type: BiteDispersal\nDesc: %2\nKernel grid size: %1").arg(mKernel.sizeX()).arg(description());
    return res;
}

void BiteDispersal::run()
{
    QJSValueList p;
    p << thisJSObj(); // parameter: this instance

    prepareGrid();
    mEvents.run("onBeforeSpread", nullptr, &p);
    spreadKernel();
    mEvents.run("onAfterSpread", nullptr, &p);


    // decide();
}

void BiteDispersal::decide()
{
    // just a test...
    BiteCell **cell = agent()->grid().begin();
    for (double *p = mGrid.begin(); p!=mGrid.end(); ++p, ++cell) {
        if (*p > 0.) {
            *p = (drandom() < *p)  ? 1. : 0.;
            if (*cell != nullptr)
                (*cell)->setActive(*p == 1.);
        }

    }

}

QStringList BiteDispersal::allowedProperties()
{
    QStringList l = BiteItem::allowedProperties();
    l << "kernel" << "maxDistance" << "debugKernel";
    return l;
}

void BiteDispersal::setupKernel(QString expr, double max_dist, QString dbg_file)
{
    qCDebug(biteSetup) << "setup of kernel: expression:" << expr << ", max.distance:" << max_dist;
    Expression expression(expr);
    int max_radius = qFloor( max_dist / cellSize() );
    if (max_radius<=0)
        throw IException("Invalid maximum distance in setup of dispersal kernel.");

    mKernel.setup(cellSize(), 2*max_radius + 1 , 2*max_radius + 1);
    int offset = max_radius;
    QPoint centerp(offset, offset);
    mKernelOffset = offset;

    for (double *p = mKernel.begin(); p!=mKernel.end(); ++p) {
        QPoint distP = mKernel.indexOf(p) - centerp;
        double dist = sqrt(distP.x()*distP.x() + distP.y()*distP.y())*cellSize();
        if (dist < max_dist) {
            double v = expression.calculate(dist);
            *p = v;
        } else {
            // outside of circle: value = 0
            *p = 0.;
        }
    }

    double ksum = mKernel.sum();
    double kmax = mKernel.max();
    if (ksum!=0.f)
        mKernel.multiply( 1.f / ksum);

    qCDebug(biteSetup) << "Kernel setup. Size (x/y): " << mKernel.sizeX() << "/" << mKernel.sizeY() << ". Sum over all cells:" << ksum << ", max." << kmax << "Scaled: sum=" << mKernel.sum() << "max:" << mKernel.max();

    if (!dbg_file.isEmpty()) {
        QString fileName = GlobalSettings::instance()->path(dbg_file);
        QString result = gridToESRIRaster(mKernel);
        Helper::saveToTextFile(fileName, result);
        qDebug() << "debug: saved dispersal kernel to " << fileName;

    }
}

void BiteDispersal::spreadKernel()
{
    BiteCell **cell = agent()->grid().begin();
    bool die_after_disp = agent()->lifeCycle()->dieAfterDispersal();
    for (double *p = mGrid.begin(); p!=mGrid.end(); ++p,++cell) {
        if (*cell && *p == 1.) {
            ++agent()->stats().nDispersal;
            if ( (*cell)->isSpreading() ) {
                if (die_after_disp) {
                    (*cell)->die();
                }
            }
            agent()->notifyItems(*cell, BiteCell::CellSpread);
            QPoint cp=mGrid.indexOf(p);
            // the cell is a source, apply the kernel
            //int imin = std::max(0, cp.x()-mKernelOffset); int imax=std::min(mGrid.sizeX(), cp.x()+mKernelOffset);
            //int jmin = std::max(0, cp.y()-mKernelOffset); int jmax=std::min(mGrid.sizeY(), cp.y()+mKernelOffset);
            int imin = cp.x()-mKernelOffset; int imax=cp.x()+mKernelOffset;
            int jmin = cp.y()-mKernelOffset; int jmax=cp.y()+mKernelOffset;
            int kj = 0;
            int ki = 0;
            for (int j=jmin; j<=jmax; ++j, ++kj) {
                ki=0;
                for (int i=imin; i<=imax; ++i, ++ki) {
                    if (mGrid.isIndexValid(i,j)) {
                        double grid_val = mGrid.valueAtIndex(i,j);
                        double k_val = mKernel(ki, kj);
                        if (k_val>0. && grid_val<1.)
                            mGrid.valueAtIndex(i,j) = std::min( 1. - (1. - grid_val)*(1.-k_val), 1.);
                    }
                }
            }
        }
    }
}

void BiteDispersal::prepareGrid()
{
    BiteCell **cell = agent()->grid().begin();
    for (double *p = mGrid.begin(); p!=mGrid.end(); ++p, ++cell) {
        if (*cell && (*cell)->isSpreading())
            *p = 1.;
        else
            *p = 0.;
    }

}


// ********************************************************************************************************************
// *******************************************   BiteDistribution *****************************************************
// ********************************************************************************************************************


BiteDistribution::BiteDistribution(QJSValue obj): BiteItem(obj)
{
    mScriptGrid=nullptr;
}

void BiteDistribution::setup(BiteAgent *parent_agent)
{
    BiteItem::setup(parent_agent);
    setRunCells(false);

    try {
        checkProperties(mObj);

        // setup of the dispersal grid
        mGrid.setup(agent()->grid().metricRect(), agent()->grid().cellsize());
        mGrid.initialize(1.); // default: everywhere allowed...

        agent()->wrapper()->registerGridVar(&mGrid, "dispersalGrid");

        mEvents.setup(mObj, QStringList() << "onCalculate" << "onSetup", agent());

        QJSValue map_file = BiteEngine::valueFromJs(mObj, "map");
        if (map_file.isString()) {
            QString map_filename = GlobalSettings::instance()->path(map_file.toString());
            mGrid.loadGridFromFile(map_filename);
            qCDebug(biteSetup) << "Loaded input file" << map_filename << "for the BiteDistribution" << name();
        }

        mScriptGrid = new ScriptGrid(&mGrid);
        mScriptGrid->setOwnership(false); // scriptgrid should not delete the grid

        // run the setup event
        QJSValueList eparam = QJSValueList() << thisJSObj();
        mEvents.run("onSetup", nullptr, &eparam);

        mThis = BiteEngine::instance()->scriptEngine()->newQObject(this);
        BiteAgent::setCPPOwnership(this);




    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of BiteDistribution item '%1': %2").arg(name()).arg(e.message());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);
    }

}

QString BiteDistribution::info()
{
    QString res = QString("Type: BiteDistribution\nDesc: %1").arg(description());
    return res;

}

void BiteDistribution::run()
{
    QJSValueList eparam = QJSValueList() << thisJSObj();
    mEvents.run("onCalculate", nullptr, &eparam);
}

QStringList BiteDistribution::allowedProperties()
{
    QStringList res = QStringList() << "map";
    return res;
}

} // end namespace
