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

#ifndef CLIMATE_H
#define CLIMATE_H

#include <QtSql>
#include "phenology.h"
/// current climate variables of a day. @sa Climate.
/// https://iland-model.org/ClimateData
struct ClimateDay
{
    int year; // year
    int month; // month (1..12)
    int dayOfMonth; // day of the month (1..31)
    double temperature; // average day  degree C (of the light hours)
    double min_temperature; // minimum temperature of the day
    double max_temperature; // maximum temperature of the day
    double mean_temp() const { return (min_temperature + max_temperature) / 2.; } // mean temperature
    double temp_delayed; // temperature delayed (after Maekela, 2008) for response calculations
    double preciptitation; // sum of day [mm]
    double radiation; // sum of day (MJ/m2)
    double vpd; // average of day [kPa] = [0.1 mbar] (1 bar = 100kPa)
    static double co2; // ambient CO2 content in ppm
    QString toString() const { return QString("%1.%2.%3").arg(dayOfMonth).arg(month).arg(year); }
    bool isValid() const  { return (year>=0); }
    int id() const { return year*10000 + month*100 + dayOfMonth; }
};

/// Sun handles calculations of day lengths, etc.
class Sun
{
public:
    void setup(const double latitude_rad);
    QString dump();
    const double &daylength(const int day) const { return mDaylength_h[day]; }
    int longestDay() const { return mDayWithMaxLength; }
    bool northernHemishere() const { return mDayWithMaxLength<300; }
    int dayShorter10_5hrs() const { return mDayWith10_5hrs; }
    int dayShorter14_5hrs() const { return mDayWith14_5hrs; }
private:
    double mLatitude; ///< latitude in radians
    int mDayWithMaxLength; ///< day of year with maximum day length
    double mDaylength_h[366]; ///< daylength per day in hours
    int mDayWith10_5hrs; // last day of year with a day length > 10.5 hours (see Establishment)
    int mDayWith14_5hrs; // last doy with at least 14.5 hours of day length
};

class Climate
{
public:
    Climate();
    void setup(); ///< setup routine that opens database connection
    bool isSetup() const { return mIsSetup; }
    const QString &name() const { return mName; } ///< table name of this climate
    // activity
    void nextYear();
    // access to climate data
    const ClimateDay *dayOfYear(const int dayofyear) const { return mBegin + dayofyear;} ///< get pointer to climate structure by day of year (0-based-index)
    const ClimateDay *day(const int month, const int day) const; ///< gets pointer to climate structure of given day (0-based indices, i.e. month=11=december!)
    int whichDayOfYear(const ClimateDay *climate) const {return climate-mBegin; } ///< get the 0-based index of the climate given by 'climate' within the current year
    /// returns two pointer (arguments!!!) to the begin and one after end of the given month (month: 0..11)
    void monthRange(const int month, const ClimateDay **rBegin, const ClimateDay **rEnd) const;
    double days(const int month) const; ///< returns number of days of given month (0..11)
    int daysOfYear() const; ///< returns number of days of current year.
    const ClimateDay *begin() const { return mBegin; } ///< STL-like (pointer)-iterator  to the first day of the current year
    const ClimateDay *end() const { return mEnd; } ///< STL-like pointer iterator to the day *after* last day of the current year
    void toDate(const int yearday, int *rDay=0, int *rMonth=0, int *rYear=0) const; ///< decode "yearday" to the actual year, month, day if provided
    //
    double totalRadiation() const { return mAnnualRadiation; } ///< return radiation sum (MJ) of the whole year
    const double* precipitationMonth() const { return mPrecipitationMonth; }
    /// the mean annual temperature of the current year (degree C)
    double meanAnnualTemperature() const { return mMeanAnnualTemperature; }
    /// annual precipitation sum (mm)
    double annualPrecipitation() const { double r=0.; for (int i=0;i<12;++i) r+=mPrecipitationMonth[i]; return r;}
    /// get a array with mean temperatures per month (deg C)
    const double *temperatureMonth() const { return mTemperatureMonth; }
    /// retrieve the year provided in the climate table
    int climateDataYear() const { return mBegin->year; }
    // access to other subsystems
    int phenologyGroupCount() const { return mPhenology.count(); }
    const Phenology &phenology(const int phenologyGroup) const; ///< phenology class of given type
    const Sun &sun() const { return mSun; } ///< solar radiation class
    double daylength_h(const int doy) const { return sun().daylength(doy); } ///< length of the day in hours

private:
    bool mIsSetup;
    bool mDoRandomSampling; ///< if true, the sequence of years is randomized
    bool mTMaxAvailable; ///< tmax is part of the climate data
    QString mName;
    Sun mSun; ///< class doing solar radiation calculations
    void load(); ///< load mLoadYears years from database
    void setupPhenology(); ///< setup of phenology groups
    void climateCalculations(const ClimateDay &lastDay); ///< more calculations done after loading of climate data
    ClimateDay mInvalidDay;
    int mLoadYears; // count of years to load ahead
    int mCurrentYear; // current year (relative)
    int mMinYear; // lowest year in store (relative)
    int mMaxYear;  // highest year in store (relative)
    double mTemperatureShift; // add this to daily temp
    double mPrecipitationShift; // multiply prec with that
    ClimateDay *mBegin; // pointer to the first day of the current year
    ClimateDay *mEnd; // pointer to the last day of the current year (+1)
    QVector<ClimateDay> mStore; ///< storage of climate data
    QVector<int> mDayIndices; ///< store indices for month / years within store
    QSqlQuery mClimateQuery; ///< sql query for db access
    QList<Phenology> mPhenology; ///< phenology calculations
    QVector<int> mRandomYearList; ///< for random sampling of years
    int mRandomListIndex; ///< current index of the randomYearList for random sampling
    double mAnnualRadiation;  ///< this year's value for total radiation (MJ/m2)
    double mPrecipitationMonth[12]; ///< this years preciptitation sum (mm) per month
    double mTemperatureMonth[12]; ///< this years average temperature per month
    double mMeanAnnualTemperature; ///< mean temperature of the current year
    static QVector<int> sampled_years; ///< list of sampled years to use
};

#endif // CLIMATE_H
