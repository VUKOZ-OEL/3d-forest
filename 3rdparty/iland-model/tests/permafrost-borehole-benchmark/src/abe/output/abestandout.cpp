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
#include "abestandout.h"
#include "globalsettings.h"

#include "forestmanagementengine.h"
#include "fmstand.h"
#include "fmunit.h"
#include "scheduler.h"
#include "species.h"

namespace ABE {

ABEStandOut::ABEStandOut()
{
    setName("Annual stand output (state).", "abeStand");
    setDescription("This output provides details about the forest state on stand level. " \
                   "The timber is provided as standing timber per hectare. \n" \
                   "The output is rather performance critical. You can use the ''condition'' XML-tag to limit the execution to certain years (e.g., mod(year,10)=1 ).");
    columns() << OutputColumn::year()
              << OutputColumn("unitid", "unique identifier of the planning unit", OutString)
              << OutputColumn("standid", "unique identifier of the forest stand", OutInteger)
              << OutputColumn("initialstandid", "stand id if not split, stand id of the source stand after splitting a stand.", OutInteger)
              << OutputColumn("area", "total area of the forest stand (ha)", OutDouble)
              << OutputColumn("volume", "standing timber volume (after harvests of the year) (m3/ha)", OutDouble)
              << OutputColumn("basalarea", "basal area (trees >4m) (m2/ha)", OutDouble)
              << OutputColumn("dbh", "mean diameter (basal area weighted, of trees >4m) (cm)", OutDouble)
              << OutputColumn("height", "mean stand tree height (basal area weighted, of trees >4m)(m)", OutDouble)
              << OutputColumn("topHeight", "dominant height (mean tree height of the 100 thickest trees >4m)(m)", OutDouble)
              << OutputColumn("stems", "number of trees (trees >4m) per ha", OutDouble)
              << OutputColumn("age", "the age of the stand (years since beginning of the rotation)", OutDouble)
                 ;
}

void ABEStandOut::exec()
{
    if (!mCondition.isEmpty())
        if (!mCondition.calculate(GlobalSettings::instance()->currentYear()))
            return;

    foreach(FMStand *stand, ForestManagementEngine::instance()->stands()) {

        // Note: EXPENSIVE reload operation for every stand and every year....
        stand->reload();

        *this << currentYear();
        *this << stand->unit()->id() << stand->id() << stand->initialStandId() << stand->area();
        *this << qRound(stand->volume()*100.)/100.;
        *this << qRound(stand->basalArea()*100.)/100.;
        *this << qRound(stand->dbh()*100.)/100.;
        *this << qRound(stand->height()*100.)/100.;
        *this << qRound(stand->topHeight()*100.)/100.;
        *this << qRound(stand->stems());
        *this << stand->absoluteAge();
        writeRow();

    }
}

void ABEStandOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);

}

ABEStandDetailsOut::ABEStandDetailsOut()
{
    setName("Detailed annual stand output (state).", "abeStandDetail");
    setDescription("This output provides details about the forest state on species- and stand level. " \
                   "This output is more detailed than the abeStand output.\n" \
                   "The output is rather performance critical. You can use the ''condition'' XML-tag to limit the execution to certain years (e.g., mod(year,10)=1 ).");
    columns() << OutputColumn::year()
              << OutputColumn::species()
              << OutputColumn("standid", "unique identifier of the forest stand", OutInteger)
              << OutputColumn("basalarea", "basal area of the species(trees >4m) (m2/ha)", OutDouble)
              << OutputColumn("relBasalarea", "relative basal area share of the species (trees >4m) (0..1)", OutDouble)
                 ;

}

void ABEStandDetailsOut::exec()
{
    if (!mCondition.isEmpty())
        if (!mCondition.calculate(GlobalSettings::instance()->currentYear()))
            return;

    foreach(FMStand *stand, ForestManagementEngine::instance()->stands()) {

        // Note: EXPENSIVE reload operation for every stand and every year....
        stand->reload();

        // loop over all species
        for (int i = 0; i<stand->nspecies(); ++i) {
            SSpeciesStand &sss = stand->speciesData(i);
            *this << currentYear();
            *this << sss.species->id();
            *this << stand->id();
            *this << sss.basalArea;
            *this << sss.relBasalArea;
            writeRow();
        }

    }


}

void ABEStandDetailsOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);

}




} // namespace
