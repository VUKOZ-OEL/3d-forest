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

#include "barkbeetlescript.h"

#include "barkbeetlemodule.h"
#include "outputmanager.h"
#include "helper.h"
#include "spatialanalysis.h"
#include "scriptgrid.h"
#include "mapgrid.h"
#include "scriptglobal.h"

/** @class BarkBeetleScript
    @ingroup beetlemodule
    BarkBeetleScript is the scripting shell for the bark beetle module.
  */

BarkBeetleScript::BarkBeetleScript(QObject *)
{
    mBeetle = 0;
}

void BarkBeetleScript::test(QString value)
{
    qDebug() << value;
}

void BarkBeetleScript::init(QJSValue fun)
{
    QJSValueList args = QJSValueList() << 0 << 0;

    if (!fun.isCallable()) {
        qDebug() << "no valid function in init!!";
        return;
    }
    for (int y=0;y < mBeetle->mGrid.sizeY(); ++y)
        for (int x=0;x < mBeetle->mGrid.sizeX(); ++x) {
            args[0] = x; args[1] = y;
            double result = fun.call(args).toNumber();
            mBeetle->mGrid.valueAtIndex(x,y).n = result;
        }
}

void BarkBeetleScript::run(QJSValue fun)
{
    QJSValueList args = QJSValueList() << 0 << 0;

    if (!fun.isCallable()) {
        qDebug() << "no valid function in run!!";
        return;
    }
    for (int y=0;y < mBeetle->mGrid.sizeY(); ++y)
        for (int x=0;x < mBeetle->mGrid.sizeX(); ++x) {
            args[0] = x; args[1] = y;
            fun.call(args);
        }
}

double BarkBeetleScript::pixelValue(int ix, int iy)
{
    if (mBeetle->mGrid.isIndexValid(ix,iy)) {
        return mBeetle->mGrid.valueAtIndex(ix, iy).n;
    } else {
        return -9999;
    }
}

void BarkBeetleScript::setPixelValue(int ix, int iy, double val)
{
    if (mBeetle->mGrid.isIndexValid(ix,iy)) {
        mBeetle->mGrid.valueAtIndex(ix, iy).n = val;
    }
}

double BarkBeetleScript::generations(int ix, int iy)
{
    if (mBeetle->mGrid.isIndexValid(ix,iy)) {
        return mBeetle->mRUGrid.valueAt( mBeetle->mGrid.cellCenterPoint(QPoint(ix,iy)) ).generations;
    } else {
        return -9999;
    }

}

void BarkBeetleScript::reloadSettings()
{
    mBeetle->loadParameters(false); // false: do not reset
}

void BarkBeetleScript::newYear()
{
    int y = mBeetle->manualYearBegin();
    qDebug() << "Barkbeetle-module: year=" << y;
}

void BarkBeetleScript::runBB(int iteration)
{
    qDebug() << "running bark beetle module....";
    mBeetle->run(iteration);
    GlobalSettings::instance()->outputManager()->save(); // just make sure database outputs are properly written
}

void BarkBeetleScript::clear()
{
    qDebug() << "clear bark beetle module....";
    mBeetle->clearGrids();
    mBeetle->loadParameters(); // do_reset=true
    mBeetle->loadAllVegetation();
}

bool BarkBeetleScript::gridToFile(QString type, QString filename)
{
    if (!GlobalSettings::instance()->model())
        return false;
    QString result;
    result = gridToESRIRaster(mBeetle->mLayers, type); // use a specific value function (see above)

    if (result.isEmpty()) {
        result = gridToESRIRaster(mBeetle->mRULayers, type); // try RU-level indicators
    }

    if (!result.isEmpty()) {
        filename = GlobalSettings::instance()->path(filename);
        Helper::saveToTextFile(filename, result);
        qDebug() << "saved grid to " << filename;
        return true;
    }
    qDebug() << "could not save gridToFile because" << type << "is not a valid grid.";
    return false;

}

QJSValue BarkBeetleScript::grid(QString type)
{
    int idx = mBeetle->mLayers.indexOf(type);
    if (idx<0)
        qDebug() << "ERROR: BarkBeetleScript:grid(): invalid grid" << type;
    // this is a copy
    Grid<double> *damage_grid = mBeetle->mLayers.copyGrid(idx);

    QJSValue g = ScriptGrid::createGrid(damage_grid, type);
    return g;
}

int BarkBeetleScript::damagedArea(int threshold, QString fileName)
{
    // get damage grid:
    Grid<double> *damage_grid = mBeetle->mLayers.copyGrid(mBeetle->mLayers.indexOf("dead"));
    SpatialAnalysis spat;
    QList<int> patches = spat.extractPatches(*damage_grid, threshold+1, fileName);
    int n=0, size=0;
    for (int i=0;i<patches.count();++i)
        if (patches[i]>threshold) {
            size+=patches[i];
            n++;
        }
    qDebug() << "BarkBeetleScript:damagedArea:" << n << "patches (area=" << size << ") above threshold" << threshold;
    delete damage_grid;
    return size;

}

int BarkBeetleScript::clearInfestedPixels(QJSValue standmap, int stand_id, double fraction)
{
    MapGridWrapper *gr = qobject_cast<MapGridWrapper*> ( standmap.toQObject() );
    if (!gr || !gr->map()) {
        qDebug() << "BarkBeetleScript::clearInfestedPixels: no valid stand-map!";
        return 0;
    }
    QRectF box = gr->map()->boundingBox(stand_id);
    //GridRunner<BarkBeetleCell> runner(mBeetle->mGrid, box);
    GridRunner<int> runner(gr->map()->grid(), box);
    int n_cleared = 0;
    while (runner.next()) {
        if (*runner.current() == stand_id) {
            BarkBeetleCell &bbc = mBeetle->mGrid.valueAt( runner.currentCoord() );
            if (bbc.infested) {
                if (fraction==1. || drandom()<fraction) {
                    bbc.infested = false;
                    n_cleared++;
                }
            }
        }
    }
    return n_cleared;
}

bool BarkBeetleScript::setInfested(int x, int y)
{
    if (!mBeetle->mGrid.isIndexValid(QPoint(x,y))) {
        qDebug() << "invalid index in BarkBeetleScript::setInfested(): x:" << x << "y:" << y;
        return false;
    }
    BarkBeetleCell &c = mBeetle->mGrid.valueAtIndex(QPoint(x,y));
    if (!c.isHost() || c.killed)
        return false;
    c.setInfested(true);
    c.outbreakYear = mBeetle->internalYear();
    return true;
}

int BarkBeetleScript::setInfestedFromMap(MapGridWrapper *grid, int key, double probability, int agerange)
{
    if (!grid) {
        qDebug() << "invalid map for BarkBeetleScropt::setInfestedMap: Map expected!";
        return -1;
    }
    if (grid->isValid()) {
        QRectF bounding_box = grid->map()->boundingBox(key);
        GridRunner<BarkBeetleCell> runner(mBeetle->mGrid, bounding_box);
        int n_infested = 0;
        int not_infested = 0;
        while (runner.next()) {
            // map grid is aligned to the 10m bark beetle grid
            if ( grid->map()->grid().constValueAtIndex( runner.currentIndex() ) == key ) {
                // set as infested
                if (runner.current()->isHost()) {
                    if (probability==1. || drandom() < probability ) {
                        runner.current()->setInfested(true);
                        runner.current()->outbreakYear = mBeetle->internalYear() + 1 - irandom(0,agerange); // +1: is effective *next* year; 0: new in the current year
                        ++n_infested;
                    } else {
                        ++not_infested; // due to prob
                    }
                } else {
                    ++not_infested; // is not a host cell
                }
            }

        }
        qDebug() << "BarkBeetle.setInfestedFromMap: for stand" << key << ": 10m px infested/not-infested:" << n_infested << "/" << not_infested;
        return n_infested;


    } else {
        qDebug() << "BarkBeetle.setInfestedFromMap: grid is not valid.";
        return -1;
    }

}

void BarkBeetleScript::setBackgroundInfestationProbability(double new_value)
{
    for (BarkBeetleRUCell *b=mBeetle->mRUGrid.begin();b!=mBeetle->mRUGrid.end();++b) {
        b->backgroundInfestationProbability = new_value;
    }
    qDebug() << "BarkBeetle: set backgroundInfestationProbability to" << new_value;

}

bool BarkBeetleScript::simulate()
{
    return mBeetle->simulate();
}

void BarkBeetleScript::setSimulate(bool do_simulate)
{
    mBeetle->setSimulate(do_simulate);
}

bool BarkBeetleScript::enabled()
{
    return mBeetle->enabled();
}

void BarkBeetleScript::setEnabled(bool do_set_enable)
{
    mBeetle->setEnabled(do_set_enable);
}
