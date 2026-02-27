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
#include "bite_global.h"

#include "biteclimate.h"
#include "climate.h"
#include "resourceunit.h"
#include "watercycle.h"

namespace BITE {

QStringList BiteClimate::mClimateVars = QStringList() << "MAT" << "MAP" << "GDD" // 0,1,2
                                                      << "TMonth1" << "TMonth2" << "TMonth3" // 3-14: monthly temp
                                                      << "TMonth4" << "TMonth5" << "TMonth6"
                                                      << "TMonth7" << "TMonth8" << "TMonth9"
                                                      << "TMonth10" << "TMonth11" << "TMonth12"
                                                      << "PMonth1" << "PMonth2" << "PMonth3" // 15-26: monthly prec
                                                      << "PMonth4" << "PMonth5" << "PMonth6"
                                                      << "PMonth7" << "PMonth8" << "PMonth9"
                                                      << "PMonth10" << "PMonth11" << "PMonth12"
                                                      << "GDD10" // 27
                                                      << "relWaterContentGS" << "relWaterContent"; // 28, 29

BiteClimate::BiteClimate()
{

}

void BiteClimate::setup(QJSValue clim_vars, BiteWrapperCore &wrapper)
{
    qCDebug(biteSetup) << "Setup of climate variables";
    QJSValueIterator it(clim_vars);
    while (it.hasNext()) {
        it.next();

        if (it.name() != "length") {
            QString var_name = it.value().toString();
            int var_index = mClimateVars.indexOf(var_name);
            if (var_index==-1)
                throw IException(QString("The climate variable '%1' is not valid!").arg(var_name));
            qCDebug(biteSetup) << "registering:" << var_name << "with index:" << var_index;

            wrapper.registerClimateVar(var_index, var_name);
        }
    }
}

double BiteClimate::value(int var_index, const ResourceUnit *ru) const
{
    Q_ASSERT(ru != nullptr);
    const Climate *c = ru->climate();
    switch (var_index) {
    case 0: return c->meanAnnualTemperature();  // mean annual temp
    case 1: return c->annualPrecipitation(); // MAP
    case 2: return calculateGDD(c, 5.); // GDD(with base temp 5 degrees)
    case 27: return calculateGDD(c, 10.); // GDD(with base temp 10 degrees)
    case 28: return calculateMeanWaterContent(ru, true); // mean rel. water content in growing season (apr - sept)
    case 29: return calculateMeanWaterContent(ru, false); // mean rel. water content over the entire year
    }
    if (var_index<3+12) {
        return c->temperatureMonth()[var_index - 3];
    }
    if (var_index<3+12+12) {
        return c->precipitationMonth()[var_index - 3 - 12];
    }

    return 0.;
}

QVector<double> BiteClimate::dailyClimateTimeseries(const ResourceUnit *ru, QString type) const
{
    Q_ASSERT(ru != nullptr);
    int vtyp = -1;
    if (type == "tmin") vtyp = 0;
    if (type == "tmax") vtyp = 1;
    if (type == "tmean") vtyp = 2;
    if (type == "prec") vtyp = 3;
    if (type == "rad") vtyp = 4;
    if (type == "vpd") vtyp = 5;
    if (vtyp < 0)
        throw IException("Error - invalid 'type' for dailyClimateTimeseries(). Needs to be one of: tmin, tmax, tmean, prec, rad, vpd.");
    const Climate *c = ru->climate();
    QVector<double> values;
    values.reserve(366);
    double value;
    for (const ClimateDay *d=c->begin();d!=c->end();++d) {
        switch (vtyp) {
            case 0: value = d->min_temperature; break;
            case 1: value = d->max_temperature; break;
            case 2: value = d->mean_temp(); break;
            case 3: value = d->preciptitation; break;
            case 4: value = d->radiation; break;
            case 5: value = d->vpd; break;
        }
        values.append(value);
    }
    return values;
}

double BiteClimate::calculateGDD(const Climate *clim, double threshold_temp) const
{
    double gdd=0.;
    for (const ClimateDay *d = clim->begin(); d!=clim->end(); ++d)
        gdd += qMax(d->mean_temp()-threshold_temp, 0.);
    return gdd;
}

double BiteClimate::calculateMeanWaterContent(const ResourceUnit *ru, bool only_growing_season) const
{
    if (!ru->waterCycle()) return 0.;
    double ru_whc = ru->waterCycle()->waterHoldingCapacity();
    if (ru_whc == 0.) return 0;

    if (only_growing_season)
        return limit( ru->waterCycle()->meanGrowingSeasonSWC() / ru_whc, 0., 1.);
    else
        return limit( ru->waterCycle()->meanSoilWaterContent() / ru_whc, 0., 1.);
}


} // namespace
