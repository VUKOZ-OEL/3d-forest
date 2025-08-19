#include "svdindicatorout.h"

#include "spatialanalysis.h"
#include "resourceunit.h"
#include "soil.h"
#include "species.h"

#include "forestmanagementengine.h"
#include "fmstand.h"

/*  ***********************************************************************  */
/*  *********************  SVD Indicator output ***************************  */
/*  ***********************************************************************  */

// list of available indicators:
static const QStringList svd_indicators = QStringList() << "shannonIndex" << "abovegroundCarbon" << "totalCarbon" <<
                                          "volume" << "crownCover" << "LAI" << "basalArea" << "stemDensity" << "saplingDensity" <<
                                          "IBP";


SVDIndicatorOut::SVDIndicatorOut()
{
    setName("SVD forest indicator data", "svdindicator");
    setDescription("Indicator data per resource unit as used by SVD.\n " \
                   "The selection of indicators is triggered by keys in the project file (sub section 'indicators').\n " \
                   "!!! indicators\n\n" \
                   "The following columns are supported:\n\n" \
                   "||__key__|__description__\n" \
                   "shannonIndex|shannon index (exponential) on the RU (based on basal area of trees >4m)\n" \
                   "abovegroundCarbon|living aboveground carbon (tC/ha) on the RU (trees + regen)\n" \
                   "totalCarbon|all C on the RU (tC/ha), including soil, lying and standing deadwood\n" \
                   "volume|tree volume (trees>4m) m3/ha\n" \
                   "crownCover|fraction of crown cover (0..1) (see saveCrownCoverGrid() in SpatialAnalysis - not yet implemented)\n" \
                   "LAI|leaf area index (trees>4m) m2/m2\n" \
                   "basalArea|basal area (trees>4m) m2/ha\n" \
                   "stemDensity|trees per ha (trees>4m) ha-1\n" \
                   "saplingDensity|density of saplings (represented trees>1.3m) ha-1||\n" \
                   "IBP|Index of Biodiversity Potential (adapted, Emberger et al 2023)\n\n" \
                   "!!! species proportions\n" \
                   "A special case is the setting 'speciesProportions': this is a list of species (Ids) separated with a comma or white space. When present, the output will " \
                   " include for each species the relative proportion calculated based on basal area (for trees >4m). \n" \
                   " \n" \
                   "!!! disturbance history\n" \
                   "The setting 'disturbanceHistory' indicates if (value > 0) and how many (value>0, maximum=3) disturbance events should be recorded and added to the " \
                   "output. Each __event__ is defined by three columns. 'tsd_x' is number of years since disturbance (0 if the disturbance happended in the current year), 'type_x' encodes the disturbance " \
                   "agent (see below), and 'addinfo_x' is agent-specific additional information (see below), with 'x' the number of event (1,2,3).\n\n" \
                   "||__value__|__type__|__additional info__\n" \
                   "0|fire|proportion of area burned per ha (0..1) \n" \
                   "1|(spruce) bark beetle|NA\n" \
                   "2|wind|NA \n" \
                   "3|BITE|NA \n" \
                   "4|ABE|NA \n" \
                   "5|base management|NA|| \n\n" \
                   "!!! example \n\n" \
                   "An example for the project file node:\n" \
                   "<indicators>\n<shannonIndex>true</shannonIndex>\n<abovegroundCarbon>false</abovegroundCarbon>\n ... \n" \
                   "<speciesProportions>Pico,Abal</speciesProportions>\n" \
                   "<disturbanceHistory>2 </disturbanceHistory>\n</indicators>\n"
                   ); // //dtFire, dtBarkBeetle, dtWind, dtBite, dtAbe, dtManagement
    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id()
              << OutputColumn("stateId", "current state of the resource unit (see 'svdstate' output)", OutInteger)
              << OutputColumn("time", "number of years the resource unit is already in the state 'stateId' (see 'svdstate' output)", OutInteger);


}

void SVDIndicatorOut::setup()
{
    // clear extra columns:
    clearColumnsAfter("time");

    // use a condition for to control execuation for the current year
    XmlHelper indicators(settings().node(".indicators"));
    // look for all defined indicators in the XML structure
    for (int i=0;i<svd_indicators.size();++i) {
        if (indicators.valueBool(QString(".%1").arg(svd_indicators[i]))) {
            // set active
            mIndicators[i] = true;
            // add to output table
            columns() << OutputColumn(svd_indicators[i], QString(), OutDouble);
        }
    }
    // special case for species proportions
    QString specieslist = indicators.value(".speciesProportions");
    if (!specieslist.isEmpty()) {
        // extract list of species, create columns and store the name for later use
        QStringList species_list = specieslist.split(QRegularExpression("([^\\.\\w]+)"));
        mSpecies.clear();
        for (int i=0;i<species_list.size();++i) {
            mSpecies.push_back( QPair<QString, int>(species_list[i], -1));
            columns() << OutputColumn(QString("prop_%1").arg(species_list[i]), QString(), OutDouble);
        }
        qDebug() << "SVDIndicatorOut: setup relative species proportions for" << species_list.count() << "species.";

    }
    // species case disturbance history
    mNDisturbanceHistory = indicators.valueInt(".disturbanceHistory", 0);
    if (mNDisturbanceHistory > 0) {
        // add columns: for each event we need 3 columns
        for (int i=0;i<mNDisturbanceHistory;++i) {
            // time since disturbance, type, additional info
            columns() << OutputColumn(QString("tsd_%1").arg(i+1), QString(), OutInteger);
            columns() << OutputColumn(QString("type_%1").arg(i+1), QString(), OutInteger);
            columns() << OutputColumn(QString("addinfo_%1").arg(i+1), QString(), OutDouble);
        }
    }



    qDebug() << "SVDIndicatorOut: setup indicators: " << mIndicators.count() << "active. Details: " <<  QString::fromStdString(mIndicators.to_string());

}

double SVDIndicatorOut::calcShannonIndex(const ResourceUnit *ru)
{
    // calculate shannon index from the given data [I did this already for PICUS...]:
    // see also ARANGE project D2.2, 4.2.2
    double total_ba = ru->statistics().basalArea();
    if (total_ba==0.)
        return 0.;

    // loop over each species:
    double shannon = 0.;
    for (QList<ResourceUnitSpecies*>::const_iterator it=ru->ruSpecies().constBegin(); it!=ru->ruSpecies().constEnd();++it){
        double ba = (*it)->statistics().basalArea();
        if (ba>0.)
            shannon += ba/total_ba * log(ba/total_ba);
    }

    // 'true diversity' is the exponent of the shannon index:
    double exp_shannon = exp( -shannon );
    return exp_shannon;

}

double SVDIndicatorOut::calcCrownCover(const ResourceUnit *ru)
{
    Model *model = GlobalSettings::instance()->model();
    GridRunner<float> runner(mCrownCoverGrid, ru->boundingBox());
    int cc_sum = 0;
    while (float *gv = runner.next()) {
        if (model->heightGridValue(runner.currentIndex().x(),
                                   runner.currentIndex().y()).isValid())
            if (*gv >= 0.5f) // 0.5: half of a 2m cell is covered by a tree crown; is a bit pragmatic but seems reasonable (and works)
                cc_sum++;
    }
    double value = cPxSize*cPxSize*cc_sum / cRUArea;
    return limit(value, 0., 1.);
}

double SVDIndicatorOut::calcTotalCarbon(const ResourceUnit *ru)
{
    double total_carbon = ru->statistics().totalCarbon() / 1000.; // aboveground, kg C/ha -> tC/ha
    double area_factor = ru->stockableArea() / cRUArea; // conversion factor from real area to per ha values
    if (ru->snag())
        total_carbon +=  ru->snag()->totalCarbon() / 1000. / area_factor; // kgC/RU -> tC/ha
    if (ru->soil())
        total_carbon += ru->soil()->totalCarbon(); // t/ha
    return total_carbon;
}

void SVDIndicatorOut::addSpeciesProportions(const ResourceUnit *ru)
{
    if (mSpecies.isEmpty())
        return;

    // do only once:
    if (mSpecies[0].second == -1) {
        for (int i=0;i<mSpecies.size();++i) {
            Species *s = Globals->model()->speciesSet()->species(mSpecies[i].first);
            if (!s)
                throw IException(QString("Setup SVDIndicatorOut: Species '%1' is not available!").arg(mSpecies[i].first));
            mSpecies[i].second = s->index(); // save index for later use
        }
    }

    double total_ba = std::max(ru->statistics().basalArea(), 0.00001);
    for (int i=0;i<mSpecies.size();++i) {
        double prop = ru->ruSpecies()[mSpecies[i].second]->constStatistics().basalArea() / total_ba;
        // add to the output
        *this << prop;
    }
}

void SVDIndicatorOut::addDisturbanceHistory(const ResourceUnit *ru)
{
    if (mNDisturbanceHistory == 0) return;

    for (int i=0;i<mNDisturbanceHistory;++i) {
        if (i < ru->mSVDState.disturbanceEvents->size()) {
            const ResourceUnit::RUSVDState::SVDDisturbanceEvent &e =  (*ru->mSVDState.disturbanceEvents)[i];
            *this << Globals->currentYear() - e.year; // time since disturbance
            *this << e.source; // type of disturbance
            *this << e.info; // some additional information
        } else {
            *this << 0 << 0 << 0; // no data available
        }
    }
}

double SVDIndicatorOut::calcIBP(const ResourceUnit *ru)
{
    // calculate Index ob Biodiversity Potential
    // based on Larrieu and Gonin 2008, Emberger et al 2023.
    // see thesis of Sarah Pfaffl (2024)

    if (!ru->soil() || !ru->snag())
        throw IException("SVDIndicator: IBP: requires carbon cycling.");


    int IBP = 0;

    // factor A: species diversity based on number of tree Genera in the stand (trees > 0.5m)
    // simplify: use tree species diversity (>4m)
    int n_species = 0;
    for (QList<ResourceUnitSpecies*>::const_iterator it=ru->ruSpecies().constBegin(); it!=ru->ruSpecies().constEnd();++it){
        if ((*it)->statistics().count() > 0)
            ++n_species;
    }
    int f_species = 0;
    if (n_species >= 3)
        f_species = 5; // full number of points
    else
        f_species += n_species; // 0,1,,2

    IBP += f_species;

    // factor B: number of vertical layers. A layer is accounted for, if covered by >20% of the ground.
    // Counted is the height of foliage. layers are: ground-veg, 0-1.5m, 1.5m-7m, 7-20m, >20m
    // update: no ground-veg; 0-1.5m -> regen layer. 20% rule: simplify to n>50 stems/ha (<20m), and n>20/ha (>20m), ground-veg: 20% occupation of pixel
    // we assume that ground-veg is always here
    int n_layers = 1;

    if (ru->statistics().cohortCount() > cPxPerHectare*0.2)
        ++n_layers;

    // counters for layers
    int n_low=0, n_mid=0, n_high=0;
    // counters for very large trees
    int n_verylargetree = 0, n_largetree = 0;
    for (const auto &t : ru->constTrees()) {
        double c_low = t.height()/2.;
        double c_mid = c_low * 1.5; // mid of the crown
        if (c_mid < 7.)
            ++n_low; // >50% of crown in lower class
        else if (c_mid < 20.)
            ++n_mid; // crown center in middle class
        else
            ++n_high;

        if (t.dbh() > 67.5) ++n_verylargetree;
        if (t.dbh() > 47.5) ++n_largetree;
    }
    if (n_low > 100) ++n_layers;
    if (n_mid > 50) ++n_layers;
    if (n_high > 20) ++ n_layers;

    int f_layers = 0;
    if (n_layers == 5) f_layers +=5;
    else if (n_layers == 3 || n_layers==4) f_layers += 2;
    else if (n_layers == 2) f_layers += 1;

    IBP += f_layers;

    // factor C: large standing deadwood
    double min_dbh, max_dbh;
    Snag::snagThresholds(min_dbh, max_dbh);
    if (min_dbh != 17.5 || max_dbh != 37.5)
        throw IException("SVDIndcator:IBP: this requires the settings of swdDBHClass12 and swdDBHClass23 to be 17.5cm and 37.5cm");

    auto *sn = ru->snag()->numberOfSnags();
    int f_swd = 0;
    if (sn[2]>= 3.) f_swd = 5;
    else if(sn[2] >= 1.) f_swd = 2;
    else if(sn[2] < 1. && sn[1] > 1.) f_swd = 1;

    IBP += f_swd;

    // factor D: downed deadwood
    // same DBH thresholds
    // iLand implementation: we have no specific stems in downed deadwood. We therefore estimate
    // the propoprtion of large trees similar to the prop of large snags
    double prop_largesnags = 0;
    if (sn[0] + sn[1] > 0.) prop_largesnags = sn[2] / (sn[0] + sn[1] + sn[2]);

    double bm_deadwood = ru->soil()->youngRefractory().biomass() * ru->soil()->youngRefractoryAbovegroundFraction();
    double n_large =  (bm_deadwood * prop_largesnags) / 450.; // kg/ha /  450kg/m3 -> m3; I assume 1 m3 per tree
    double n_medium = (bm_deadwood * (1.-prop_largesnags)) / 450.;

    int f_dwd = 0;
    if (n_large>= 3.) f_dwd = 5;
    else if(n_large >= 1.) f_dwd = 2;
    else if(n_large < 1. && n_medium > 1.) f_dwd = 1;

    IBP += f_dwd;

    // factor E: number of very large trees (>67.5cm DBH) and large trees (>47.5cm DBH)
    int f_largetrees = 0;
    if (n_verylargetree >= 5) f_largetrees = 5;
    else if (n_verylargetree >=1) f_largetrees = 2;
    else if (n_largetree >=1) f_largetrees = 1;

    IBP += f_largetrees;

    // factor F: microhabitats: not considered

    // factor G: %area that is open for flowering plants
    // implementation in iLand: we use the crown cover
    double crown_cover = calcCrownCover(ru);

    int f_ccover = 0;
    if ( crown_cover < 0.9) f_ccover = 2;

    IBP += f_ccover;

    // factor H: continuity and forest management
    // we assume a disturbance due to managment when management happened in the last 20 yrs
    int f_continuity = 5;
    if (GlobalSettings::instance()->model()->ABEngine()) {
        ABE::FMStand *stand = GlobalSettings::instance()->model()->ABEngine()->standAt(ru->boundingBox().center());
        if (stand) {
            if (stand->lastExecution() > -1 && GlobalSettings::instance()->currentYear() - stand->lastExecution() < 20) {
                if (stand->age() < 20 && stand->volume() < 100)
                    f_continuity = 0; // we assume full clearcut after a rotation when standing volume is low
                else
                    f_continuity = 2;
            }

        }
    }
    IBP += f_continuity;

    // factor I and J: aquatic  (I) and rocky biotops (J): not considered

    // this gives a maximum number of points of 7 x 5 = 35
    if (logLevelDebug())
        qDebug() << "SVDIndicator:IBP: RID:" << ru->id() <<
                  "IBP: " << IBP << " Details (A B C D E G H):" <<
                    f_species << f_layers << f_swd << f_dwd << f_largetrees << f_ccover << f_continuity;

    double rel_IBP = IBP / 35.;

    return rel_IBP;


}

void SVDIndicatorOut::exec()
{
    if (!GlobalSettings::instance()->model()->svdStates()) {
        qWarning() << "Output SVDIndicatorOut cannot be used, because it requires the 'svdstate' output (and the SVD subsystem ('model.settings.svdStates.enabled')). Output disabled.";

        throw IException("Setup of SVDIndcatorOut: SVD states are required for this output ('model.settings.svdStates.enabled').");
    }

    // global calculations
    // we need to run crown projections for all trees...
    if (mIndicators.test(EcrownCover)) {
        if (mCrownCoverGrid.isEmpty())
            mCrownCoverGrid.setup(*GlobalSettings::instance()->model()->grid());

        SpatialAnalysis::runCrownProjection2m(&mCrownCoverGrid);

    }


    QList<ResourceUnit*>::const_iterator it;
    Model *m = GlobalSettings::instance()->model();
    for (it=m->ruList().constBegin(); it!=m->ruList().constEnd(); ++it) {
        if ((*it)->id()==-1)
            continue; // do not include if out of project area

        *this << currentYear() << (*it)->index() << (*it)->id();
        *this << (*it)->mSVDState.stateId
              << (*it)->mSVDState.time;

        // process indicators:
        // Note: sequence important: see string list svd_indicators !
        if (mIndicators.test(EshannonIndex))
            *this << calcShannonIndex(*it);
        if (mIndicators.test(EabovegroundCarbon))
            *this << (*it)->statistics().totalCarbon() / 1000.; // trees + regen, t C/ha
        if (mIndicators.test(EtotalCarbon))
            *this << calcTotalCarbon(*it); //
        if (mIndicators.test(Evolume))
            *this << (*it)->statistics().volume();
        if (mIndicators.test(EcrownCover))
            *this << calcCrownCover(*it);
        if (mIndicators.test(ELAI))
            *this << (*it)->statistics().leafAreaIndex(); // LAI trees > 4m
        if (mIndicators.test(EbasalArea))
            *this << (*it)->statistics().basalArea();
        if (mIndicators.test(EstemDensity))
            *this << (*it)->statistics().count();
        if (mIndicators.test(EsaplingDensity))
            *this << (*it)->statistics().saplingCount();
        if (mIndicators.test(EIBP))
            *this << calcIBP(*it);

        addSpeciesProportions(*it);

        addDisturbanceHistory(*it);

        writeRow();
    }


}
