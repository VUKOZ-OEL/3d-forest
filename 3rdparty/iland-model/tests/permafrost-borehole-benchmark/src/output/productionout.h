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

#ifndef PRODUCTIONOUT_H
#define PRODUCTIONOUT_H
#include "output.h"
#include "expression.h"
class ResourceUnitSpecies;
/** ProductionOut describes finegrained production details on the level of resourceunits per month. */
class ProductionOut : public Output
{
public:
    ProductionOut();
    void execute(const ResourceUnitSpecies *rus);
    virtual void exec();
    virtual void setup();
};

#endif // PRODUCTIONOUT_H
