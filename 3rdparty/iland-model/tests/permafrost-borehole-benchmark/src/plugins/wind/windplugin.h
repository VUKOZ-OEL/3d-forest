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
#ifndef WINDPLUGIN_H
#define WINDPLUGIN_H

#include <QObject>

#include "plugin_interface.h"
class ResourceUnit; // forward
class QJSEngine;
class WindModule;// forward
class WindOut;

/** a iLand plugin for wind disturbances.
  */
class WindPlugin: public QObject,
        public DisturbanceInterface,
        public SetupResourceUnitInterface
{
    Q_OBJECT
    Q_INTERFACES(DisturbanceInterface SetupResourceUnitInterface)
    #if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "at.ac.boku.iland.windplugin" FILE  "windplugin.json")
    #endif

public:
    WindPlugin();
    ~WindPlugin();
    // general interface details
    QString name(); ///< a unique name of the plugin
    QString version(); ///< a version identification
    QString description(); ///< some additional description. This info is shown in the GUI and is printed to the log file.

    // setup
    /// setup after the main iLand model frame is created
    void setup();
    /// setup resource unit specific parameters
    void setupResourceUnit(const ResourceUnit *ru);
    /// setup additional javascript related features
    void setupScripting(QJSEngine *engine);


    // calculations
    void yearBegin();
    void run();
    // special functions for direct access (testing)
    WindModule *windModule() { return mWind; }

private:
    WindModule *mWind;
    WindOut *mWindOut;
};


#endif // WINDPLUGIN_H
