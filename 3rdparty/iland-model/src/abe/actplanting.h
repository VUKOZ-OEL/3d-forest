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
        //SPlantingItem() {}
        ~SPlantingItem() { if (patches) delete patches; }
        Species *species {nullptr};
        QJSValue fraction {0};
        double height {0.05};
        int age {1};
        bool clear {false};
        bool grouped {false}; ///< true for pattern creation
        int group_type {0}; ///< index of the pattern in the pattern list
        QJSValue n; ///< the number of patterns (random spread)
        int offset {0}; ///< offset (in LIF-pixels) for the pattern algorithm
        QJSValue spacing;  ///< distance between two applications of a pattern
        bool random {false}; ///< true if patterns are to be applied randomly
        bool on_patches {false}; ///< true if planting is applied on within-stand patches
        Expression* patches {nullptr}; ///< which patches to apply the planting to
        double *patch_var; ///< for the expression
        bool setup(QJSValue value);
        void run(FMStand *stand);
    };
    QVector<SPlantingItem> mItems;
    bool mRequireLoading;

    static QStringList mAllowedProperties;


};

} // end namespace
#endif // ACTPLANTING_H
