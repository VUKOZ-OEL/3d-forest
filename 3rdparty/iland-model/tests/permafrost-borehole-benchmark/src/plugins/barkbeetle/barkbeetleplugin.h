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

#ifndef BARKBEETLEPLUGIN_H
#define BARKBEETLEPLUGIN_H

#include <QObject>
#include "barkbeetlemodule.h"

#include "plugin_interface.h"
class BarkBeetlePlugin: public QObject,
        public DisturbanceInterface,
        public SetupResourceUnitInterface,
        public TreeDeathInterface
{
    Q_OBJECT
    #if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "at.ac.boku.iland.barkbeetleplugin" FILE "barkbeetleplugin.json")
    #endif
    Q_INTERFACES(DisturbanceInterface SetupResourceUnitInterface TreeDeathInterface)


public:
    BarkBeetlePlugin();
    // general interface details
    QString name(); ///< a unique name of the plugin
    QString version(); ///< a version identification
    QString description(); ///< some additional description. This info is shown in the GUI and is printed to the log file.

    // setup
    /// setup after the main iLand model frame is created
    void setup();
    /// setup resource unit specific parameters
    void setupResourceUnit(const ResourceUnit *ru) { mBeetle.setup(ru);}
    /// setup additional javascript related features
    void setupScripting(QJSEngine *engine);

    /// function is called whenever a tree dies in iLand (in order to track storm damage)
    void treeDeath(const Tree *tree, const int removal_type)   {  Q_UNUSED(removal_type); mBeetle.treeDeath(tree);   }

    // calculations
    void yearBegin() { mBeetle.yearBegin(); }
    void run() { mBeetle.run(); }

    // special functions for direct access (testing)
    BarkBeetleModule *barkBeetleModule() { return &mBeetle; }
private:
    BarkBeetleModule mBeetle;
};

#endif // BARKBEETLEPLUGIN_H
