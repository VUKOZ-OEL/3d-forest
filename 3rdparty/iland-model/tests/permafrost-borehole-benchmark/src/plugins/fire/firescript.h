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
#ifndef FIRESCRIPT_H
#define FIRESCRIPT_H

#include <QObject>
#include <QJSValue>
class FireModule; // forward
class FireRUData; // forward

class FireScript : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(int x READ x)
    Q_PROPERTY(int y READ y)
    Q_PROPERTY(QJSValue onIgnitionRU READ onIgnitionRU WRITE setOnIgnitionRU)
    Q_PROPERTY(QJSValue onCalculateFireSize READ onCalculateFireSize WRITE setOnCalculateFireSize )
    Q_PROPERTY(QJSValue onIgnition READ onIgnition WRITE setOnIgnition)

public:
    explicit FireScript(QObject *parent = nullptr);
    void setFireModule(FireModule *module) { mFire = module; }
    int id() const; ///< the id of the last ignited fire during the simulation
    double x() const; ///< the x-coordinate of the last ignition
    double y() const; ///< the x-coordinate of the last ignition
    static QString fireRUValueType; // for the exporter

    // event handlers
    QJSValue onIgnitionRU() const
    {
        return mOnIgnitionRU;
    }
    bool hasIgnitionRUHandler() { return mOnIgnitionRU.isCallable(); }

    // run the handler
    double calcDyanmicManagementEffect(FireRUData *data);

    bool hasCalculateFireSizeHandler() { return mCalcFireSize.isCallable(); }
    double calculateFireSize(const FireRUData *data, double distribution_value);
    QJSValue onCalculateFireSize() const
    {
        return mCalcFireSize;
    }
    QJSValue onIgnition() const {return mExternalIgnitions; }
    void setOnIgnition(QJSValue handler) { mExternalIgnitions = handler; }

signals:

public slots:
/**    Ignite a fire event with pre-defined properties, i.e. at a given location (parameters x_meter, y_meter) with given conditions (i.e. wind direction and speed). The fire size (as taken from the distribution)
    x_meter, y_meter: metric coordinates of the ignition point in the landscape
    firesize: provide a fire size (m2). The fire size will be drawn from the fire size distribution if firesize=-1 or omitted.
    windspeed: wind speed (m/s), drawn randomly if omitted or -1.
    winddirection: wind direction (0 deg =N..180deg=S..270=Wdeg), drawn randomly if omitted or set to -1.
    Returns the burnt area */
    double ignite(double x, double y, double firesize=-1, double windspeed=-1, double winddirection=-1);
    double igniteBurnIn(double x, double y, double length, double max_fire_size, bool simulate=false);
    bool gridToFile(QString grid_type, QString file_name); ///< create a "ESRI-grid" text file 'grid_type' is one of a fixed list of names, 'file_name' the ouptut file location
    /// returns a ScriptGrid with the requested type
    QJSValue grid(QString type);

    // setters
    void setOnIgnitionRU(QJSValue onIgnitionRU)
    {
        mOnIgnitionRU = onIgnitionRU;
    }

    void setOnCalculateFireSize(QJSValue onCalculateFireSize)
    {
        mCalcFireSize = onCalculateFireSize;
    }


private:
    FireModule *mFire;

    // event handler
    QJSValue mOnIgnitionRU;
    QJSValue mCalcFireSize;
    QJSValue mExternalIgnitions;
};

#endif // FIRESCRIPT_H
