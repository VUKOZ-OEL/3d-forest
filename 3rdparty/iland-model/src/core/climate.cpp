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


/** @class Climate
  Climate handles climate input data and performs some basic related calculations on that data.
  @ingroup core
  @sa https://iland-model.org/ClimateData
*/
#include "global.h"
#include "climate.h"
#include "model.h"
#include "timeevents.h"
#include "csvfile.h"

QVector<int> Climate::sampled_years; // list of sampled years to use

double ClimateDay::co2 = 350.; // base value of ambient CO2-concentration (ppm)
int Climate::co2Startyear = 1980;
QString Climate::co2Pathway;
QMap<QString, QVector<double> > Climate::fixedCO2concentrations;


void Sun::setup(double latitude_rad)
{
    mLatitude = latitude_rad;
    if (mLatitude>0)
        mDayWithMaxLength = 182-10; // 21st of June
    else
        mDayWithMaxLength = 365-10; //southern hemisphere
    // calculate length of day using  the approximation formulae of: http://herbert.gandraxa.com/length_of_day.aspx
    const double j = M_PI / 182.625;
    const double ecliptic = RAD(23.439);
    double m;
    for (int day=0;day<366;day++) {
        m = 1. - tan(latitude_rad)*tan(ecliptic*cos(j*(day+10))); // day=0: winter solstice => subtract 10 days
        m = limit(m, 0., 2.);
        mDaylength_h[day] = acos(1-m)/M_PI * 24.; // result in hours [0..24]
    }
    mDayWith10_5hrs = 0;
    for (int day=mDayWithMaxLength;day<366;day++) {
        if (mDaylength_h[day]<10.5) {
            mDayWith10_5hrs = day;
            break;
        }
    }
    mDayWith14_5hrs = 0;
    for (int day=mDayWithMaxLength;day<366;day++) {
        if (mDaylength_h[day]<14.5) {
            mDayWith14_5hrs = day;
            break;
        }
    }
}

QString Sun::dump()
{
    QString result=QString("lat: %1, longest day: %2\ndoy;daylength").arg(mLatitude).arg(mDayWithMaxLength);
    for (int i=0;i<366;i++)
        result+=QString("\n%1;%2").arg(i).arg(mDaylength_h[i]);
    return result;
}

Climate::Climate()
{
    mLoadYears = 1;
    mInvalidDay.dayOfMonth=mInvalidDay.month=mInvalidDay.year=-1;
    mBegin = mEnd = 0;
    mIsSetup = false;
}


// access functions

const ClimateDay *Climate::day(const int month, const int day) const
{
    if (mDayIndices.isEmpty())
        return &mInvalidDay;
    return mStore.data() + mDayIndices[mCurrentYear*12 + month] + day;
}
void Climate::monthRange(const int month, const ClimateDay **rBegin, const ClimateDay **rEnd) const
{
    *rBegin = mStore.data() + mDayIndices[mCurrentYear*12 + month];
    *rEnd = mStore.data() + mDayIndices[mCurrentYear*12 + month+1];
    //qDebug() << "monthRange returning: begin:"<< (*rBegin)->toString() << "end-1:" << (*rEnd-1)->toString();
}

double Climate::days(const int month) const
{
    return (double) mDayIndices[mCurrentYear*12 + month + 1]-mDayIndices[mCurrentYear*12 + month];
}
int Climate::daysOfYear() const
{
    if (mDayIndices.isEmpty())
        return -1;
    return mEnd - mBegin;
}

void Climate::toDate(const int yearday, int *rDay, int *rMonth, int *rYear) const
{
    const ClimateDay *d = dayOfYear(yearday);
    if (rDay) *rDay = d->dayOfMonth-1;
    if (rMonth) *rMonth = d->month-1;
    if (rYear) *rYear = d->year;
}

void Climate::setup(bool do_log)
{
    GlobalSettings *g=GlobalSettings::instance();
    XmlHelper xml(g->settings().node("model.climate"));
    QString tableName =xml.value("tableName");

    mName = tableName;
    QString filter = xml.value("filter");

    mLoadYears = (int) qMax(xml.valueDouble("batchYears", 1.),1.);
    mDoRandomSampling = xml.valueBool("randomSamplingEnabled", false);
    mRandomYearList.clear();
    mRandomListIndex = -1;
    QString list = xml.value("randomSamplingList");
    if (mDoRandomSampling) {
        if (!list.isEmpty()) {
            QStringList strlist = list.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
            foreach(const QString &s,strlist)
                mRandomYearList.push_back(s.toInt());
            // check for validity
            foreach(int year, mRandomYearList)
                if (year < 0 || year>=mLoadYears)
                    throw IException(QString("Setup climate: invalid 'randomSamplingList'! \nYear numbers are 0-based and must to between 0 and batchYears-1 (check value of 'batchYears')!\n Tried to access: '%1', batchYears: '%2'").arg(year).arg(mLoadYears) );
        }
        if (do_log) {
            if (mRandomYearList.count()>0)
                qDebug() << "Climate: Random sampling enabled with fixed list" << mRandomYearList.count() << "of years.";
            else
                qDebug() << "Climate: Random sampling enabled (without a fixed list)." ;
            }
    }
    mTemperatureShift = xml.valueDouble("temperatureShift", 0.);
    mPrecipitationShift = xml.valueDouble("precipitationShift", 1.);
    if (mTemperatureShift!=0. || mPrecipitationShift!=1.)
        if (do_log) qDebug() << "Climate modifaction: add temperature:" << mTemperatureShift << ". Multiply precipitation: " << mPrecipitationShift;

    mStore.resize(mLoadYears * 366 + 1); // reserve enough space (1 more than used at max)
    mCurrentYear=0;
    mMinYear = 0;
    mMaxYear = 0;

    // add a where-clause
    if (!filter.isEmpty()) {
        filter = QString("where %1").arg(filter);
        if (do_log) qDebug() << "adding climate table where-clause:" << filter;
    }

    QString query=QString("select year,month,day,min_temp,max_temp,prec,rad,vpd from '%1' %2 order by year, month, day").arg(tableName).arg(filter);
    // here add more options...
    mClimateQuery = QSqlQuery(g->dbclimate());
    mClimateQuery.exec(query);
    mTMaxAvailable = true;
    if (mClimateQuery.lastError().isValid()){
        // fallback: if there is no max_temp try the older format:
        QString errmsg = mClimateQuery.lastError().text();
        QString query_fb=QString("select year,month,day,temp,min_temp,prec,rad,vpd from '%1' order by year, month, day").arg(tableName);
        mClimateQuery.exec(query_fb);
        mTMaxAvailable = false;
        if (mClimateQuery.lastError().isValid()){
            throw IException(QString("Error setting up climate: %1 \n %2 (\n\ntried also fallback '%4' and got: '%3')").arg(query, errmsg, mClimateQuery.lastError().text(), query_fb) );
        }
    }
    // setup query
    // load first chunk...
    load();
    setupPhenology(); // load phenology
    // setup sun
    mSun.setup(Model::settings().latitude);
    mCurrentYear--; // go to "-1" -> the first call to next year will go to year 0.
    sampled_years.clear();

    co2Pathway = xml.value("co2pathway", "No");
    co2Startyear = xml.valueInt("co2startYear", 1980);
    if (co2Pathway == "No")
        co2Pathway = QString();
    mIsSetup = true;
}


void Climate::load()
{
    if (!mClimateQuery.isActive())
       throw IException(QString("Error loading climate file - query not active."));

    ClimateDay lastDay = *day(11,30); // 31.december
    mMinYear = mMaxYear;
    ClimateDay *store=mStore.data();

    mDayIndices.clear();
    ClimateDay *cday = store;
    int lastmon = -1;
    int lastyear = -1;
    int yeardays;
    for (int i=0;i<mLoadYears;i++) {
        yeardays = 0;
        if (GlobalSettings::instance()->model()->timeEvents()) {
            QVariant val_temp = GlobalSettings::instance()->model()->timeEvents()->value(GlobalSettings::instance()->currentYear() + i, "model.climate.temperatureShift");
            QVariant val_prec = GlobalSettings::instance()->model()->timeEvents()->value(GlobalSettings::instance()->currentYear() + i, "model.climate.precipitationShift");
            if (val_temp.isValid())
                mTemperatureShift = val_temp.toDouble();
            if (val_prec.isValid())
                mPrecipitationShift = val_prec.toDouble();

            if (mTemperatureShift!=0. || mPrecipitationShift!=1.) {
                qDebug() << "Climate modification: add temperature:" << mTemperatureShift << ". Multiply precipitation: " << mPrecipitationShift;
                if (mDoRandomSampling) {
                    qWarning() << "WARNING - Climate: using a randomSamplingList and temperatureShift/precipitationShift at the same time. The same offset is applied for *every instance* of a year!!";
                    //throw IException("Climate: cannot use a randomSamplingList and temperatureShift/precipitationShift at the same time. Sorry.");
                }
            }
        }

        //qDebug() << "loading year" << lastyear+1;
        while(1==1) {
            if(!mClimateQuery.next()) {
                if (mDoRandomSampling)
                    throw IException(QString("Climate: not enough years in climate database - tried to load %1 years (random sampling of climate is enabled).\n%2").arg(mLoadYears).arg(mClimateQuery.lastQuery()) );

                // rewind to the start of the time series
                qDebug() << "restart of climate table";
                lastyear=-1;
                if (!mClimateQuery.first())
                    throw IException("Error rewinding climate file!");

            }
            yeardays++;
            if (yeardays>366)
                throw IException(QString("Error in reading climate file: Year with >366 days detected! Year: %1, table: '%2'.").arg(cday->year).arg(mName));


            cday = store++; // store values directly in the QVector

            cday->year = mClimateQuery.value(0).toInt();
            cday->month = mClimateQuery.value(1).toInt();
            cday->dayOfMonth = mClimateQuery.value(2).toInt();
            if (mTMaxAvailable) {
                //References for calculation the temperature of the day:
                //Floyd, R. B., Braddock, R. D. 1984. A simple method for fitting average diurnal temperature curves.  Agricultural and Forest Meteorology 32: 107-119.
                //Landsberg, J. J. 1986. Physiological ecology of forest production. Academic Press Inc., 197 S.

                cday->min_temperature = mClimateQuery.value(3).toDouble() + mTemperatureShift;
                cday->max_temperature = mClimateQuery.value(4).toDouble() + mTemperatureShift;
                cday->temperature = 0.212*(cday->max_temperature - cday->mean_temp()) + cday->mean_temp();

            } else {
               // for compatibility: the old method
                cday->temperature = mClimateQuery.value(3).toDouble() + mTemperatureShift;
                cday->min_temperature = mClimateQuery.value(4).toDouble() + mTemperatureShift;
                cday->max_temperature = cday->temperature;
            }
            cday->preciptitation = mClimateQuery.value(5).toDouble() * mPrecipitationShift;
            cday->radiation = mClimateQuery.value(6).toDouble();
            cday->vpd = mClimateQuery.value(7).toDouble();
            // sanity checks
            if (cday->month<1 || cday->dayOfMonth<1 || cday->month>12 || cday->dayOfMonth>31)
                qDebug() << QString("Invalid dates in climate table %1: year %2 month %3 day %4!").arg(name()).arg(cday->year).arg(cday->month).arg(cday->dayOfMonth);
            DBG_IF(cday->month<1 || cday->dayOfMonth<1 || cday->month>12 || cday->dayOfMonth>31,"Climate:load", "invalid dates");
            DBG_IF(cday->temperature<-70 || cday->temperature>50,"Climate:load", "temperature out of range (-70..+50 degree C)");
            DBG_IF(cday->preciptitation<0 || cday->preciptitation>200,"Climate:load", "precipitation out of range (0..200mm)");
            DBG_IF(cday->radiation<0 || cday->radiation>50,"Climate:load", "radiation out of range (0..50 MJ/m2/day)");
            DBG_IF(cday->vpd<0 || cday->vpd>10,"Climate:load", "vpd out of range (0..10 kPa)");

            if (cday->month != lastmon) {
                // new month...
                lastmon = cday->month;
                // save relative position of the beginning of the new month
                mDayIndices.push_back( cday - mStore.data() );
            }
            if (yeardays==1) {
                // check on first day of the year
                if (lastyear!=-1 && cday->year!=lastyear+1)
                    throw IException(QString("Error in reading climate file: invalid year break at y-m-d: %1-%2-%3!").arg(cday->year).arg(cday->month).arg(cday->dayOfMonth));
            }
            if (cday->month==12 && cday->dayOfMonth==31)
                break;

            if (cday >= mStore.data() + mStore.size() )
                throw IException("Error in reading climate file: read across the end!");
        }
        lastyear = cday->year;
    }
    while (store < mStore.data() + mStore.size())
        *store++ = mInvalidDay; // save a invalid day at the end...

    mDayIndices.push_back(cday- mStore.data()); // the absolute last day...
    mMaxYear = mMinYear+mLoadYears;
    mCurrentYear = 0;
    mBegin = mStore.data() + mDayIndices[mCurrentYear*12];
    mEnd = mStore.data() + mDayIndices[(mCurrentYear+1)*12];; // point to the 1.1. of the next year

    climateCalculations(lastDay); // perform additional calculations based on the climate data loaded from the database

}


void Climate::nextYear()
{

    if (!mDoRandomSampling) {
        // default behaviour: simply advance to next year, call load() if end reached
        if (mCurrentYear >= mLoadYears-1) // need to load more data
            load();
        else
            mCurrentYear++;
    } else {
        // random sampling
        if (mRandomYearList.isEmpty()) {
            // random without list
            // make sure that the sequence of years is the same for the full landscape
            if (sampled_years.size()-1<GlobalSettings::instance()->currentYear()) {
                while (sampled_years.size()-1 < GlobalSettings::instance()->currentYear())
                    sampled_years.append(irandom(0,mLoadYears));
            }

            mCurrentYear = sampled_years[GlobalSettings::instance()->currentYear()];

        } else {
            // random with fixed list
            mRandomListIndex++;
            if (mRandomListIndex>=mRandomYearList.count())
                mRandomListIndex=0;
            mCurrentYear = mRandomYearList[mRandomListIndex];
            if (mCurrentYear >= mLoadYears)
                throw IException(QString("Climate: load year with random sampling: the actual year %1 is invalid. Only %2 years are loaded from the climate database.").arg(mCurrentYear).arg(mLoadYears) );
        }
        if (logLevelDebug())
            qDebug() << "Climate: current year (randomized):" << mCurrentYear;
    }

    // update ambient CO2 level
    updateCO2concentration();

    mBegin = mStore.data() + mDayIndices[mCurrentYear*12];
    mEnd = mStore.data() + mDayIndices[(mCurrentYear+1)*12];; // point to the 1.1. of the next year

    // some aggregates:
    // calculate radiation sum of the year and monthly precipitation
    mAnnualRadiation = 0.;
    mMeanAnnualTemperature = 0.;
    for (int i=0;i<12;i++)  {
        mPrecipitationMonth[i]=0.;
        mTemperatureMonth[i]=0.;
    }

    for (const ClimateDay *d=begin();d!=end();++d) {
        mAnnualRadiation+=d->radiation;
        mMeanAnnualTemperature += d->temperature;
        mPrecipitationMonth[d->month-1]+= d->preciptitation;
        mTemperatureMonth[d->month-1] += d->temperature;
    }
    for (int i=0;i<12;++i) {
        mTemperatureMonth[i] /= days(i);
    }
    mMeanAnnualTemperature /= daysOfYear();

    // calculate phenology
    for(int i=0;i<mPhenology.count(); ++i)
        mPhenology[i].calculate();
}

void Climate::climateCalculations(const ClimateDay &lastDay)
{
    ClimateDay *c = mStore.data();
    const double tau = Model::settings().temperatureTau;
    // handle first day: use tissue temperature of the last day of the last year (if available)
    if (lastDay.isValid())
        c->temp_delayed = lastDay.temp_delayed + 1./tau * (c->temperature - lastDay.temp_delayed);
    else
        c->temp_delayed = c->temperature;
    c++;
    while (c->isValid()) {
        // first order dynamic delayed model (Maekela 2008)
        c->temp_delayed=(c-1)->temp_delayed + 1./tau * (c->temperature - (c-1)->temp_delayed);
        ++c;
    }

}

QMutex _loadco2;
void Climate::updateCO2concentration()
{

    if (!co2Pathway.isEmpty()) {

        QMutexLocker lock(&_loadco2);
        if (fixedCO2concentrations.isEmpty()) {

            // load from data file
            CSVFile input_file(":/rcp_co2_1980_2100.txt");

            for (int i=0;i<input_file.rowCount(); ++i) {
                fixedCO2concentrations["RCP2.6"].append(input_file.value(i, "RCP_26").toDouble());
                fixedCO2concentrations["RCP4.5"].append(input_file.value(i, "RCP_45").toDouble());
                fixedCO2concentrations["RCP6.0"].append(input_file.value(i, "RCP_60").toDouble());
                fixedCO2concentrations["RCP8.5"].append(input_file.value(i, "RCP_85").toDouble());
            }
            if (fixedCO2concentrations["RCP2.6"].isEmpty())
                throw IException("Error in loading file with CO2 concentrations!");
        }



        int year = Globals->currentYear();
        // derive actual year: first year of sim should get the value from co2Startyear (first year in the file is 1980)

        int yearindex =  std::max( (year + co2Startyear - 1) -  1980, 0);
        yearindex = std::min(yearindex, static_cast<int>(fixedCO2concentrations[co2Pathway].size()-1));
        if (yearindex < 0)
            throw IException("climate: set co2 concentration: invalid value for co2. Valid values for 'co2pathway' are: 'No', 'RCP2.6', 'RCP4.5', 'RCP6.0', 'RCP8.5'");

        double new_co2_value = fixedCO2concentrations[co2Pathway][yearindex];
        ClimateDay::co2 = new_co2_value;

    } else {
        ClimateDay::co2 = GlobalSettings::instance()->settings().valueDouble("model.climate.co2concentration", 380.);

    }

    if (logLevelDebug())
        qDebug() << "CO2 concentration" << ClimateDay::co2 << "ppm.";


}


void Climate::setupPhenology()
{
    mPhenology.clear();
    mPhenology.push_back(Phenology(this)); // id=0
    XmlHelper xml(GlobalSettings::instance()->settings().node("model.species.phenology"));
    int i=0;
    do {
        QDomElement n = xml.node(QString("type[%1]").arg(i));
        if (n.isNull())
            break;
        i++;
        int id;
        id = n.attribute("id", "-1").toInt();
        if (id<0) throw IException(QString("Error setting up phenology: id invalid\ndump: %1").arg(xml.dump("").join(" - ")));
        xml.setCurrentNode(n);
        Phenology item( id,
                        this,
                        xml.valueDouble(".vpdMin",0.5), // use relative access to node (".x")
                        xml.valueDouble(".vpdMax", 5),
                        xml.valueDouble(".dayLengthMin",10),
                        xml.valueDouble(".dayLengthMax",11),
                        xml.valueDouble(".tempMin", 2),
                        xml.valueDouble(".tempMax", 9) );

        mPhenology.push_back(item);
    } while(true);
}

/** return the phenology of the group... */
const Phenology &Climate::phenology(const int phenologyGroup) const
{
    const Phenology &p = mPhenology.at(phenologyGroup);
    if (p.id() == phenologyGroup)
        return p;
    // search...
    for (int i=0;i<mPhenology.count();i++)
        if (mPhenology[i].id()==phenologyGroup)
            return mPhenology[i];
    throw IException(QString("Error at SpeciesSet::phenology(): invalid group: %1").arg(phenologyGroup));
}

