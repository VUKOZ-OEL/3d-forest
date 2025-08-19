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

#include "floatingaverage.h"
#include <QtCore>

FloatingAverage::FloatingAverage()
{
        mCurrentAverage=0;
        mSize=0;
        mInitValue = 0.;
        mPos=-1;
}

void FloatingAverage::setup(const int size, const double InitValue)
{
       mInitValue = InitValue;
       mSize=size;
       mData.resize(mSize);
       mPos=-1;
       mCurrentAverage=0;
       mFilled=false;
       for (int i=0; i<size; i++)
           mData[i]=mInitValue;
}


double FloatingAverage::add(double add_value)
{

    mPos++;
    if (mPos>=mSize) {
        mPos=0;      // rollover again
        mFilled=true;
    }
    mData[mPos]=add_value;

    int countto=mSize;
    if (!mFilled)
        countto=mPos+1;
    double sum=0;
    for (int i=0;i<countto; i++)
        sum+=mData[i];
    if (countto)
        mCurrentAverage = sum/countto;
    else
        mCurrentAverage = mInitValue; // kann sein, wenn als erster wert 0 uebergeben wird.
    return mCurrentAverage;
}

double FloatingAverage::sum() const
{
     if (mFilled)
        return mCurrentAverage * mSize;
     else
        return mCurrentAverage * (mPos+1);
}
