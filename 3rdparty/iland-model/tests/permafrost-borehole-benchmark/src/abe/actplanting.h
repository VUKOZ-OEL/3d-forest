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
#ifndef ACTPLANTING_H
#define ACTPLANTING_H

#include "activity.h"
#include "species.h"

class ResourceUnitSpecies; // forward

namespace ABE {

class FMSTP; // forward
class FMStand; // forward

class ActPlanting : public Activity
{
public:
    ActPlanting(FMSTP *parent);
    QString type() const { return "planting"; }
    void setup(QJSValue value);
    bool execute(FMStand *stand);
    //bool evaluate(FMStand *stand);
    QStringList info();

    // run an one-time planting item
    static void runSinglePlantingItem(FMStand *stand, QJSValue value);
private:
    struct SPlantingItem {
        SPlantingItem(): species(0), fraction(0.), height(0.05), age(1), clear(false), grouped(false), group_type(-1), n(0), offset(0), spacing(0) {}
        Species *species;
        QJSValue fraction;
        double height;
        int age;
        bool clear;
        bool grouped; ///< true for pattern creation
        int group_type; ///< index of the pattern in the pattern list
        QJSValue n; ///< the number of patterns (random spread)
        int offset; ///< offset (in LIF-pixels) for the pattern algorithm
        QJSValue spacing;  ///< distance between two applications of a pattern
        bool random; ///< true if patterns are to be applied randomly
        bool setup(QJSValue value);
        void run(FMStand *stand);
    };
    QVector<SPlantingItem> mItems;
    bool mRequireLoading;

    static QStringList mAllowedProperties;


};

} // end namespace
#endif // ACTPLANTING_H
