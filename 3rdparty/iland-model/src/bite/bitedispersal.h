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
#ifndef BITEDISPERSAL_H
#define BITEDISPERSAL_H

#include "biteitem.h"
#include "grid.h"
#include "scriptgrid.h"
#include "bitecellscript.h"


namespace BITE {

class BiteDispersal: public BiteItem
{
    Q_OBJECT
    Q_PROPERTY(ScriptGrid* grid READ grid)

public:
    //BiteDispersal();
    Q_INVOKABLE BiteDispersal(QJSValue obj);
    void setup(BiteAgent *parent_agent);
    ScriptGrid *grid() { Q_ASSERT(mScriptGrid != nullptr); return mScriptGrid; }

    QString info();

public slots:
    // actions
    void run();
    void decide();
protected:
    QStringList allowedProperties();
private:
    /// build the dispersal kernel
    void setupKernel(QString expr, double max_dist, QString dbg_file);
    /// apply the spread kernel (probabilistically)
    void spreadKernel();
    /// prepare the grid
    void prepareGrid();
    /// run background infestation
    void backgroundInfestation();
    Grid<double>  mKernel;
    int mKernelOffset;
    Grid<double> mGrid;
    ScriptGrid *mScriptGrid;
    DynamicExpression mBackgroundInfestationProbability;

    Events mEvents;

};

class BiteDistribution : public BiteItem
{
    Q_OBJECT
    Q_PROPERTY(ScriptGrid* grid READ grid)

public:
    Q_INVOKABLE BiteDistribution(QJSValue obj);
    void setup(BiteAgent *parent_agent);
    ScriptGrid *grid() { Q_ASSERT(mScriptGrid != nullptr); return mScriptGrid; }
    QString info();


public slots:
    void run();
protected:
    QStringList allowedProperties();
private:
    Grid<double> mGrid;
    ScriptGrid *mScriptGrid;

    Events mEvents;

};


} // end namespace
#endif // BITEDISPERSAL_H
