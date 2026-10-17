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

#include "saplingout.h"
#include "model.h"
#include "resourceunit.h"
#include "saplings.h"
#include "species.h"

SaplingOut::SaplingOut()
{

    setName("Sapling Output", "sapling");
    setDescription("Output of the establishment/sapling layer per resource unit and species.\n" \
                   "The output covers trees between a dbh of 1cm (height>1.3m) and the recruitment threshold (i.e. a height of 4m)." \
                   "Cohorts with a dbh < 1cm are counted in 'cohort_count_ha' but not used for average calculations.\n\n" \
                   "You can specify a 'condition' to limit execution for specific time/ area with the variables 'ru' (resource unit id) and 'year' (the current year)");
    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id() << OutputColumn::species()
            << OutputColumn("count_ha", "number of represented individuals per ha (tree height >1.3m).", OutInteger)
               << OutputColumn("count_small_ha", "number of represented individuals per ha (with height <=1.3m).", OutInteger)
            << OutputColumn("cohort_count_ha", "number of cohorts per ha.", OutInteger)
            << OutputColumn("height_avg_m", "arithmetic average height of the cohorts (m) ", OutDouble)
            << OutputColumn("age_avg", "arithmetic average age of the sapling cohorts (years)", OutDouble)
            << OutputColumn("LAI", "leaf area index of the regeneration layer (m2/m2)", OutDouble);
 }

void SaplingOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);
    if (!mCondition.isEmpty()) {
        mVarRu = mCondition.addVar("ru");
        mVarYear = mCondition.addVar("year");
    }
}

void SaplingOut::exec()
{
    Model *m = GlobalSettings::instance()->model();
    if (!GlobalSettings::instance()->model()->saplings())
        return;


    double avg_dbh, avg_height, avg_age;
    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area

        if (!mCondition.isEmpty()) {
            *mVarRu = ru->id();
            *mVarYear = GlobalSettings::instance()->currentYear();
            if (!mCondition.execute() )
                continue;
        }

        foreach(const ResourceUnitSpecies *rus, ru->ruSpecies()) {
            const StandStatistics &stat = rus->constStatistics();
            const SaplingStat &sap = const_cast<ResourceUnitSpecies*>(rus)->saplingStat();

            if (stat.cohortCount()==0)
                continue;
            *this << currentYear() << ru->index() << ru->id() << rus->species()->id(); // keys

            // calculate statistics based on the number of represented trees per cohort
            sap.livingStemNumber(rus->species(), avg_dbh, avg_height, avg_age);
            *this << sap.livingSaplings()
                  << sap.livingSaplingsSmall()
                  << sap.livingCohorts()
                  << sap.averageHeight()
                  << sap.averageAge()
                  << sap.leafAreaIndex();
            writeRow();
        }
    }
}

SaplingDetailsOut::SaplingDetailsOut()
{
    setName("Sapling Details Output", "saplingdetail");
    setDescription("Detailed output on indidvidual sapling cohorts.\n" \
                   "For each occupied and living 2x2m pixel, a row is generated, unless" \
                   "the tree diameter is below the 'minDbh' threshold (cm). " \
                   "You can further specify a 'condition' to limit execution for specific time/ area with the variables 'ru' (resource unit id) and 'year' (the current year).");
    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id() << OutputColumn::species()
              << OutputColumn("position", "location of the cell within the resource unit; a number between 0 (lower left corner) and 2499 (upper right corner) (x=index %% 50; y=floor(index / 50) ).", OutInteger)
              << OutputColumn("n_represented", "number of trees that are represented by the cohort (Reineke function).", OutDouble)
              << OutputColumn("dbh", "diameter of the cohort (cm).", OutDouble)
              << OutputColumn("height", "height of the cohort (m).", OutDouble)
              << OutputColumn("age", "age of the cohort (years) ", OutInteger);

}

void SaplingDetailsOut::exec()
{
    Model *m = GlobalSettings::instance()->model();
    if (!GlobalSettings::instance()->model()->saplings())
        return;

    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area

        // exclude if a condition is specified and condition is not met
        if (!mCondition.isEmpty()) {
            *mVarRu = ru->id();
            *mVarYear = GlobalSettings::instance()->currentYear();
            if (!mCondition.execute() )
                continue;
        }
        SaplingCell *s = ru->saplingCellArray();
        for (int px=0;px<cPxPerHectare;++px, ++s) {
            int n_on_px = s->n_occupied();
            if (n_on_px>0) {
                for (int i=0;i<NSAPCELLS;++i) {
                    if (s->saplings[i].is_occupied()) {
                        ResourceUnitSpecies *rus = s->saplings[i].resourceUnitSpecies(ru);
                        const Species *species = rus->species();
                        double dbh = s->saplings[i].height / species->saplingGrowthParameters().hdSapling  * 100.;
                        // check minimum dbh
                        if (dbh<mMinDbh)
                            continue;

                        double n_repr = species->saplingGrowthParameters().representedStemNumberH(s->saplings[i].height) / static_cast<double>(n_on_px);

                        *this <<  currentYear() << ru->index() << ru->id() << rus->species()->id() << px;
                        *this << n_repr << dbh << s->saplings[i].height << s->saplings[i].age;
                        writeRow();
                    }
                }
            }
        }
    }
}

void SaplingDetailsOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);
    if (!mCondition.isEmpty()) {
        mVarRu = mCondition.addVar("ru");
        mVarYear = mCondition.addVar("year");
    }
    mMinDbh = settings().valueDouble(".minDbh");
}
