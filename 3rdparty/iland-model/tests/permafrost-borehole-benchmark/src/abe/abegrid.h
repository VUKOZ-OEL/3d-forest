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
#ifndef ABEGRID_H
#define ABEGRID_H
#include "layeredgrid.h"
namespace ABE {
class FMStand; // forward
class Agent; // forward
}

/** Helper class for visualizing ABE management data.
*/
typedef ABE::FMStand* FMStandPtr;
class ABELayers: public LayeredGrid<FMStandPtr> {
  public:
    ~ABELayers();
    void setGrid(Grid<FMStandPtr> &grid) { mGrid = &grid; }
    double value(const FMStandPtr &data, const int index) const;
    const QVector<LayeredGridBase::LayerElement> &names();
    const QString labelvalue(const int value, const int index) const;
    void registerLayers();
    void clearClasses(); // clear ID and agent classes...
private:
    QVector<LayeredGridBase::LayerElement> mNames;
    mutable QHash<const ABE::Agent*, int > mAgentIndex;
    mutable QHash<QString, int> mUnitIndex;
    mutable QHash<int, int> mStandIndex;
    mutable QHash<QString, int> mSTPIndex;
    mutable QHash<QString, int> mActivityIndex;
};



#endif // ABEGRID_H
