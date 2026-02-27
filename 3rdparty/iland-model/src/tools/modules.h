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

#ifndef MODULES_H
#define MODULES_H

class DisturbanceInterface; // forward
class SetupResourceUnitInterface; // forward
class WaterInterface; // forward
class TreeDeathInterface; // forward
class ResourceUnit; // forward
class Tree; // forward
class WaterCycleData; // forward
class QJSEngine; // forward
/** The Modules class is the container for iLand modules (e.g. Fire, Wind, ...).
    It handles loading and invoking the functionality defined in the modules.
*/
class Modules
{
public:
    Modules();
    ~Modules();

    /// general setup
    void setup();
    /// get interface by name (or 0 if not present)
    DisturbanceInterface *module(const QString &module_name);

    bool hasSetupResourceUnits() { return !mSetupRUs.isEmpty(); }
    // setup of resource unit specific parameters
    void setupResourceUnit(const ResourceUnit* ru);

    // functions
    void yearBegin(); ///< executes yearly initialization code for each module
    void run(); ///< execute the modules
    // water
    void calculateWater(const ResourceUnit *resource_unit, const WaterCycleData *water_data);

    // tree death
    void treeDeath(const Tree *tree, int removal_type);
private:
    void init();
    QList<DisturbanceInterface*> mInterfaces; ///< the list stores only the active modules
    QList<SetupResourceUnitInterface*> mSetupRUs;
    QList<WaterInterface*> mWater;
    QList<TreeDeathInterface*> mTreeDeath;
};

#endif // MODULES_H
