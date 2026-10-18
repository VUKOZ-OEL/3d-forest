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

#include <QtCore>
#include <QtSql>
#include <QVector>
#include "global.h"
#include "globalsettings.h"
#include "xmlhelper.h"
#include "speciesset.h"
#include "species.h"
#include "model.h"
#include "seeddispersal.h"
#include "modelsettings.h"
#include "debugtimer.h"

/** @class SpeciesSet
    A SpeciesSet acts as a container for individual Species objects. In iLand, theoretically,
    multiple species sets can be used in parallel.
  */

SpeciesSet::SpeciesSet()
{
    mSetupQuery = 0;
}

SpeciesSet::~SpeciesSet()
{
   clear();
}

void SpeciesSet::clear()
{
    qDeleteAll(mSpecies.values());
    mSpecies.clear();
    mActiveSpecies.clear();
}

const Species *SpeciesSet::species(const int &index)
{
    foreach(Species *s, mSpecies)
        if (s->index() == index)
            return s;
    return nullptr;
}

bool SpeciesSet::hasVar(const QString &varName)
{
    Q_ASSERT(mSetupQuery!=nullptr);
    if (!mSetupQuery)
        throw IException("SpeciesSet: query is not active!");

    int idx = mSetupQuery->record().indexOf(varName);
    return idx >= 0;
}

/** loads active species from a database table and creates/setups the species.
    The function uses the global database-connection.
  */
int SpeciesSet::setup()
{
    const XmlHelper &xml = GlobalSettings::instance()->settings();
    QString tableName = xml.value("model.species.source", "species");
    mName = tableName;
    QString readerFile = xml.value("model.species.reader", "reader.bin");
    readerFile = GlobalSettings::instance()->path(readerFile, "lip");
    mReaderStamp.load(readerFile);
    if (GlobalSettings::instance()->settings().paramValueBool("debugDumpStamps", false) )
        qDebug() << mReaderStamp.dump();


    QSqlQuery query(GlobalSettings::instance()->dbin());
    mSetupQuery = &query;
    QString sql = QString("select * from %1").arg(tableName);
    query.exec(sql);
    if (query.lastError().isValid()){
        throw IException(QString("Error loading species set: %1 \n %2").arg(sql, query.lastError().text()) );
    }

    clear();
    qDebug() << "attempting to load a species set from" << tableName;
    while (query.next()) {
        if (var("active").toInt()==0)
            continue;

        Species *s = new Species(this); // create
        // call setup routine (which calls SpeciesSet::var() to retrieve values
        s->setup();

        mSpecies.insert(s->id(), s); // store
        if (s->active())
            mActiveSpecies.append(s);

        Expression::addConstant(s->id(), s->index());
    } // while query.next()
    qDebug() << "loaded" << mSpecies.count() << "active species:";
    qDebug() << "index, id, name";
    foreach(const Species *s, mActiveSpecies)
        qDebug() << s->index() << s->id() << s->name();

    mSetupQuery = 0;

    // setup nitrogen response
    XmlHelper resp(xml.node("model.species.nitrogenResponseClasses"));
    if (!resp.isValid())
        throw IException("model.species.nitrogenResponseClasses not present!");
    mNitrogen_1a = resp.valueDouble("class_1_a");
    mNitrogen_1b = resp.valueDouble("class_1_b");
    mNitrogen_2a = resp.valueDouble("class_2_a");
    mNitrogen_2b = resp.valueDouble("class_2_b");
    mNitrogen_3a = resp.valueDouble("class_3_a");
    mNitrogen_3b = resp.valueDouble("class_3_b");
    if (mNitrogen_1a*mNitrogen_1b*mNitrogen_2a*mNitrogen_2b*mNitrogen_3a*mNitrogen_3b == 0.)
        throw IException("at least one parameter of model.species.nitrogenResponseClasses is not valid (value=0)!");

    // setup CO2 response
    XmlHelper co2(xml.node("model.species.CO2Response"));
    mCO2base = co2.valueDouble("baseConcentration");
    mCO2comp = co2.valueDouble("compensationPoint");
    mCO2beta0 = co2.valueDouble("beta0");
    mCO2p0 = co2.valueDouble("p0");
    if (mCO2base*mCO2comp*(mCO2base-mCO2comp)*mCO2beta0*mCO2p0==0.)
        throw IException("at least one parameter of model.species.CO2Response is not valid!");

    // setup Light responses
    XmlHelper light(xml.node("model.species.lightResponse"));
    mLightResponseTolerant.setAndParse(light.value("shadeTolerant"));
    mLightResponseIntolerant.setAndParse(light.value("shadeIntolerant"));
    mLightResponseTolerant.linearize(0., 1.);
    mLightResponseIntolerant.linearize(0., 1.);
    if (mLightResponseTolerant.expression().isEmpty() || mLightResponseIntolerant.expression().isEmpty())
        throw IException("at least one parameter of model.species.lightResponse is empty!");
    // lri-correction
    mLRICorrection.setAndParse(light.value("LRImodifier","1"));
    // x: LRI, y: relative heigth
    mLRICorrection.linearize2d(0., 1., 0., 1.);

    createRandomSpeciesOrder();
    return mSpecies.count();

}

void SpeciesSet::setupRegeneration()
{
    SeedDispersal::setupExternalSeeds();
    foreach(Species *s, mActiveSpecies) {
        SeedDispersal *sd = new SeedDispersal(s);
        sd->setup(); // setup memory for the seed map (grid)
        s->setSeedDispersal(sd); // establish the link between species and the map
    }
    SeedDispersal::finalizeExternalSeeds();
    qDebug() << "Setup of seed dispersal maps finished.";
}

static void nc_seed_distribution(Species *species)
{
    species->seedDispersal()->execute();
}

void SpeciesSet::regeneration()
{
    if (!GlobalSettings::instance()->model()->settings().regenerationEnabled)
        return;
    DebugTimer t("seed dispersal (all species)");

    ThreadRunner runner(mActiveSpecies); // initialize a thread runner object with all active species
    runner.run(nc_seed_distribution);

    if (logLevelDebug())
        qDebug() << "seed dispersal finished.";
}

void SpeciesSet::clearSaplingSeedMap()
{
    foreach(Species *s, mActiveSpecies) {
        if (s->seedDispersal())
            s->seedDispersal()->clearSaplingMap();
    }
}

/** newYear is called by Model::runYear at the beginning of a year before any growth occurs.
  This is used for various initializations, e.g. to clear seed dispersal maps
  */
void SpeciesSet::newYear()
{
    if (!GlobalSettings::instance()->model()->settings().regenerationEnabled)
        return;
    foreach(Species *s, mActiveSpecies) {
        s->newYear();
    }
}

/** retrieves variables from the datasource available during the setup of species.
  */
QVariant SpeciesSet::var(const QString& varName)
{
    Q_ASSERT(mSetupQuery!=nullptr);
    if (!mSetupQuery)
        throw IException("SpeciesSet: query is not active!");

    int idx = mSetupQuery->record().indexOf(varName);
    if (idx>=0)
        return mSetupQuery->value(idx);
    throw IException(QString("SpeciesSet: The species parameter table does not contain the column: '%1'\n%2")
                     .arg(varName)
                     .arg("Please check https://iland-model.org/release+notes for details."));
    //throw IException(QString("load species parameter: field %1 not found!").arg(varName));
    // lookup in defaults
    //qDebug() << "variable" << varName << "not found - using default.";
    //return GlobalSettings::instance()->settingDefaultValue(varName);
}

void SpeciesSet::randomSpeciesOrder(QVector<int>::const_iterator &rBegin, QVector<int>::const_iterator &rEnd)
{
    int iset = irandom(0,mNRandomSets);
    rBegin=mRandomSpeciesOrder.begin() + iset * mActiveSpecies.size();
    rEnd=rBegin+mActiveSpecies.size();
}

//
void SpeciesSet::createRandomSpeciesOrder()
{

    mRandomSpeciesOrder.clear();
    mRandomSpeciesOrder.reserve(mActiveSpecies.size() * mNRandomSets);
    for (int i=0;i<mNRandomSets;++i) {
        QList<int> samples;
        // fill list
        foreach (const Species* s, mActiveSpecies)
            samples.push_back(s->index());
        // sample and reduce list
        while (!samples.isEmpty()) {
            mRandomSpeciesOrder.push_back( samples.takeAt(irandom(0, samples.size()))  );
        }
    }
}

inline double SpeciesSet::nitrogenResponse(const double &availableNitrogen, const double &NA, const double &NB) const
{
    if (availableNitrogen<=NB)
        return 0;
    double x = 1. - exp(NA * (availableNitrogen-NB));
    return x;
}



/// calculate nitrogen response for a given amount of available nitrogen and a respone class
/// for fractional values, the response value is interpolated between the fixedly defined classes (1,2,3)
double SpeciesSet::nitrogenResponse(const double availableNitrogen, const double &responseClass) const
{
    double value1, value2, value3;
    if (responseClass>2.) {
        if (responseClass==3.)
            return nitrogenResponse(availableNitrogen, mNitrogen_3a, mNitrogen_3b);
        else {
            // interpolate between 2 and 3
            value2 = nitrogenResponse(availableNitrogen, mNitrogen_2a, mNitrogen_2b);
            value3 = nitrogenResponse(availableNitrogen, mNitrogen_3a, mNitrogen_3b);
            return value2 + (responseClass-2)*(value3-value2);
        }
    }
    if (responseClass==2.)
        return nitrogenResponse(availableNitrogen, mNitrogen_2a, mNitrogen_2b);
    if (responseClass==1.)
        return nitrogenResponse(availableNitrogen, mNitrogen_1a, mNitrogen_1b);
    // last ressort: interpolate between 1 and 2
    value1 = nitrogenResponse(availableNitrogen, mNitrogen_1a, mNitrogen_1b);
    value2 = nitrogenResponse(availableNitrogen, mNitrogen_2a, mNitrogen_2b);
    return value1 + (responseClass-1)*(value2-value1);
}

/** calculation for the CO2 response for the ambientCO2 for the water- and nitrogen responses given.
    The calculation follows Friedlingsstein 1995 (see also links to equations in code)
    see also: https://iland-model.org/CO2+response
    @param ambientCO2 current CO2 concentration (ppm)
    @param nitrogenResponse (yearly) nitrogen response of the species
    @param soilWaterResponse soil water response (mean value for a month)
*/
double SpeciesSet::co2Response(const double ambientCO2, const double nitrogenResponse, const double soilWaterResponse) const
{
    if (nitrogenResponse==0.)
        return 0.;

    double co2_water = 2. - soilWaterResponse;
    double beta = mCO2beta0 * co2_water * nitrogenResponse;

    double r =1. +  M_LN2 * beta; // NPP increase for a doubling of atmospheric CO2 (Eq. 17)

    // fertilization function (cf. Farquhar, 1980) based on Michaelis-Menten expressions
    double deltaC = mCO2base - mCO2comp;
    double K2 = ((2*mCO2base - mCO2comp) - r*deltaC ) / ((r-1.)*deltaC*(2*mCO2base - mCO2comp)); // Eq. 16
    double K1 = (1. + K2*deltaC) / deltaC;

    double response = mCO2p0 * K1*(ambientCO2 - mCO2comp) / (1 + K2*(ambientCO2-mCO2comp)); // Eq. 16
    return response;

}

/** calculates the lightResponse based on a value for LRI and the species lightResponseClass.
    LightResponse is classified from 1 (very shade inolerant) and 5 (very shade tolerant) and interpolated for values between 1 and 5.
    Returns a value between 0..1
    @sa https://iland-model.org/allocation#reserve_and_allocation_to_stem_growth */
double SpeciesSet::lightResponse(const double lightResourceIndex, const double lightResponseClass) const
{
    double low = mLightResponseIntolerant.calculate(lightResourceIndex);
    double high = mLightResponseTolerant.calculate(lightResourceIndex);
    double result = low + 0.25*(lightResponseClass-1.)*(high-low);
    return limit(result, 0., 1.);

}



