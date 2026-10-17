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
#ifndef BITELIFECYCLE_H
#define BITELIFECYCLE_H

#include "biteitem.h"
#include "bitecellscript.h"

namespace BITE {

class BiteLifeCycle: public BiteItem
{
    Q_OBJECT

public:
    Q_INVOKABLE BiteLifeCycle(QJSValue obj);

    void setup(BiteAgent *parent_agent);
    QString info();
    void notify(BiteCell *cell, BiteCell::ENotification what);
    void run();


    bool dieAfterDispersal() const { return mDieAfterDispersal; }

    /// fetch the number of cycles the agent should run for the cell
    int numberAnnualCycles(BiteCell *cell);

    /// should the cell be an active spreader in the next iteration?
    bool shouldSpread(BiteCell *cell);

    /// number of years that an outbreak wave is currently active
    /// the value is 0 for non-outbreak years
    int outbreakYears() { return mOutbreakYears; }

protected:
    QStringList allowedProperties();
private:
    void calcOutbreakWaves();
    DynamicExpression mSpreadFilter;
    DynamicExpression mVoltinism;
    DynamicExpression mSpreadInterval;
    DynamicExpression mMortality;
    int mSpreadDelay;
    bool mDieAfterDispersal;

    // outbreak waves
    DynamicExpression mOutbreakDuration;
    DynamicExpression mOutbreakStart;
    int mOutbreakYears; // number of years in an outbreak
    int mThisOutbreakDuration;
    int mNextOutbreakStart;

    Events mEvents;

};

} // end namespace
#endif // BITELIFECYCLE_H
