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
#include "global.h"
#include "statdata.h"

// StatData
StatData::StatData(QVector<double> &data)
{
    mData=data;
    calculate();
}

void StatData::calculatePercentiles() const
{
    mP25 = percentile(25);
    mP75 = percentile(75);
    mMedian = percentile(50);
}

void StatData::calculate()
{
   if (mData.isEmpty()) {
       mSum=mMedian=mP25=mP75=mMean=mMin=mMax=0.;
       return;
   }
   mP25 = std::numeric_limits<double>::max();
   mP75 = std::numeric_limits<double>::max();
   mMedian = std::numeric_limits<double>::max();
   mMin = std::numeric_limits<double>::max();
   mMax = - std::numeric_limits<double>::max();
   mSD = std::numeric_limits<double>::max();

   QVector<double>::const_iterator end = mData.constEnd();
   QVector<double>::const_iterator i = mData.constBegin();
   mSum = 0.;
   while (i!=end) {
       mSum += *i;
       mMin = qMin(*i, mMin);
       mMax = qMax(*i, mMax);
       ++i;
   }
   mMean = mSum / double(mData.count());
   //qDebug() << QString("p25: %1 Median: %2 p75: %3 min: %4 max: %5").arg(mP25).arg(mMedian).arg(mP75).arg(mMin).arg(mMax);
}

double StatData::calculateSD() const
{
    if (mData.count()==0) {
        mSD = 0.;
        return 0.;
    }
    // calculate the standard deviation...
    QVector<double>::const_iterator end = mData.constEnd();
    QVector<double>::const_iterator i = mData.constBegin();
    double sum = 0.;
    while (i!=end) {
        sum += (*i - mMean)*(*i - mMean);
        ++i;
    }
    mSD =sqrt(sum / double(mData.count()));
    return mSD;
}

double StatData::percentile(const int percent) const
{
// double *Values, int ValueCount,
    // code von: Fast median search: an ANSI C implementation, Nicolas Devillard, http://ndevilla.free.fr/median/median/index.html
        // algo. kommt von Wirth, hier nur an c++ angepasst.

    int perc = limit(percent, 1, 99);
    int ValueCount = mData.count();
    int i,j,l,m, n, k ;
    double x, temp ;
    if (ValueCount==0)
      return 0;
    n = ValueCount;
    // k ist der "Index" des gesuchten wertes
    if (perc!=50) {
        // irgendwelche perzentillen
        int d = 100 / ( (perc>50?(100-perc):perc) );
        k = ValueCount / d;
        if (perc>50)
          k=ValueCount - k - 1;
    } else {
        // median
        if (ValueCount & 1)  // gerade/ungerade?
          k = ValueCount / 2 ;  // mittlerer wert
        else
          k= ValueCount / 2 -1; // wert unter der mitte
    }
    l=0 ; m=n-1 ;
    while (l<m) {
        x=mData[k] ;
        i=l ;
        j=m ;
        do {
            while (mData[i]<x) i++ ;
            while (x<mData[j]) j-- ;
            if (i<=j) {
                //ELEM_SWAP(a[i],a[j]) ; swap elements:
                temp = mData[i]; mData[i]=mData[j]; mData[j]=temp;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return mData[k] ;

}

/** calculate Ranks.
  @param data values for N items,
  @param descending true: better ranks for lower values
  @return a vector that contains for the Nth data item the resulting rank.
  Example: in: {5, 2, 7, 5}
           out: {2, 1, 4, 2}
  */
QVector<int> StatData::calculateRanks(const QVector<double> &data, bool descending)
{
   // simple ranking algorithm.
   // we have "N" data-values.
   // rank := N - (N smaller or equal)
   int i, j;
   int smaller;
   QVector<int> ranks;
   ranks.resize(data.count());
   int n=data.count();
   for (i=0;i<n;i++) {
       smaller = 0;
       for (j=0;j<n;j++) {
          if (i==j)
             continue;
          if (data[j]<=data[i])
             smaller++;
       }
       if (descending) // better rank if lower value...
          ranks[i] = smaller + 1;
       else
          ranks[i] = n - smaller;  // better rank if value is higher...
   }
   return ranks;
}

/** scale the data in such a way that the sum of all data items is "targetSum"
  */
void StatData::normalize(QVector<double> &data, double targetSum)
{
    QVector<double>::iterator i, end=data.end();
    double sum=0.;
    for (i=data.begin(); i!=end; ++i)
        sum+=*i;

    if (sum!=0) {
        double m = targetSum / sum;
        for (i=data.begin(); i!=end; ++i)
        *i *= m;
    }
}
