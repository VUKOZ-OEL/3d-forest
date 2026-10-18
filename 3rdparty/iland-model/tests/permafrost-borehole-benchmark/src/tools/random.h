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

#ifndef RANDOMTOOLS_H
#define RANDOMTOOLS_H

#include <QtCore/QString>

// RandomIndex: get indicies in a random order
class RandomIndex
{
public:
        RandomIndex(int aCount); ///< creates a index with aCount entries.
        ~RandomIndex();
        bool next(); ///< retrieve next index. return false if all indices used.
        int index() const { return mIndex; } ///< retrieve (random) index
private:
        int mCount;
        int mIndex; ///< currently selected
        char *mField;
        int  mRemaining;
};

class RandomWeighted
{
public:
        RandomWeighted();
        ~RandomWeighted();
        void setup(const int gridSize);
        void setWeight(const int index, const int value);
        int get();
        double getRelWeight(const int index);
        double getRelWeight(const int from, const int to);
private:
        int *mGrid;
        int mSize;
        int mMemorySize;
        int mMaxVal;
        bool mUpdated;
        void updateValues();

};

class Expression;
class RandomCustomPDF
{
public:
        RandomCustomPDF();
        RandomCustomPDF(const QString &densityFunction){ mExpression=0; setup(densityFunction);}
        ~RandomCustomPDF();
        void setup(const QString &funcExpr, const double lowerBound=0., const double upperBound=1., const bool isSumFunc=false, const int stepCount=100);
        // properties
        const QString &densityFunction() const { return mFunction; }
        // operation
        double get(); ///< get a random number
        double getProbOfRange(const double lowerBound, const double upperBound); ///< get probability of random numbers between given bounds.
private:
        QString mFunction;
        RandomWeighted mRandomIndex;
        Expression *mExpression;
        int mSteps;
        double mLowerBound, mUpperBound;
        double mDeltaX;
        bool mSumFunction;
};


#endif // RANDOMTOOLS_H
