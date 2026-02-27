#include "svdstate.h"

#include "resourceunit.h"
#include "species.h"
#include "model.h"

SVDStates *SVDState::svd = nullptr;

SVDStates::SVDStates()
{
    // add an empty state ("unforested") to the machine
    SVDState s = SVDState();
    mStates.push_back(s);
    mCompositionString.push_back( createCompositionString(s) );
    mStateLookup[mStates[0]]=0;
    s.svd = this;
    const XmlHelper &xml = GlobalSettings::instance()->settings();

    QString cls = xml.value("model.settings.svdStates.structure", "4m");
    if (cls=="4m")
        mStructureClassification = Structure4m;
    else if (cls == "2m")
        mStructureClassification = Structure2m;
    else
        throw IException(QString("Setup of SVD States: invalid value for 'structure': '%1', allowed values are '2m', '4m'.").arg(cls) );

    cls = xml.value("model.settings.svdStates.functioning", "3");
    if (cls=="3")
        mFunctioningClassification = Functioning3Classes;
    else if (cls == "5")
        mFunctioningClassification = Functioning5Classes;
    else
        throw IException(QString("Setup of SVD States: invalid value for 'functioning': '%1', allowed values are '3', '5'.").arg(cls) );


    qDebug() << "setup of SVDStates completed.";
}

int SVDStates::evaluateState(ResourceUnit *ru)
{
    SVDState s;
    bool rIrregular=false;
    double h = ru->topHeight(rIrregular);

    switch (mStructureClassification) {
        case Structure4m: {
            // normal height classes: 4m classes
            int hcls = limit( int(h/4), 0, 20); // 21 classes (0..20)
            if (h==4.)
                hcls = 0; // <4m: heightgrid height==4m

            if (rIrregular) {
                // irregular height classes: 12m steps:
                // 21: <12m, 22: 12-24m, 23: 24-36m, 24: 38-48m, 25: 48-60m, 26: >60m
                // irregular: >50% of the area < 50% of top height (which is 90th percentile of 10m pixels)
                hcls = 21 + limit(int(h/12), 0, 5);
            }
            s.structure = hcls;
            break;
        }
        case Structure2m: {
            // normal height classes: 2m classes
            int hcls = limit( int(h/2), 0, 30); // 31 classes (0..30)
            if (h==4.) {
                // 0-4m: differentiate: if saplings >2m exist, then class 1, otherwise class 0.
                if (GlobalSettings::instance()->model()->saplings()->topHeight(ru)>2.)
                    hcls=1;
                else
                    hcls=0;
            }
            if (rIrregular) {
                // irregular classes: 8m steps, max 56m: 31: <8m, 32: 8-16m, ... 37: 48-56m, 38: >=56m
                hcls = 31 + limit(int(h/8), 0, 7);
            }
            s.structure = hcls;

        }
    }


    double lai = ru->statistics().leafAreaIndex();
    lai+= ru->statistics().leafAreaIndexSaplings();

    switch (mFunctioningClassification) {
    case Functioning3Classes:
        s.function=0;
        if (lai>2.) s.function=1;
        if (lai>4.) s.function=2;
        break;
    case Functioning5Classes:
        s.function= limit(int(lai), 0, 4); //
        break;
    }


    // species
    int other_i = 0;

    double total_ba = ru->statistics().basalArea() + ru->statistics().saplingBasalArea();
    if (total_ba>0.) {
        QList<ResourceUnitSpecies*>::const_iterator it=ru->ruSpecies().constBegin();
        for (; it!=ru->ruSpecies().constEnd(); ++it) {
            double rel_ba = ((*it)->statistics().basalArea() + (*it)->statistics().saplingBasalArea()) / total_ba;
            if (rel_ba>0.66)
                s.dominant_species_index = (*it)->species()->index();
            else if (rel_ba>0.2)
                s.admixed_species_index[other_i++] = (*it)->species()->index();
        }
        if (other_i>=5)
            throw IException("SVDStates: too many species!");
        // generate +- unique number: this is mostly used for hashing, uniqueness is not strictly required
        s.composition=s.dominant_species_index;
        for (int i=0;i<5;++i)
            if (s.admixed_species_index[i]>-1)
                s.composition = (s.composition << 6) + s.admixed_species_index[i];
    }

    // lookup state in the hash table and return
    if (!mStateLookup.contains(s)) {
        s.Id = mStates.size();
        mStates.push_back(s);
        mCompositionString.push_back(createCompositionString(s));
        mStateLookup[s] = s.Id; // add to hash
    }

    return mStateLookup.value(s); // which is a unique index
}

// 37 values, roughly a circle with 7px diameter
static QVector<QPoint> mid_points =  QVector<QPoint>()
        <<  QPoint(-1,-3) << 	QPoint(0,-3)  << QPoint(1,-3)
        <<  QPoint(-2,-2) << 	QPoint(-1,-2) << QPoint(0,-2) << QPoint(1,-2) << QPoint(2,-2)
        <<  QPoint(-3,-1) << 	QPoint(-2,-1) << QPoint(-1,-1)<< QPoint(0,-1) << QPoint(1,-1) << QPoint(2,-1) << QPoint(3,-1)
        <<  QPoint(-3,0)  << 	QPoint(-2,0)  << QPoint(-1,0) << QPoint(1,0)  << QPoint(2,0)  << QPoint(3,0)
        <<  QPoint(-3,1)  << 	QPoint(-2,1)  << QPoint(-1,1) << QPoint(0,1)  << QPoint(1,1)  << QPoint(2,1)  << QPoint(3,1)
        <<  QPoint(-2,2)  << 	QPoint(-1,2)  << QPoint(0,2)  << QPoint(1,2)  << QPoint(2,2)
        <<  QPoint(-1,3)  << 	QPoint(0,3)   << QPoint(1,3);

static QVector<QPoint> close_points =  QVector<QPoint>() << QPoint(-1,-1) << QPoint(0,-1) << QPoint(1,-1)
                                                         << QPoint(-1,0) << QPoint(1,0)
                                                         << QPoint(-1,1) << QPoint(0,1) << QPoint(1,1);

void SVDStates::evaluateNeighborhood(ResourceUnit *ru)
{
    if (!ru->mSVDState.localComposition) {
        // create vectors on the heap only when really needed
        int nspecies = GlobalSettings::instance()->model()->speciesSet()->activeSpecies().size();
        ru->mSVDState.localComposition = new QVector<float>(nspecies, 0.f);
        ru->mSVDState.midDistanceComposition = new QVector<float>(nspecies, 0.f);
    }

    QVector<float> &local = *ru->mSVDState.localComposition;
    QVector<float> &midrange = *ru->mSVDState.midDistanceComposition;

    // clear the vectors
    local.fill(0.f);
    midrange.fill(0.f);

    // get the center point
    const Grid<ResourceUnit*> &grid = GlobalSettings::instance()->model()->RUgrid();
    QPoint cp= grid.indexAt(ru->boundingBox().center());

    // do the work:
    executeNeighborhood(local, cp, close_points, grid);
    executeNeighborhood(midrange, cp, mid_points, grid);
}

QString SVDStates::stateLabel(int index)
{
    if (index<0 || index>=mStates.size())
        return QStringLiteral("invalid");
    const SVDState &s=state(index);
    QString label;
    if (s.dominant_species_index>=0)
        label=GlobalSettings::instance()->model()->speciesSet()->species(s.dominant_species_index)->id().toUpper() + " ";
    for (int i=0;i<5;++i)
        if (s.admixed_species_index[i]>=0)
            label += GlobalSettings::instance()->model()->speciesSet()->species(s.admixed_species_index[i])->id().toLower() + " ";

    QString hlabel;
    switch (mStructureClassification) {
    case Structure4m:
        if (s.structure<21)
            hlabel = QString("%1m-%2m").arg(s.structure*4).arg(s.structure*4+4);
        else
            hlabel = QString("Irr: %1m-%2m").arg((s.structure-21)*12).arg( (s.structure-21+1)*12);
        break;
    case Structure2m:
        if (s.structure<31)
            hlabel = QString("%1m-%2m").arg(s.structure*2).arg(s.structure*2+2);
        else
            hlabel = QString("Irr: %1m-%2m").arg((s.structure-31)*8).arg( (s.structure-31+1)*8);
        break;

    }
    QString flabel;
    switch (mFunctioningClassification) {
    case Functioning3Classes:
        flabel =  s.function==0 ? QStringLiteral("<2") : (s.function==1 ? QStringLiteral("2-4") : QStringLiteral(">4"));
        break;
    case Functioning5Classes:
        if (s.function == 0) flabel = "<1";
        else if (s.function==4) flabel = ">=4";
        else flabel = QString("%1-%2").arg(s.function).arg(s.function+1);
        break;
    }


    label += QString("%1 (LAI %2)").arg( hlabel ).arg(flabel);
    return label;
}

/// run the neighborhood evaluation; list: points to check, vec: a vector with a slot for each species, grid: the resource unit grid,
/// center_point: the coordinates of the focal resource unit
void SVDStates::executeNeighborhood(QVector<float> &vec, QPoint center_point, QVector<QPoint> &list, const Grid<ResourceUnit *> &grid)
{
    // evaluate the mid range neighborhood
    float n=0.;
    for (QVector<QPoint>::const_iterator i=list.constBegin(); i!=list.constEnd(); ++i) {
        if (grid.isIndexValid(center_point + *i)) {
            ResourceUnit *nb = grid[center_point + *i];
            if (nb) {
                int s = nb->svdStateId();
                if (isStateValid(s)) {
                    mStates[s].neighborhoodAnalysis(vec);
                    ++n;
                }
            }
        }
    }
    if (n>0) {
        for (QVector<float>::iterator i=vec.begin(); i!=vec.end();++i)
            *i/=n;
    }
}

QString SVDStates::createCompositionString(const SVDState &s)
{
    QString label;
    if (s.dominant_species_index>=0)
        label=GlobalSettings::instance()->model()->speciesSet()->species(s.dominant_species_index)->id().toUpper() + " ";
    for (int i=0;i<5;++i)
        if (s.admixed_species_index[i]>=0)
            label += GlobalSettings::instance()->model()->speciesSet()->species(s.admixed_species_index[i])->id().toLower() + " ";
    if (label.isEmpty()) {
        if (s.structure>0)
            return QStringLiteral("mix");
        else
            return QStringLiteral("unforested");
    }
    label.chop(1); // remove the trailing space
    return label;
}

QString SVDState::compositionString() const
{
    if (svd)
        return svd->compositionString(Id);
    else
        return QStringLiteral("invalid");
}

QString SVDState::stateLabel() const
{
    if (svd)
        return svd->stateLabel(Id);
    else
        return QStringLiteral("invalid");
}

float SVDState::neighborhoodAnalysis(QVector<float> &v)
{
    /* apply some rules:
     * (a) only 1 dominant species: 100%
     * (b) 1 dom. and 1 other: 67/33
     * (c) only 1 other: 50
     * (d) two other: 50/50
     * (e) three other: 33/33/33
     * (f) 4 other: 4x 25
     * none: 0
     * */
    float total_weight=1.f;
    if (dominant_species_index>-1) {
        if (admixed_species_index[0]==-1) {
            v[dominant_species_index]+=1.f; // (a)
        } else {
            // max 1 other species: >66% + >20% -> at least 86% -> no other species possible
            v[dominant_species_index]+=0.67f; // (b)
            v[admixed_species_index[0]]+=0.33f;
        }
    } else {
        // no dominant species
        int n_s = 0;
        for (int i=0;i<5;++i)
            if (admixed_species_index[i]>-1) ++n_s;

        float f=0.;
        switch (n_s) {
        case 0: return 0.f; // (f)
        case 1: f=0.5f; break;
        case 2: f=0.5f; break;
        case 3: f=0.33f; total_weight=0.99f; break;
        case 4: f=0.25f; break;

        }

        // apply cases

        for (int i=0;i<n_s;++i)
            v[admixed_species_index[i]]+=f;

    }
    return total_weight;

}
