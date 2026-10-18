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
#ifndef BITECOLONIZATION_H
#define BITECOLONIZATION_H

#include "biteitem.h"
#include "bitecellscript.h"

namespace BITE {

class BiteCell; // forward

class BiteColonization: BiteItem
{
    Q_OBJECT
    //Q_PROPERTY(ScriptGrid* grid READ grid)
public:

    BiteColonization();
    Q_INVOKABLE BiteColonization(QJSValue obj);
    void setup(BiteAgent *parent_agent);
    void afterSetup();

    void runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist);
protected:
    QStringList allowedProperties();

private:
    Constraints mCellConstraints;
    Constraints mTreeConstraints;
    Constraints mSaplingConstraints;
    DynamicExpression mDispersalFilter;
    Events mEvents;
    DynamicExpression mInitialAgentBiomass;
    int iAgentBiomass;

};

} // end namespace
#endif // BITECOLONIZATION_H
