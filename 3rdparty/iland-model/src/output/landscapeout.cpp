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

#include "landscapeout.h"
#include "model.h"
#include "resourceunit.h"
#include "species.h"

LandscapeOut::LandscapeOut()
{
    setName("Landscape aggregates per species", "landscape");
    setDescription("Output of aggregates on the level of landscape x species. Values are always aggregated per hectare. "\
                   "The output is created after the growth of the year, " \
                   "i.e. output with year=2000 means effectively the state of at the end of the " \
                   "year 2000. The initial state (without any growth) is indicated by the year 'startyear-1'." \
                   "You can use the 'condition' to control if the output should be created for the current year(see also dynamic stand output)");
    columns() << OutputColumn::year()
              << OutputColumn("area", "total stockable area of the simulated landscape with the grain of 10m cells (ha)", OutDouble)
              << OutputColumn("area_100m", "total area of all simulated resource units (with the grain 100m). This area is larger then 'area', when (some) RUs are only partially stockable.", OutDouble)
              << OutputColumn::species()
              << OutputColumn("count_ha", "tree count (living, >4m height) per ha", OutInteger)
              << OutputColumn("dbh_avg_cm", "average dbh (cm)", OutDouble)
              << OutputColumn("height_avg_m", "average tree height (m)", OutDouble)
              << OutputColumn("volume_m3", "volume (geomery, taper factor) in m3", OutDouble)
              << OutputColumn("total_carbon_kg", "total carbon in living biomass (aboveground compartments and roots) of all living trees (including regeneration layer) (kg/ha)", OutDouble)
              << OutputColumn("gwl_m3", "'gesamtwuchsleistung' (total growth including removed/dead trees) volume (geomery, taper factor) in m3", OutDouble)
              << OutputColumn("basal_area_m2", "total basal area at breast height (m2)", OutDouble)
              << OutputColumn("NPP_kg", "sum of NPP (aboveground + belowground) kg Biomass/ha", OutDouble)
              << OutputColumn("NPPabove_kg", "sum of NPP (abovegroundground) kg Biomass/ha", OutDouble)
              << OutputColumn("LAI", "Leafareaindex (m2/m2)", OutDouble)
              << OutputColumn("cohort_count_ha", "number of cohorts in the regeneration layer (<4m) /ha", OutInteger);

 }

void LandscapeOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);
}

void LandscapeOut::exec()
{
    Model *m = GlobalSettings::instance()->model();
    if (!mCondition.isEmpty())
        if (!mCondition.calculate(GlobalSettings::instance()->currentYear()))
            return;

    // clear landscape stats
    for (QMap<QString, StandStatistics>::iterator i=mLandscapeStats.begin(); i!= mLandscapeStats.end();++i)
        i.value().clear();

    //  total stockable area (in m2)
    double total_area = m->totalStockableArea() * cRUArea;

    if (total_area==0.)
        return;

    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area
        foreach(const ResourceUnitSpecies *rus, ru->ruSpecies()) {
            const StandStatistics &stat = rus->constStatistics();
            if (stat.count()==0. && stat.cohortCount()==0 && stat.gwl()==0.) {
                continue;
            }
            mLandscapeStats[rus->species()->id()].addAreaWeighted(stat, ru->stockableArea() / total_area);
        }
    }
    // now add to output stream
    QMap<QString, StandStatistics>::iterator i = mLandscapeStats.begin();
    while (i != mLandscapeStats.end()) {
        StandStatistics &stat = i.value();
        stat.calculateAreaWeighted(); // calculate average dbh, height

        *this << currentYear() << m->totalStockableArea() << m->ruList().size();
        *this << i.key(); // keys: year, species
        *this << stat.count() << stat.dbh_avg() << stat.height_avg()
                              << stat.volume() << stat.totalCarbon() << stat.gwl() << stat.basalArea()
                              << stat.npp() << stat.nppAbove() << stat.leafAreaIndex() << stat.cohortCount();
        writeRow();
        ++i;
    }
}

LandscapeRemovedOut::LandscapeRemovedOut()
{
    mIncludeDeadTrees = false;
    mIncludeHarvestTrees = true;

    setName("Aggregates of removed trees due to death, harvest, and disturbances per species", "landscape_removed");
    setDescription("Aggregates of all removed trees due to 'natural' death, harvest, or disturbance per species and reason. All values are totals for the whole landscape."\
                   "The user can select with options whether to include 'natural' death and harvested trees (which may slow down the processing). " \
                   "Set the setting in the XML project file 'includeNatural' to 'true' to include trees that died due to natural mortality, " \
                   "the setting 'includeHarvest' controls whether to include ('true') or exclude ('false') harvested trees.\n" \
                   "To enable output per dbh class, set the 'dbhClasses' setting to a comma delimeted list of dbh thresholds (e.g., '10,20,30,40,50'). The value in the output column " \
                   "'dbh_class' refers to the class (e.g.: 0: 0-10, 1: 10-20, 2: 20-30, 3: 30-40, 4: 40-50, 5: >=50). ");
    columns() << OutputColumn::year()
              << OutputColumn::species()
              << OutputColumn("dbh_class", "dbh class (see above). 0 if dbh classes are off.", OutInteger)
              << OutputColumn("reason", "Resaon for tree death: 'N': Natural mortality, 'H': Harvest (removed from the forest), 'D': Disturbance (not salvage-harvested), 'S': Salvage harvesting (i.e. disturbed trees which are harvested), 'C': killed/cut down by management", OutString)
              << OutputColumn("count", "number of died trees (living, >4m height) ", OutInteger)
              << OutputColumn("volume_m3", "sum of volume (geomery, taper factor) in m3", OutDouble)
              << OutputColumn("basal_area_m2", "total basal area at breast height (m2)", OutDouble)
              << OutputColumn("total_carbon", "total carbon (sum of stem, branch, foliage, coarse and fine roots) (kg C)", OutDouble)
              << OutputColumn("stem_c", "carbon in stems (kg C)", OutDouble)
              << OutputColumn("branch_c", "carbon on branch compartment (kg C)", OutDouble)
              << OutputColumn("foliage_c", "carbon in foliage (kg C)", OutDouble);

}

static QMutex protect_output;

void LandscapeRemovedOut::execRemovedTree(const Tree *t, int reason)
{
    Tree::TreeRemovalType rem_type = static_cast<Tree::TreeRemovalType>(reason);
    if (rem_type==Tree::TreeDeath && !mIncludeDeadTrees)
        return;
    if ((rem_type==Tree::TreeHarvest || rem_type==Tree::TreeSalavaged || rem_type==Tree::TreeCutDown) && !mIncludeHarvestTrees)
        return;

    QMutexLocker protector(&protect_output); // output creation can come from many threads

    int key = dbhClass(t->dbh())*100000 +  reason*10000 + t->species()->index();
    LROdata &d = mLandscapeRemoval[key];
    d.basal_area += t->basalArea();
    d.volume += t->volume();
    d.carbon += (t->biomassBranch()+t->biomassCoarseRoot()+t->biomassFineRoot()+t->biomassFoliage()+t->biomassStem())*biomassCFraction;
    d.cstem += t->biomassStem() * biomassCFraction;
    d.cbranch += t->biomassBranch() * biomassCFraction;
    d.cfoliage += t->biomassFoliage() * biomassCFraction;
    d.n++;


}


void LandscapeRemovedOut::exec()
{
    QHash<int,LROdata>::iterator i  = mLandscapeRemoval.begin();

    while (i != mLandscapeRemoval.end()) {
        if (i.value().n>0) {
            Tree::TreeRemovalType rem_type = static_cast<Tree::TreeRemovalType>((i.key() % 100000) / 10000 );
            int species_index = i.key() % 10000;
            int dbh_class = i.key() / 100000;
            *this << currentYear() << GlobalSettings::instance()->model()->speciesSet()->species(species_index)->id() << dbh_class;
            if (rem_type==Tree::TreeDeath) *this << QStringLiteral("N");
            if (rem_type==Tree::TreeHarvest) *this << QStringLiteral("H");
            if (rem_type==Tree::TreeDisturbance) *this << QStringLiteral("D");
            if (rem_type==Tree::TreeSalavaged) *this << QStringLiteral("S");
            if (rem_type==Tree::TreeCutDown) *this << QStringLiteral("C");
            *this << i.value().n << i.value().volume<< i.value().basal_area << i.value().carbon << i.value().cstem << i.value().cbranch << i.value().cfoliage;
            writeRow();
        }
        ++i;
    }

    // clear data (no need to clear the hash table, right?)
    i = mLandscapeRemoval.begin();
    while (i != mLandscapeRemoval.end()) {
        i.value().clear();
        ++i;
    }
}

void LandscapeRemovedOut::setup()
{
    mIncludeHarvestTrees = settings().valueBool(".includeHarvest", true);
    mIncludeDeadTrees = settings().valueBool(".includeNatural", false);

    mDBHThreshold.clear();
    QString dbh_cls = settings().value(".dbhClasses");
    if (!dbh_cls.isEmpty())
        setupDbhClasses(dbh_cls);
    Tree::setLandscapeRemovalOutput(this);

}

int LandscapeRemovedOut::dbhClass(float dbh)
{
    if (mDBHThreshold.size() == 0)
        return 0;
    int idbh = static_cast<int>(dbh);
    int cls = mDBHClass[ qMin(idbh, mMaxDbh) ];
    return cls;
}

void LandscapeRemovedOut::setupDbhClasses(QString cls_string)
{

    QStringList cls_list = cls_string.split(',');
    for (int i=0;i<cls_list.size();++i) {
        mDBHThreshold.push_back( cls_list[i].toInt()  );
    }
    qDebug() << "landscaperemoved output: use dbh classes:" << mDBHThreshold;
    mDBHThreshold.push_back(100000); // upper limit
    int current_cls = 0;
    mDBHClass.clear();
    for (int i=0;i<=mMaxDbh;++i) {
        if (i>=mDBHThreshold[current_cls])
            current_cls++;

        mDBHClass.push_back(current_cls);

    }
}
