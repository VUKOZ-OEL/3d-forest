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

/** @class SpeciesResponse
  @ingroup core
    Environmental responses relevant for production of a tree species on resource unit level.
    SpeciesResponse combines data from different sources and converts information about the environment
    into responses of a species. The spatial level is the "ResourceUnit", where homogenetiy of environmental factors
    is assumed. The temporal aggregation depends on the factor, but usually, the daily environmental data is
    aggregated to monthly response values (which subsequently are used during 3PG production).
 Sources are:
    - vapour pressure deficit (dryness of atmosphere): directly from climate data (daily)
    - soil water status (dryness of soil)(daily)
    - temperature: directly from climate data (daily)
    - phenology: @sa Phenology, combines several sources (quasi-monthly)
    - CO2: @sa SpeciesSet::co2Response() based on ambient CO2 level (climate data), nitrogen and soil water responses (yearly)
    - nitrogen: based on the amount of available nitrogen (yearly)
*/
#include "speciesresponse.h"

#include "resourceunit.h"
#include "species.h"
#include "resourceunitspecies.h"
#include "climate.h"
#include "model.h"
#include "watercycle.h"
#include "debugtimer.h"

SpeciesResponse::SpeciesResponse()
{
    mSpecies=0;
    mRu=0;
}
void SpeciesResponse::clear()
{
    for (int i=0;i<12;i++)
        mCO2Response[i]=mSoilWaterResponse[i]=mTempResponse[i]=mRadiation[i]=mUtilizableRadiation[i]=mVpdResponse[i]=0.;

    mNitrogenResponse = 0.;
    mTotalRadiation = 0.;
    mTotalUtilizeableRadiation = 0.;

}
void SpeciesResponse::setup(ResourceUnitSpecies *rus)
{
    mSpecies = rus->species();
    mRu = rus->ru();
    clear();
}

/// calculate responses for VPD and Soil Water. Return the minimum of those responses
/// @param psi_kPa psi of the soil in kPa
/// @param vpd vapor pressure deficit in kPa
/// @return minimum of soil water and vpd response
void SpeciesResponse::soilAtmosphereResponses(const double psi_kPa, const double vpd, double &rMinResponse) const
{
    double water_resp = mSpecies->soilwaterResponse(psi_kPa);
    double vpd_resp = mSpecies->vpdResponse( vpd );
    rMinResponse = qMin(water_resp, vpd_resp);
}


/// Main function that calculates monthly / annual species responses
void SpeciesResponse::calculate()
{
    DebugTimer tpg("SpeciesResponse::calculate");

    clear(); // reset values

    // calculate yearly responses
    const WaterCycle *water = mRu->waterCycle();
    const Phenology &pheno = mRu->climate()->phenology(mSpecies->phenologyClass());
    int veg_begin = pheno.vegetationPeriodStart();
    int veg_end = pheno.vegetationPeriodEnd();

    // yearly response
    const double nitrogen = mRu->resouceUnitVariables().nitrogenAvailable + mRu->resouceUnitVariables().nitrogenAvailableDelta;
    // Nitrogen response: a yearly value based on available nitrogen
    mNitrogenResponse = mSpecies->nitrogenResponse( nitrogen );
    const double ambient_co2 = mRu->climate()->begin()->co2; // CO2 level of first day of year (co2 is static)

    double water_resp, vpd_resp, temp_resp, min_resp;
    double  utilizeable_radiation;
    int doy=0;
    int month;
    const ClimateDay *end = mRu->climate()->end();
    for (const ClimateDay *day=mRu->climate()->begin(); day!=end; ++day) {
        month = day->month - 1;
        // environmental responses
        water_resp = mSpecies->soilwaterResponse(water->psi_kPa(doy));
        vpd_resp = mSpecies->vpdResponse( day->vpd );
        temp_resp = mSpecies->temperatureResponse(day->temp_delayed);
        mSoilWaterResponse[month] += water_resp;
        mTempResponse[month] += temp_resp;
        mVpdResponse[month] += vpd_resp;
        mRadiation[month] += day->radiation;

        if (doy>=veg_begin && doy<=veg_end) {
            // environmental responses for the day
            // combine responses
            min_resp = qMin(qMin(vpd_resp, temp_resp), water_resp);
            // calculate utilizable radiation, Eq. 4, https://iland-model.org/primary+production
            utilizeable_radiation = day->radiation * min_resp;

        } else {
            utilizeable_radiation = 0.; // no utilizable radiation outside of vegetation period
            min_resp = 0.;
        }
        mUtilizableRadiation[month]+= utilizeable_radiation;
        doy++;
        //DBGMODE(
            if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dDailyResponses) && mRu->shouldCreateDebugOutput()) {
                DebugList &out = GlobalSettings::instance()->debugList(day->id(), GlobalSettings::dDailyResponses);
                // climatic variables
                out << mSpecies->id() << day->id() << mRu->index() << mRu->id(); // date << day->temperature << day->vpd << day->preciptitation << day->radiation;
                out << water_resp << temp_resp << vpd_resp << day->radiation << utilizeable_radiation;
            }
        //); // DBGMODE()

    }
    mTotalRadiation = mRu->climate()->totalRadiation();
    // monthly values
    for (int i=0;i<12;i++) {
        double days = mRu->climate()->days(i);
        mTotalUtilizeableRadiation += mUtilizableRadiation[i];
        mSoilWaterResponse[i]/=days;
        mTempResponse[i]/=days;
        mVpdResponse[i]/=days;
        mCO2Response[i] = mSpecies->speciesSet()->co2Response(ambient_co2,
                                                           mNitrogenResponse,
                                                           mSoilWaterResponse[i]);
    }

}


