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

#include "establishment.h"

#include "climate.h"
#include "species.h"
#include "resourceunit.h"
#include "resourceunitspecies.h"
#include "model.h"
#include "watercycle.h"
#include "permafrost.h"
#include "microclimate.h"

/** @class Establishment
    Establishment deals with the establishment process of saplings.
    https://iland-model.org/establishment
    Prerequisites for establishment are:
    the availability of seeds: derived from the seed-maps per Species (@sa SeedDispersal)
    the quality of the abiotic environment (TACA-model): calculations are performend here, based on climate and species responses
    the quality of the biotic environment, mainly light: based on the LIF-values

  */
Establishment::Establishment()
{
    mPAbiotic = 0.;
}

Establishment::Establishment(const Climate *climate, const ResourceUnitSpecies *rus)
{
    setup(climate, rus);
}

void Establishment::setup(const Climate *climate, const ResourceUnitSpecies *rus)
{
    mClimate = climate;
    mRUS = rus;
    mPAbiotic = 0.;
    mNumberEstablished = 0;
    if (climate==0)
        throw IException(QString("Establishment::setup: no valid climate for a resource unit: RU-Index: %1, RU-ID: %2, coords: (%3/%4) ").arg(rus->ru()->index()).arg(rus->ru()->id()).
                         arg(rus->ru()->boundingBox().center().x()).arg(rus->ru()->boundingBox().center().y()) );
    if (rus==0 || rus->species()==0 || rus->ru()==0)
        throw IException("Establishment::setup: important variable is null (are the species properly set up?).");
}



void Establishment::clear()
{
    mPAbiotic = 0.;
    mNumberEstablished = 0;
    mTACA_min_temp=mTACA_chill=mTACA_gdd=mTACA_frostfree=false;
    mTACA_frostAfterBuds=0;
    mSumLIFvalue = 0.;
    mLIFcount = 0;
    mWaterLimitation = 0.;

}


double Establishment::calculateWaterLimitation()
{
    // return 1 if effect is disabled
    if (mRUS->species()->establishmentParameters().psi_min >= 0.)
        return 1.;

    double psi_min = mRUS->species()->establishmentParameters().psi_min;
    // get the psi min of the current year
    double psi_mpa = mRUS->ru()->waterCycle()->estPsiMin( mRUS->species()->phenologyClass() );

    // calculate the response of the species to this value of psi (see also Species::soilwaterResponse())
    double result = limit( (psi_mpa - psi_min) / (-0.015 -  psi_min) , 0., 1.);

    return result;


/*
    double psi_min = mRUS->species()->establishmentParameters().psi_min;
    const WaterCycle *water = mRUS->ru()->waterCycle();
    int days = mRUS->ru()->climate()->daysOfYear();

    // two week (14 days) running average of actual psi-values on the resource unit
    static const int nwindow = 14;
    double psi_buffer[nwindow];
    for (int i=0;i<nwindow;++i)
        psi_buffer[i] = 0.;
    double current_sum = 0.;

    int i_buffer = 0;
    double min_average = 9999999.;
    double current_avg = 0.;
    for (int day=0;day<days;++day) {
        // running average: remove oldest item, add new item in a ringbuffer
        current_sum -= psi_buffer[i_buffer];
        psi_buffer[i_buffer] = water->psi_kPa(day);
        current_sum += psi_buffer[i_buffer];

        if (day>=veg_period_start && day<=veg_period_end) {
            current_avg = day>0? current_sum / std::min(day, nwindow) : current_sum;
            min_average = std::min(min_average, current_avg);
        }

        // move to next value in the buffer
        i_buffer = (i_buffer + 1) % nwindow;
    }

    if (min_average > 1000.)
        return 1.; // invalid vegetation period?

    // calculate the response of the species to this value of psi (see also Species::soilwaterResponse())
    const double psi_mpa = min_average / 1000.; // convert to MPa
    double result = limit( (psi_mpa - psi_min) / (-0.015 -  psi_min) , 0., 1.);

    return result;
*/
}

double Establishment::calculateSOLDepthLimitation()
{
    if (mRUS->species()->establishmentParameters().SOL_thickness == 0.)
        return 1.; // no effect for the current species

    if (!mRUS->ru()->waterCycle()->permafrost())
        return 1.; // no limitation if permafrost module is disabled

    double depth = mRUS->ru()->waterCycle()->permafrost()->mossLayerThickness() +
            mRUS->ru()->waterCycle()->permafrost()->SOLLayerThickness();

    depth = depth * 100.; // to cm

    double est_SOLlimit = mRUS->species()->establishmentParameters().SOL_thickness;
    double effect = exp( -est_SOLlimit * depth );
    return effect;
}



/** Calculate the abiotic environemnt for seedling for a given species and a given resource unit.
 The model is closely based on the TACA approach of Nitschke and Innes (2008), Ecol. Model 210, 263-277
 more details: https://iland-model.org/establishment#abiotic_environment
 a model mockup in R: script_establishment.r

 */
void Establishment::calculateAbioticEnvironment()
{
    //DebugTimer t("est_abiotic"); t.setSilent();
    // make sure that required calculations (e.g. watercycle are already performed)
    const_cast<ResourceUnitSpecies*>(mRUS)->calculate(true); // calculate the 3pg module and run the water cycle (this is done only if that did not happen up to now); true: call comes from regeneration

    const EstablishmentParameters &p = mRUS->species()->establishmentParameters();
    const Phenology &pheno = mClimate->phenology(mRUS->species()->phenologyClass());

    mTACA_min_temp = true; // minimum temperature threshold
    mTACA_chill = false;  // (total) chilling requirement
    mTACA_gdd = false;   // gdd-thresholds
    mTACA_frostfree = false; // frost free days in vegetation period
    mTACA_frostAfterBuds = 0; // frost days after bud birst
    mGDD = 0;

    // should we use microclimate temperatures?
    bool use_micro_clim = Model::settings().microclimateEnabled && mRUS->ru()->microClimate()->settings().establishment_effect;


    const ClimateDay *day = mClimate->begin();
    int doy = 0;
    double GDD=0.;
    double GDD_BudBirst = 0.;
    int chill_days = pheno.chillingDaysLastYear(); // chilling days of the last autumn
    int frost_free = 0;
    mTACA_frostAfterBuds = 0;
    bool chill_ok = false;
    bool buds_are_birst = false;
    int veg_period_end = pheno.vegetationPeriodEnd();
    if (veg_period_end >= 365)
        veg_period_end = mClimate->sun().dayShorter10_5hrs();

    for (; day!=mClimate->end(); ++day, ++doy) {

        double day_tmin = day->min_temperature;
        double day_tavg = day->temperature;

        if (use_micro_clim) {
            // use microclimate calculations to modify the temperature
            // for establishment
            double mc_min_buf = mRUS->ru()->microClimate()->minimumMicroclimateBufferingRU(day->month-1);
            double mc_max_buf = mRUS->ru()->microClimate()->maximumMicroclimateBufferingRU(day->month-1);
            double mc_mean_buf = (mc_min_buf + mc_max_buf) / 2.;

            day_tmin += mc_min_buf;
            day_tavg += mc_mean_buf;

        }

        // minimum temperature: if temp too low -> set prob. to zero
        if (day_tmin < p.min_temp)
            mTACA_min_temp = false;

        // count frost free days
        if (day_tmin > 0.)
            frost_free++;

        // chilling requirement, GDD, bud birst
        if (day_tavg >=-5. && day_tavg <5. && doy<=veg_period_end)
            chill_days++;
        if (chill_days>p.chill_requirement)
            chill_ok=true;
        // GDDs above the base temperature are counted if beginning from the day where the chilling requirements are met
        // up to a fixed day ending the veg period
        if (doy<=veg_period_end) {
            // accumulate growing degree days
            if (chill_ok && day_tavg > p.GDD_baseTemperature) {
                GDD += day_tavg - p.GDD_baseTemperature;
                GDD_BudBirst += day_tavg - p.GDD_baseTemperature;
            }
            // if day-frost occurs, the GDD counter for bud birst is reset
            if (day_tavg <= 0.)
                GDD_BudBirst = 0.;

            if (GDD_BudBirst > p.bud_birst)
                buds_are_birst = true;

            if (doy<veg_period_end && buds_are_birst && day_tmin <= 0.)
                mTACA_frostAfterBuds++;
        }
    }
    // chilling requirement
    if (chill_ok)
        mTACA_chill = true;

    // GDD requirements
    mGDD = static_cast<int>(GDD);
    if (GDD>p.GDD_min && GDD<p.GDD_max)
        mTACA_gdd = true;

    // frost free days in the vegetation period
    if (frost_free > p.frost_free)
        mTACA_frostfree = true;

    // if all requirements are met:
    if (mTACA_chill && mTACA_min_temp && mTACA_gdd && mTACA_frostfree) {
        // negative effect of frost events after bud birst
        double frost_effect = 1.;
        if (mTACA_frostAfterBuds>0)
            frost_effect = pow(p.frost_tolerance, sqrt(double(mTACA_frostAfterBuds)));
        // negative effect due to water limitation on establishment [1: no effect]
        mWaterLimitation = calculateWaterLimitation();
        // negative effect of a thick soil organic layer on regeneration [1: no effect]
        double SOL_limitation = calculateSOLDepthLimitation();

        // combine effects of drought, frost, and soil organic layer depth multiplicatively
        mPAbiotic = frost_effect * mWaterLimitation * SOL_limitation;
    } else {
        mPAbiotic = 0.; // if any of the requirements is not met
    }

}

void Establishment::writeDebugOutputs()
{
    if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dEstablishment) && mRUS->ru()->shouldCreateDebugOutput()) {
        DebugList &out = GlobalSettings::instance()->debugList(mRUS->ru()->index(), GlobalSettings::dEstablishment);
        // establishment details
        out << mRUS->species()->id() << mRUS->ru()->index() << mRUS->ru()->id();
        out << TACAminTemp() << TACAchill() << TACAfrostFree() << TACgdd();
        out << TACAfrostDaysAfterBudBirst() << waterLimitation() << growingDegreeDays()  << abioticEnvironment();
        out << mRUS->prod3PG().fEnvYear() << mRUS->constSaplingStat().newSaplings();
    }
}

