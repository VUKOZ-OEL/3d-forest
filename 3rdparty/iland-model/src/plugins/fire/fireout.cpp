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

#include "fireout.h"
#include "model.h"
#include "resourceunit.h"
#include "snag.h"
#include "soil.h"

FireOut::FireOut()
{
    mFire = 0;
    setName("Fire RU/yr", "fire");
    setDescription("Fire event aggregates per fire event. "\
                   "The output contains a row for each (ignited) fire event. " \
                   " ");
    columns() << OutputColumn::year()
              << OutputColumn("fireId", "unique ID of the fire event (1..N) on the whole project area.", OutInteger)
              << OutputColumn("area_plan_m2", "Area of the planned fire m2", OutInteger)
              << OutputColumn("area_m2", "Realized area of burnt cells m2", OutInteger)
              << OutputColumn("iterations", "Number of iterations of the cellular automaton", OutInteger)
              << OutputColumn("coord_x", "Coordinates (x) of the starting point (m)", OutDouble)
              << OutputColumn("coord_y", "Coordinates (y) of the starting point (m)", OutDouble)
              << OutputColumn("n_trees", "total number of trees on all burning cells", OutInteger)
              << OutputColumn("n_trees_died", "total number of trees that were killed by the fire", OutDouble)
              << OutputColumn("basalArea_total", "sum of basal area on burning pixels of the fire (m2)", OutDouble)
              << OutputColumn("basalArea_died", "sum of basal area of died trees (m2)", OutDouble)
              << OutputColumn("psme_died", "fraction of doug fir that died (based on basal area of psme trees on burning pixels)", OutDouble)
              << OutputColumn("avgFuel_kg_ha", "average total fuel (dry) (forest floor + dwd + moss) of burning cells (kg biomass/ha)", OutDouble)
              << OutputColumn("windSpeed", "current wind speed during the event (m/s)", OutDouble)
              << OutputColumn("windDirection", "current wind direction during the event (degree)", OutDouble) ;



}

void FireOut::setup()
{
}

// Output function
// fire data is aggregated in this function for each fire event.
void FireOut::exec()
{
    *this << currentYear();
    *this << mFire->mFireId;
    *this << mFire->fireStats.fire_size_plan_m2 << mFire->fireStats.fire_size_realized_m2;
    *this << mFire->fireStats.iterations;
    *this << mFire->fireStats.startpoint.x() << mFire->fireStats.startpoint.y();
    int fire_id = mFire->mFireId;
    double avg_fuel = 0.;
    int n_ru = 0;
    double n_trees = 0.;
    double n_trees_died = 0.;
    double basal_area = 0.;
    double basal_area_died = 0.;
    for (FireRUData *fds = mFire->mRUGrid.begin(); fds!=mFire->mRUGrid.end(); ++fds) {
        if (fds->fireRUStats.fire_id == fire_id) {
            // the current fire burnt on this area
            n_ru++;
            avg_fuel += fds->fireRUStats.fuel_dwd+fds->fireRUStats.fuel_ff+fds->fireRUStats.fuel_moss;
            n_trees += fds->fireRUStats.n_trees;
            n_trees_died += fds->fireRUStats.n_trees_died;
            basal_area += fds->fireRUStats.basal_area;
            basal_area_died += fds->fireRUStats.died_basal_area;
        }
    }
    if (n_ru>0) {
        avg_fuel /= double(n_ru);
    }
    *this << n_trees << n_trees_died << basal_area << basal_area_died;
    *this << (mFire->fireStats.fire_psme_total>0.?mFire->fireStats.fire_psme_died / mFire->fireStats.fire_psme_total:0.);
    *this << avg_fuel;
    *this << mFire->mCurrentWindSpeed << mFire->mCurrentWindDirection;

    writeRow();

}

