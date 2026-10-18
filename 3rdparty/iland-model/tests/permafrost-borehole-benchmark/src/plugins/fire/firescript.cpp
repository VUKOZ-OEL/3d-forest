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
#include "firescript.h"
#include "firemodule.h"
#include "helper.h"
#include "scriptgrid.h"

/** @class FireScript
    @ingroup firemodule
    FireScript is the scripting shell for the fire module.
  */
FireScript::FireScript(QObject *parent) :
    QObject(parent)
{
    mFire = 0;
}

double FireScript::ignite(double x, double y, double firesize, double windspeed, double winddirection)
{
    double result=-1.;
    if (x>=0 && y>=0) {
        result = mFire->prescribedIgnition(x, y, firesize, windspeed, winddirection);
        qDebug() << "FireEvent triggered by javascript: " << x << y << firesize << windspeed << winddirection;
    } else {
        //int idx, gen, refill;
        //RandomGenerator::debugState(idx, gen, refill);
        //qDebug() << "before-ignite:" << idx << gen << refill;

        int old_id = mFire->fireId();
        bool only_ignite = firesize == -1;
        result = mFire->ignition(only_ignite);
        if (mFire->fireId() != old_id)
            qDebug() << "Burning fire triggered from javascript!" << result;
       }
    return result;
}

double FireScript::igniteBurnIn(double x, double y, double length, double max_fire_size, bool simulate)
{
    qDebug() << "fire event (burn in) triggered by Javascript at:" << x << "/" << y << "length:"<<  length << "max-fire-size (m2):" << max_fire_size;
    return mFire->burnInIgnition(x,y,length, max_fire_size, simulate);
}

QString FireScript::fireRUValueType=QLatin1String("");

QString fireRUValue(const FireRUData &data) {
    if (FireScript::fireRUValueType=="kbdi") return QString::number(data.kbdi());
    if (FireScript::fireRUValueType=="dbh") return QString::number(data.fireRUStats.avg_dbh);
    if (FireScript::fireRUValueType=="crownkill") return QString::number(data.fireRUStats.crown_kill);
    if (FireScript::fireRUValueType=="basalarea") return QString::number(data.fireRUStats.basal_area>0?data.fireRUStats.died_basal_area / data.fireRUStats.basal_area:0.);
    if (FireScript::fireRUValueType=="baseIgnition") return QString::number(data.baseIgnitionProbability());
    return "Error";
}
QString fireCharToStr(const char &c) {
    return QString('0' + c ); // convert \0 to '0', \1 to '1', ...
}

bool FireScript::gridToFile(QString grid_type, QString file_name)
{
    if (!GlobalSettings::instance()->model())
        return false;
    QString result;
    if (grid_type == "spread") {
        result = gridToESRIRaster(mFire->mGrid);
    } else if (grid_type == "border") {
        if (mFire->mBorderGrid.isEmpty())
            throw IException("Fire: 'border' grid not available!");
        result = gridToESRIRaster(mFire->mBorderGrid, &fireCharToStr);
    } else {
        fireRUValueType = grid_type;
        result = gridToESRIRaster(mFire->mRUGrid, &fireRUValue); // use a specific value function (see above)
    }

    if (!result.isEmpty()) {
        file_name = GlobalSettings::instance()->path(file_name);
        Helper::saveToTextFile(file_name, result);
        qDebug() << "saved grid to " << file_name;
        return true;
    }
    qDebug() << "could not save gridToFile because" << grid_type << "is not a valid grid.";
    return false;

}

QJSValue FireScript::grid(QString type)
{
    int idx = mFire->mFireLayers.indexOf(type);
    if (idx<0)
        qDebug() << "ERROR: FireScript:grid(): invalid grid" << type << "valid:" << mFire->mFireLayers.layerNames();

    // this is a copy!
    Grid<double> *damage_grid =  mFire->mFireLayers.copyGrid(idx);

    QJSValue g = ScriptGrid::createGrid(damage_grid, type);
    return g;

}

double FireScript::x() const
{
    if (mFire) return mFire->fireX(); else return -1;
}

double FireScript::y() const
{
    if (mFire) return mFire->fireY(); else return -1;
}

double FireScript::calcDyanmicManagementEffect(FireRUData *data)
{
    // call the event handler and return the dynamic management value
    QJSValue result = mOnIgnitionRU.call(QJSValueList() << data->kbdi() << data->managementSuppression());
    if (result.isUndefined())
        return data->managementSuppression();
    else
        return result.toNumber();
}

double FireScript::calculateFireSize(const FireRUData *data, double distribution_value)
{
    // call the event handler and return the dynamic management value
    QJSValue result = mCalcFireSize.call(QJSValueList() << data->kbdi()/data->kbdiRef() << distribution_value );
    if (result.isUndefined())
        return distribution_value;
    else
        return result.toNumber();

}

int FireScript::id() const
{
    if (!mFire) return -1;
    return mFire->fireId();
}
