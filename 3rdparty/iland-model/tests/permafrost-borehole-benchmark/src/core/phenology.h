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

#ifndef PHENOLOGY_H
#define PHENOLOGY_H
class Climate;

class Phenology
{
public:
    Phenology(const Climate* climate) {mClimate=climate; mId=0; mMinVpd=mMaxVpd=mMinDayLength=mMaxDayLength=mMinTemp=mMaxTemp=0.; mDayStart=0;mDayEnd=365;mChillDaysBefore=-1; mChillDaysAfter=0;mChillDaysAfterLastYear=0;}
    Phenology(const int id, const Climate* climate, const double minVpd, const double maxVpd,
              const double minDayLength, const double maxDayLength,
              const double minTemp, const double maxTemp): mId(id), mClimate(climate), mMinVpd(minVpd), mMaxVpd(maxVpd),
                                mMinDayLength(minDayLength), mMaxDayLength(maxDayLength), mMinTemp(minTemp), mMaxTemp(maxTemp),mChillDaysAfter(0),mChillDaysAfterLastYear(0) {}
    // access
    int id() const { return mId; }
    /// calculate the phenology for the current year
    void calculate();
    /// get result of phenology calcualtion for this year (a pointer to a array of 12 values between 0..1: 0: no days with foliage)
    const double *month() const { return mPhenoFraction; }
    int vegetationPeriodLength() const { return mDayEnd - mDayStart; } ///< length of vegetation period in days, returs 365 for evergreens
    int vegetationPeriodStart() const { return mDayStart; } ///< day of year when vegeation period starts
    int vegetationPeriodEnd() const { return mDayEnd; } ///< day of year when vegeation period stops
    // chilling days
    /// get days of year that meet chilling requirements: the days in the autumn of the last year + the days of this spring season
    int chillingDays() const { return mChillDaysBefore + mChillDaysAfterLastYear; }
    int chillingDaysLastYear() const { return mChillDaysAfterLastYear; }


private:
    int mId; ///< identifier of this Phenology group
    const Climate *mClimate; ///< link to relevant climate source
    double mMinVpd; ///< minimum vpd [kPa]
    double mMaxVpd; ///< maximum vpd [kPa]
    double mMinDayLength; ///< minimum daylength [hours]
    double mMaxDayLength; ///< maximum daylength [hours]
    double mMinTemp; ///< minimum temperature [deg]
    double mMaxTemp; ///< maximum temperature [deg]
    double mPhenoFraction[12]; ///< fraction [0..1] of month i [0..11] to are inside the vegetation period, i.e. have leafs
    int mDayStart; ///< start of vegetation period (in day of year)
    int mDayEnd; ///< end of vegetation period (in days of year, 1.1. = 0)
    // some special calculations used for establishment
    void calculateChillDays(const int end_of_season=-1);
    int mChillDaysBefore, mChillDaysAfter; ///< number of days that meet chilling requirements (>-5 deg C, <+5 deg C) before and after the vegetation period in this yeaer
    int mChillDaysAfterLastYear; ///< chilling days of the last years autumn/winter
};

#endif // PHENOLOGY_H
