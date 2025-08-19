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
#ifndef BITEBIOMASS_H
#define BITEBIOMASS_H

#include "biteitem.h"
#include "bitecellscript.h"
#include "grid.h"

namespace BITE {

class BiteBiomass: public BiteItem
{
    Q_OBJECT

public:
    Q_INVOKABLE BiteBiomass(QJSValue obj);
    void setup(BiteAgent *parent_agent);
    QString info();
    void notify(BiteCell *cell, BiteCell::ENotification what);


public slots:
    void runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist);
    void beforeRun();
    void afterSetup();

protected:
    QStringList allowedProperties();

    Events mEvents;
private:
    void calculateLogisticGrowth(BiteCell *cell);
    Grid<double> mHostBiomass;
    Grid<double> mAgentBiomass;
    Grid<double> mImpact;
    QString mHostTreeFilter;
    DynamicExpression mCalcHostBiomass; // calculate host biomass based on trees / cells
    DynamicExpression mMortality;
    Expression mGrowthFunction; // (logistic) growth function
    DynamicExpression mGrowthRateFunction; // function to calculate the growth rate 'r'
    int mGrowthIterations;  // number of iterations during a time step (year) for updating agent/host biomass
    DynamicExpression mGrowthConsumption;  // consumption rate: cons= kg host biomass / kg agent per year
    bool mVerbose;

};


} // end namespace
#endif // BITEBIOMASS_H
