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
#ifndef ACTSALVAGE_H
#define ACTSALVAGE_H

#include "activity.h"
#include "grid.h"

class Expression; // forward
class Tree; // forward
namespace ABE {

class FMSTP; // forward
class FMStand; // forward

class ActSalvage : public Activity
{
public:
    ActSalvage(FMSTP *parent);
    ~ActSalvage();
    QString type() const { return "salvage"; }
    void setup(QJSValue value);
    bool execute(FMStand *stand);
    QStringList info();
    // special functions of salvage activity

    /// return true, if the (disturbed) tree should be harvested by the salvage activity
    bool evaluateRemove(Tree* tree) const;
    bool barkbeetleAttack(FMStand *stand, double generations, int infested_px_ha);

    bool checkSanitation(FMStand *stand);
private:
    void checkStandAfterDisturbance(FMStand *stand);
    int floodFillHelper(Grid<int> &grid, QPoint start, int old_color, int color);
    int neighborFinderHelper(Grid<int> &grid, QVector<int> &neighbors, int stand_id);
    int replaceValueHelper(Grid<int> &grid, int old_value, int new_value);
    bool mDebugSplit;
    Expression *mCondition; ///< formula to determine which trees should be harvested
    Expression *mSanitationCondition; ///< formula to determine which trees should be sanitation treated
    int mMaxPreponeActivity; ///< no of years that a already scheduled (regular) activity is 'preponed'
    double mThresholdMinimal; ///< lower threshold (below no action is taken) in m3/ha
    double mThresholdSplit; ///<threshold (relative damage, 0..1) when a split of the stand should be initiated
    double mThresholdClear; ///<threshold (relative damage, 0..1) when a stand should be completely cleared
    bool mJSCondition; ///< true if the check for salvaging of a tree is done in JS
    QJSValue mJSConditionFunc; ///< JS function to call

};


} // namespace
#endif // ACTSALVAGE_H
