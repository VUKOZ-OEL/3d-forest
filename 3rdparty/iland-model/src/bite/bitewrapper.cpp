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
#include "bitewrapper.h"

#include "bitecell.h"
#include "biteagent.h"
#include "bitelifecycle.h"

#include <QStringList>

namespace BITE {


BiteWrapperCore::~BiteWrapperCore()
{
    //for (auto &s : mVarObj) {
    //    if (s.first == VarDoubleGrid)
    //        delete static_cast<Grid<double>* >(s.second);
    //}
    mVarObj.clear();
}

void BiteWrapperCore::registerGridVar(Grid<double> *grid, QString var_name)
{
    if (mVariables.contains(var_name))
        throw IException(QString("Variable '%1' (for a grid) already in the list of BiteCell variables!").arg(var_name));

    mVariables.push_back(var_name);
    mVarObj.push_back( QPair<EVarType, void*> (VarDoubleGrid, static_cast<void*>(grid)) );
}

void BiteWrapperCore::registerClimateVar(int var_index, QString var_name)
{
    if (mVariables.contains(var_name))
        throw IException(QString("Variable '%1' (for a climate variables) already in the list of BiteCell variables!").arg(var_name));
    mVariables.push_back(var_name);
    mVarObj.push_back( QPair<EVarType, void*>(VarClimate, static_cast<void*>(this+var_index))); // trick to store a int in a void*

}

const QStringList BiteWrapperCore::getVariablesList()
{
    return mVariables;
}

double BiteWrapperCore::valueCell(const int variableIndex, const BiteCell *cell)
{
    switch (mVarObj[variableIndex].first) {
    case VarDoubleGrid: {
        Grid<double>*g = static_cast<Grid<double>* >( mVarObj[variableIndex].second );
        return g->constValueAtIndex(cell->index());
    }

    case VarNone:
        switch (variableIndex) {
        case 0: return cell->index();
        case 1: return cell->isActive() ? 1. : 0.;
        case 2: return cell->isSpreading() ? 1. : 0.;
        case 3: return cell->yearsLiving();
        case 4: return cell->cumYearsLiving();
        case 5: return cell->agent()->lifeCycle()->outbreakYears();
        default: throw IException("Invalid variable index");
        }
    case VarClimate: {
        int var_idx  = static_cast<int>( static_cast<BiteWrapperCore*>(mVarObj[variableIndex].second) - this ); // get the int back
        return cell->climateVar(var_idx);
    }

    }
    return 0.;
}

void BiteWrapperCore::setValueCell(const int variableIndex, const BiteCell *cell, double new_value)
{
    if (variableIndex<0 || variableIndex>=mVariables.size())
        throw IException("Invald setValueCell index");

    switch (mVarObj[variableIndex].first) {
    case VarDoubleGrid: {
        Grid<double>*g = static_cast<Grid<double>* >( mVarObj[variableIndex].second );
        g->valueAtIndex(cell->index()) = new_value;
        return;
    }

    case VarNone: {
        BiteCell *c = const_cast<BiteCell*>(cell);
        switch (variableIndex) {
        case 0: throw IException("setValueCell: read-only property: index");
        case 1: c->setActive( new_value == 1. ); break;
        case 2: c->setSpreading( new_value == 1. ); break;
        case 3: throw IException("setValueCell: read-only property: yearsLiving");
        }
        return;
    }
    case VarClimate:
        throw IException("setValueCell: attempting to update a climate variable: climate variables are read only!");
    }

    qDebug() << "setValueCell called";
}

Grid<double> *BiteWrapperCore::grid(QString var_name)
{
    int varidx = mVariables.indexOf(var_name);
    if (varidx<0)
        return nullptr;
    if( mVarObj[varidx].first == VarDoubleGrid )
        return static_cast<Grid<double>* >( mVarObj[varidx].second );
    else
        return nullptr;

}

void BiteWrapperCore::buildVarList()
{
    // standard variables
    mVariables.push_back("index"); // index 0
    mVarObj.push_back(QPair<EVarType, void*>(VarNone, nullptr));
    mVariables.push_back("active"); // index 1
    mVarObj.push_back(QPair<EVarType, void*>(VarNone, nullptr));
    mVariables.push_back("spreading"); // index 2
    mVarObj.push_back(QPair<EVarType, void*>(VarNone, nullptr));
    mVariables.push_back("yearsLiving"); // index 3
    mVarObj.push_back(QPair<EVarType, void*>(VarNone, nullptr));
    mVariables.push_back("cumYearsLiving"); // index 4
    mVarObj.push_back(QPair<EVarType, void*>(VarNone, nullptr));
    mVariables.push_back("outbreakYears"); // index 5
    mVarObj.push_back(QPair<EVarType, void*>(VarNone, nullptr));

}




} // end namespace
