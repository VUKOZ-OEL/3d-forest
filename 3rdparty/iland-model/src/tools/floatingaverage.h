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

#ifndef FLOATINGAVERAGE_H
#define FLOATINGAVERAGE_H
#include <QVector>
/** Helper class for floating averages.
  Use add(new_value) to add a value (and get the the current average). average() returns the current average
  and sum() the total sum of stored values. Use setup() to setup place for "size" values. */
class FloatingAverage
{
public:
    FloatingAverage();
    FloatingAverage(int size) { setup(size); }
    void setup(const int size, const double InitValue = 0.);
    double add(double add_value); ///< add a value and return current average

    double average() const {return mCurrentAverage; } ///< retrieve current average
    double sum() const; ///< retrieve total sum of values.
private:
    double mCurrentAverage;
    QVector<double> mData;
    int    mSize;
    int    mPos;
    bool   mFilled;
    double mInitValue;

};
#endif // FLOATINGAVERAGE_H
