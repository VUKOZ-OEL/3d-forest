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

#include "carbonout.h"
#include "model.h"
#include "resourceunit.h"
#include "snag.h"
#include "soil.h"
#include "watercycle.h"
#include "permafrost.h"

CarbonOut::CarbonOut()
{
    setName("Carbon and nitrogen pools above and belowground per RU/yr", "carbon");
    setDescription("Carbon and nitrogen pools (C and N) per resource unit / year and/or by landsacpe/year. "\
                   "On resource unit level, the outputs contain aggregated above ground pools (kg/ha) " \
                   "and below ground pools (kg/ha). \n " \
                   "For landscape level outputs, all variables are scaled to kg/ha stockable area. "\
                   "The area column contains the stockable area (per resource unit / landscape) and can be used to scale to values to the actual value on the ground. \n " \
                   "You can use the 'condition' to control if the output should be created for the current year(see also dynamic stand output).\n" \
                   "The 'conditionRU' can be used to suppress resource-unit-level details; eg. specifying 'in(year,100,200,300)' limits output on reosurce unit level to the years 100,200,300 " \
                   "(leaving 'conditionRU' blank enables details per default).");
    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id()
              << OutputColumn("area_ha", "total stockable area of the resource unit (ha)", OutDouble)
              << OutputColumn("stem_c", "Stem carbon kg/ha", OutDouble)
              << OutputColumn("stem_n", "Stem nitrogen kg/ha", OutDouble)
              << OutputColumn("branch_c", "branches carbon kg/ha", OutDouble)
              << OutputColumn("branch_n", "branches nitrogen kg/ha", OutDouble)
              << OutputColumn("foliage_c", "Foliage carbon kg/ha", OutDouble)
              << OutputColumn("foliage_n", "Foliage nitrogen kg/ha", OutDouble)
              << OutputColumn("coarseRoot_c", "coarse root carbon kg/ha", OutDouble)
              << OutputColumn("coarseRoot_n", "coarse root nitrogen kg/ha", OutDouble)
              << OutputColumn("fineRoot_c", "fine root carbon kg/ha", OutDouble)
              << OutputColumn("fineRoot_n", "fine root nitrogen kg/ha", OutDouble)
              << OutputColumn("regeneration_c", "total carbon in regeneration layer (h<4m) kg/ha", OutDouble)
              << OutputColumn("regeneration_n", "total nitrogen in regeneration layer (h<4m) kg/ha", OutDouble)
              << OutputColumn("snags_c", "standing dead wood carbon kg/ha", OutDouble)
              << OutputColumn("snags_n", "standing dead wood nitrogen kg/ha", OutDouble)
              << OutputColumn("snagsOther_c", "branches and coarse roots of standing dead trees, carbon kg/ha", OutDouble)
              << OutputColumn("snagsOther_n", "branches and coarse roots of standing dead trees, nitrogen kg/ha", OutDouble)
              << OutputColumn("snagsOther_c_ag", "branches of standing dead trees (also included in snagsOther_c), carbon kg/ha", OutDouble)
              << OutputColumn("downedWood_c", "downed woody debris (yR, branches, stems, coarse roots), carbon kg/ha", OutDouble)
              << OutputColumn("downedWood_n", "downed woody debris (yR), nitrogen kg/ga", OutDouble)
              << OutputColumn("downedWood_c_ag", "downed woody debris aboveground (yR, stems, branches, also included in downedWood_c), nitrogen kg/ga", OutDouble)
              << OutputColumn("litter_c", "soil litter (yl, foliage and fine roots) carbon kg/ha", OutDouble)
              << OutputColumn("litter_n", "soil litter (yl), nitrogen kg/ha", OutDouble)
              << OutputColumn("litter_c_ag", "soil litter aboveground (yl, foliage, part of litter_c) carbon kg/ha", OutDouble)
              << OutputColumn("soil_c", "soil organic matter (som), carbon kg/ha", OutDouble)
              << OutputColumn("soil_n", "soil organic matter (som), nitrogen kg/ha", OutDouble)
              << OutputColumn("understorey_c", "living understorey vegetation (e.g. moss) kg C/ha", OutDouble);



}

void CarbonOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);

    condition = settings().value(".conditionRU", "");
    mConditionDetails.setExpression(condition);

}


void CarbonOut::exec()
{
    Model *m = GlobalSettings::instance()->model();

    // global condition
    if (!mCondition.isEmpty() && mCondition.calculate(GlobalSettings::instance()->currentYear())==0.)
        return;

    bool ru_level = true;
    // switch off details if this is indicated in the conditionRU option
    if (!mConditionDetails.isEmpty() && mConditionDetails.calculate(GlobalSettings::instance()->currentYear())==0.)
        ru_level = false;


    QVector<double> v(26, 0.); // 8 data values
    QVector<double>::iterator vit;

    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1 || !ru->snag())
            continue; // do not include if out of project area

        const StandStatistics &s = ru->statistics();
        int ru_count = 0;
        double area_factor = ru->stockableArea() / cRUArea; // conversion factor from real area to per ha values
        if (ru_level) {
            *this << currentYear() << ru->index() << ru->id() << area_factor; // keys
            // biomass from trees (scaled to 1ha already)

            *this << s.cStem() << s.nStem()   // stem
                               << s.cBranch() << s.nBranch()   // branch
                               << s.cFoliage() << s.nFoliage()   // foliage
                               << s.cCoarseRoot() << s.nCoarseRoot()   // coarse roots
                               << s.cFineRoot() << s.nFineRoot();   // fine roots

            // biomass from regeneration
            *this << s.cRegeneration() << s.nRegeneration();

            // biomass from standing dead wood: this is not scaled to ha-values!
            *this << ru->snag()->totalSWD().C / area_factor
                    << ru->snag()->totalSWD().N / area_factor   // snags
                    << ru->snag()->totalOtherWood().C/area_factor
                    << ru->snag()->totalOtherWood().N / area_factor
                    << ru->snag()->totalOtherWood().C/ area_factor * ru->snag()->otherWoodAbovegroundFraction();   // snags, other (branch + coarse root)

            // biomass from soil (convert from t/ha -> kg/ha)
            *this << ru->soil()->youngRefractory().C*1000.
                    << ru->soil()->youngRefractory().N*1000.   // wood
                    << ru->soil()->youngRefractory().C*1000. * ru->soil()->youngRefractoryAbovegroundFraction() // aboveground fraction
                    << ru->soil()->youngLabile().C*1000.
                    << ru->soil()->youngLabile().N*1000.   // litter
                    << ru->soil()->youngLabile().C*1000. * ru->soil()->youngLabileAbovegroundFraction() // aboveground fraction
                    << ru->soil()->oldOrganicMatter().C*1000.
                    << ru->soil()->oldOrganicMatter().N*1000.;   // soil

            // biomass for understorey (currently only moss)
            double understorey_c = 0.;
            if (ru->waterCycle()->permafrost())
                understorey_c = ru->waterCycle()->permafrost()->mossBiomass() * biomassCFraction * 10000.; // convert from kg/m2 -> kg C / ha

            *this << understorey_c;

            writeRow();
        }
        // landscape level statistics

        ++ru_count;
        vit = v.begin();
        *vit++ += area_factor;
        // carbon pools aboveground are in kg/resource unit, e.g., the sum of stem-carbon of all trees, so no scaling required
        *vit++ += s.cStem() * area_factor; *vit++ += s.nStem()* area_factor;
        *vit++ += s.cBranch() * area_factor; *vit++ += s.nBranch()* area_factor;
        *vit++ += s.cFoliage() * area_factor; *vit++ += s.nFoliage()* area_factor;
        *vit++ += s.cCoarseRoot() * area_factor; *vit++ += s.nCoarseRoot()* area_factor;
        *vit++ += s.cFineRoot() * area_factor; *vit++ += s.nFineRoot()* area_factor;
        // regen
        *vit++ += s.cRegeneration(); *vit++ += s.nRegeneration();
        // standing dead wood
        *vit++ += ru->snag()->totalSWD().C ; *vit++ += ru->snag()->totalSWD().N ;
        *vit++ += ru->snag()->totalOtherWood().C ; *vit++ += ru->snag()->totalOtherWood().N ; *vit++ += ru->snag()->totalOtherWood().C * ru->snag()->otherWoodAbovegroundFraction();
        // biomass from soil (converstion to kg/ha), and scale with fraction of stockable area
        *vit++ += ru->soil()->youngRefractory().C*area_factor * 1000.; *vit++ += ru->soil()->youngRefractory().N*area_factor * 1000.; *vit++ += ru->soil()->youngRefractory().C*area_factor * 1000. * ru->soil()->youngRefractoryAbovegroundFraction();
        *vit++ += ru->soil()->youngLabile().C*area_factor * 1000.; *vit++ += ru->soil()->youngLabile().N*area_factor * 1000.; *vit++ += ru->soil()->youngLabile().C*area_factor * 1000. * ru->soil()->youngLabileAbovegroundFraction();
        *vit++ += ru->soil()->oldOrganicMatter().C*area_factor * 1000.; *vit++ += ru->soil()->oldOrganicMatter().N*area_factor * 1000.;

    }
    // write landscape sums
    double total_stockable_area = v[0]; // convert to ha of stockable area
    *this << currentYear() << -1 << -1; // keys
    *this << v[0]; // stockable area [m2]
    for (int i=1;i<v.size();++i)
        *this << v[i] / total_stockable_area;
    writeRow();

}

