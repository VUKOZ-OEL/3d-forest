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
#ifndef FOMEWRAPPER_H
#define FOMEWRAPPER_H

#include "expressionwrapper.h"

namespace ABE {
class FMStand;


/** FOMEWrapper provides the context for the Forest Management Engine
 *  This wrapper blends activties, stand variables, and agent variables together.
*/

class FOMEWrapper: public ExpressionWrapper
{
public:
    FOMEWrapper(): mStand(0)  {}
    FOMEWrapper(const FMStand *stand):  mStand(stand) {}
    virtual const QStringList getVariablesList();
    virtual double value(const int variableIndex);

private:
    void buildVarList();
    double valueActivity(const int variableIndex);
    double valueStand(const int variableIndex);
    double valueSite(const int variableIndex);
    const FMStand *mStand;
};

} // namespace

#endif // FOMEWRAPPER_H
