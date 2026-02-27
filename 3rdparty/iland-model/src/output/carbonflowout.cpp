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

#include "carbonflowout.h"
#include "globalsettings.h"
#include "model.h"
#include "resourceunit.h"
#include "resourceunitspecies.h"
#include "production3pg.h"
#include "soil.h"

CarbonFlowOut::CarbonFlowOut()
{

    setName("Carbon fluxes per RU or landscape/yr", "carbonflow");
    setDescription("Carbon fluxes per resource unit and year and/or aggregated for the full landscape. All values are reported on a per hectare basis (use the area provided in carbon or stand outputs to scale to realized values on the respective resource unit). " \
                   "For results limited to the project area, the data values need to be scaled to the stockable area.\n" \
                   "For landsacpe level outputs, data is always given per ha of (stockable) project area (i.e. scaling with stockable area is already included).\n" \
                   "Furthermore, the following sign convention is used in iLand: fluxes "\
                   "from the atmosphere to the ecosystem are positive, while C leaving the ecosystem is reported as negative C flux.\n" \
                   "You can specify a 'condition' to limit output execution to specific years (variable 'year'). " \
                   "The 'conditionRU' can be used to suppress resource-unit-level details; eg. specifying 'in(year,100,200,300)' limits output on reosurce unit level to the years 100,200,300 " \
                   "(leaving 'conditionRU' blank enables details per default).");


    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id()
              << OutputColumn("area_ha", "total stockable area of the resource unit (or landscape) (ha)", OutDouble)
              << OutputColumn("GPP", "actually realized gross primary production, kg C; ((primary production|GPP)) including " \
                                         "the effect of decreasing productivity with age; note that a rough estimate of "\
                                         "((sapling growth and competition|#sapling C and N dynamics|sapling GPP)) is added to the GPP of adult trees here.", OutDouble)
              << OutputColumn("NPP", "net primary production, kg C; calculated as NPP=GPP-Ra; Ra, the autotrophic respiration (kg C/ha) is calculated as"\
                                     " a fixed fraction of GPP in iLand (see ((primary production|here)) for details). ", OutDouble)
              << OutputColumn("Rh", "heterotrophic respiration, kg C; sum of C released to the atmosphere from detrital pools, i.e."\
                                    " ((snag dynamics|#Snag decomposition|snags)), ((soil C and N cycling|downed deadwood, litter, and mineral soil)).", OutDouble)
              << OutputColumn("dist_loss", "disturbance losses, kg C; C that leaves the ecosystem as a result of disturbances, e.g. fire consumption", OutDouble)
              << OutputColumn("mgmt_loss", "management losses, kg C; C that leaves the ecosystem as a result of management interventions, e.g. harvesting", OutDouble)
              << OutputColumn("NEP", "net ecosytem productivity kg C, NEP=NPP - Rh - disturbance losses - management losses. "\
                                     "Note that NEP is also equal to the total net changes over all ecosystem C pools, as reported in the "\
                                     "carbon output (cf. [http://www.jstor.org/stable/3061028|Randerson et al. 2002])", OutDouble)
              << OutputColumn("cumNPP", "cumulative NPP, kg C. This is a running sum of NPP (including tree NPP and sapling carbon gain).", OutDouble)
              << OutputColumn("cumRh", "cumulative flux to atmosphere (heterotrophic respiration), kg C. This is a running sum of Rh.", OutDouble)
              << OutputColumn("cumNEP", "cumulative NEP (net ecosystem productivity), kg C. This is a running sum of NEP (positive values: carbon gain, negative values: carbon loss).", OutDouble);
}

void CarbonFlowOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);

    condition = settings().value(".conditionRU", "");
    mConditionDetails.setExpression(condition);

}


void CarbonFlowOut::exec()
{
    Model *m = GlobalSettings::instance()->model();

    // global condition
    if (!mCondition.isEmpty() && mCondition.calculate(GlobalSettings::instance()->currentYear())==0.)
        return;

    bool ru_level = true;
    // switch off details if this is indicated in the conditionRU option
    if (!mConditionDetails.isEmpty() && mConditionDetails.calculate(GlobalSettings::instance()->currentYear())==0.)
        ru_level = false;

    double npp = 0.;
    int ru_count = 0;
    QVector<double> v(10, 0.); // 11 data values
    QVector<double>::iterator vit;

    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area
        if (!ru->soil() || !ru->snag()) {
            qDebug() << "CarbonFlowOut::exec: resource unit without soil or snags module - no output generated.";
            continue;
        }


        npp = 0.;
        double area_factor = ru->stockableArea() / cRUArea; //conversion factor
        npp += ru->statistics().npp() * biomassCFraction; // kg C/ha
        npp += ru->statistics().nppSaplings() * biomassCFraction; // kgC/ha
        // Snag pools are not scaled per ha (but refer to the stockable RU), soil pools and biomass statistics (NPP, ...) are scaled.
        double to_atm = ru->snag()->fluxToAtmosphere().C / area_factor; // from snags, kg/ha
        to_atm += ru->soil()->fluxToAtmosphere().C * cRUArea/10.; // soil: t/ha -> t/m2 -> kg/ha

        double to_dist = ru->snag()->fluxToDisturbance().C / area_factor; // convert to kgC/ha
        to_dist += ru->soil()->fluxToDisturbance().C * cRUArea/10.; // kgC/ha

        double to_harvest = ru->snag()->fluxToExtern().C / area_factor; // kgC/ha

        double nep = npp - to_atm - to_harvest - to_dist; // kgC/ha

        if (ru_level) {
            *this << currentYear() << ru->index() << ru->id() << area_factor; // keys
            *this <<  npp / cAutotrophicRespiration // GPP_act
                    << npp // NPP
                    << -to_atm // rh
                    << -to_dist // disturbance
                    << -to_harvest // management loss
                    << nep; // nep
            *this << ru->resouceUnitVariables().cumCarbonUptake << ru->resouceUnitVariables().cumCarbonToAtm << ru->resouceUnitVariables().cumNEP;

            writeRow();
        }
        // landscape level
        ++ru_count;
        vit = v.begin();
        *vit++ += area_factor; // total area in ha
        *vit++ += npp / cAutotrophicRespiration * area_factor; // GPP_act
        *vit++ += npp * area_factor; // NPP
        *vit++ += -to_atm * area_factor; // rh
        *vit++ += -to_dist * area_factor; // disturbance
        *vit++ += -to_harvest * area_factor; // management loss
        *vit++ += nep *area_factor; // net ecosystem productivity
        *vit++ += ru->resouceUnitVariables().cumCarbonUptake * area_factor; // cum. NPP
        *vit++ += ru->resouceUnitVariables().cumCarbonToAtm * area_factor; // cum. Rh
        *vit++ += ru->resouceUnitVariables().cumNEP * area_factor; // cum. NEP

    }

    // write landscape sums
    double total_stockable_area = v[0]; // total ha of stockable area
    if (ru_count==0. || total_stockable_area==0.)
        return;
    *this << currentYear() << -1 << -1; // codes -1/-1 for landscape level
    *this << v[0]; // stockable area [m2]
    for (int i=1;i<v.size();++i)
        *this << v[i] / total_stockable_area;
    writeRow();


}

