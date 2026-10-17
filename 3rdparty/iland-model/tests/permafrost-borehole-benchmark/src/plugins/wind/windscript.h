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

#ifndef WINDSCRIPT_H
#define WINDSCRIPT_H

#include <QObject>
#include <QJSValue>

class WindModule; // forward

class WindScript : public QObject
{
    Q_OBJECT
public:
    explicit WindScript(QObject *parent = 0);
    void setModule(WindModule *module) { mModule = module; }
signals:
    
public slots:
    /** trigger a wind event from javascript.
      @param windspeed average wind speed (m/s)
      @param winddireciton wind direction (0=N..180=S..270=W)
      @param maximum_iterations maximum number of iterations
      @param simulate if true, trees are not really affected
      @param iteration if given a value >=0, then only one iteration is calculated ("interactive mode")
    */
    int windEvent(double windspeed, double winddirection, int max_iteration, bool simulate=false, int iteration=-1);
    /// create a "ESRI-grid" text file 'grid_type' is one of a fixed list of names, 'file_name' the ouptut file location
    bool gridToFile(QString grid_type, QString file_name);
    /// returns a ScriptGrid with the requested type
    QJSValue grid(QString type);
    /// initialize/ reset the wind module
    void initialize();
    /// initialize/reset the age of edges
    void initializeEdgeAge(int years);

    /// number of damaged pixels with a patchsize>'threshold'; save to grid to "fileName" (if not empty)
    int damagedArea(int threshold, QString fileName);

private:
    WindModule *mModule;
};

#endif // WINDSCRIPT_H
