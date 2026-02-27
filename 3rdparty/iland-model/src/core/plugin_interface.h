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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

/** This file contains the interfaces for iLand disturbance modules.
    It uses the plugin mechanism provided by Qt; the destructor is needed by some compilers (see Qt-doc).
    The interface contains several parts:
    * General information: used to identify plugins

*/
class QJSEngine; // forward
class ResourceUnit; // forward

class DisturbanceInterface
{
public:
    virtual ~DisturbanceInterface() {}

    // general information / properties
    virtual QString name()=0; ///< a unique name of the plugin
    virtual QString version()=0; ///< a version identification
    virtual QString description()=0; ///< some additional description. This info is shown in the GUI and is printed to the log file.

    // setup
    virtual void setup()=0; ///< setup after general iLand model frame is created.
    virtual void yearBegin()=0; ///< function executes at the beginning of a year (e.g., cleanup)
    virtual void run()=0; ///< main function that once a year (after growth)
    virtual void setupScripting(QJSEngine *engine)=0; ///< allow module specific javascript functions/classes
};

class SetupResourceUnitInterface
{
public:
    virtual ~SetupResourceUnitInterface() {}
    /// setup of parameters specific for resource unit.
    /// this allows using spatially explicit parmater values.
    virtual void setupResourceUnit(const ResourceUnit *ru)=0;

};

/** WaterInterface allows accessing intermediate water variables (e.g. interception)

 */
class WaterCycleData; // forward
class ResourceUnit; // forward
class WaterInterface
{
public:
    virtual ~WaterInterface() {}

    virtual void calculateWater(const ResourceUnit *resource_unit, const WaterCycleData *water_data)=0;
};

/**

    */
class Tree; // forward
//class Tree::TreeRemovalType;
class TreeDeathInterface
{
public:
    virtual ~TreeDeathInterface() {}
    virtual void treeDeath(const Tree *tree, const int removal_type)=0;
};

Q_DECLARE_INTERFACE(DisturbanceInterface, "org.iland-model.DisturbanceInterface/1.0")

Q_DECLARE_INTERFACE(WaterInterface, "org.iland-model.WaterInterface/1.0")

Q_DECLARE_INTERFACE(SetupResourceUnitInterface, "org.iland-model.SetupResourceUnitInterface/1.0")

Q_DECLARE_INTERFACE(TreeDeathInterface, "org.iland-model.TreeDeathInterface/1.0")

#endif // PLUGIN_INTERFACE_H
