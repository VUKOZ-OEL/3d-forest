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
#include "dbhdistribution.h"

#include "globalsettings.h"
#include "model.h"
#include "species.h"
#include "resourceunit.h"
#include "helper.h"
#include "mapgrid.h"

// classes: 0..5, 5..10, 10..15, 15..20, ...., 95..100, >100 = 21 classes, + 2 classes for total basal area / volume
int DBHDistribution::mNClasses = 21 + 2;


DBHDistribution::~DBHDistribution()
{
    qDeleteAll(mData);
}

void DBHDistribution::calculateFromLandscape()
{
    // reset
    clear( mData );
    AllTreeIterator at(GlobalSettings::instance()->model());
    while (Tree *t=at.nextLiving())
        addTree( t );

}

void DBHDistribution::calculate()
{
    clear(mData);
    mLandscapeInfo = SStandInfo();
    // add all stands
    QHash<int, QVector<double*> >::const_iterator i = mStands.constBegin();
    while (i != mStands.constEnd()) {
        addDistribution(i.value(), mData);
        mLandscapeInfo.volume += mStandInfo[i.key()].volume;
        mLandscapeInfo.basalarea += mStandInfo[i.key()].basalarea;
        ++i;
    }
    mLandscapeInfo.basalarea /= mTotalArea;
    mLandscapeInfo.volume /= mTotalArea;
}

void DBHDistribution::addStand(const int standId)
{
    // get all trees of the stand
    const MapGrid *mg = GlobalSettings::instance()->model()->standGrid();
    if (!mg) return;
    QVector<QPair<Tree *, double> > trees;
    // load all trees for the stand
    mg->loadTrees(standId, trees);
    double area = mg->area(standId);
    QVector<Tree*> tree_list;
    tree_list.reserve(trees.size());
    for (int i=0;i<trees.size();++i)
        tree_list.push_back( trees[i].first );
    addStand(standId, area, tree_list);
}

void DBHDistribution::addResourceUnit(const ResourceUnit *ru)
{
    foreach(const Tree &t, ru->constTrees()) {
        addTree( &t );
    }
}

void DBHDistribution::addStand(const int standId, const double area, QVector<Tree *> &tree_list)
{
    QVector<double*> &vec = mStands[standId];
    if (vec.size() == 0) {
        int n_species = GlobalSettings::instance()->model()->speciesSet()->count();
        for (int i=0;i<n_species;++i) {
            double *ptr = new double[ nClasses() ];
            for (int j=0;j<nClasses();++j)
                ptr[j] = 0.;
            vec.push_back(ptr);
        }
    } else {
        clear(vec);
    }

    // add all trees of the stand
    SStandInfo info;
    info.standId = standId;
    info.area = area;
    foreach(Tree* t, tree_list) {
        addTree(t, vec);
        info.volume += t->volume();
        info.basalarea += t->basalArea();
        info.stems++;
    }
    mStandInfo[standId] = info; // store data

}

// quick'n'dirty dump to a text file
void DBHDistribution::saveToTextFile(QString filename)
{
    QString path = GlobalSettings::instance()->path(filename);
    // header
    QString line="species";
    QStringList content;
    for (int i=0;i<nClasses();++i)
        line+=QString(";%1").arg(classLabel(i));
    content+=line;
    for (int si=0;si<mData.size();++si) {
        line=mSpeciesIds[si];
        for (int i=0;i<nClasses();++i)
            line+=QString(";%1").arg(mData[si][i]);
        content+=line;
    }
    Helper::saveToTextFile(path, content.join(QChar('\n')));
    qDebug() << "saved dbh distribution to text file" << path;
}

void DBHDistribution::saveStandInfo(QString filename)
{
    QString path = GlobalSettings::instance()->path(filename);
    QStringList content;
    content << "standId;area;basalArea;volume;stems";
    QHash<int, SStandInfo >::const_iterator i = mStandInfo.constBegin();
    while(i != mStandInfo.constEnd()) {
        content << QString("%1;%2;%3;%4;%5").arg(i.value().standId)
                   .arg(i.value().area)
                   .arg(i.value().basalarea)
                   .arg(i.value().volume)
                   .arg(i.value().stems);
        ++i;
    }
    Helper::saveToTextFile(path, content.join(QChar('\n')));
    qDebug() << "saved stand information to text file" << path;
}

DBHDistribution::DBHDistribution(QObject *)
{
    mTotalArea = GlobalSettings::instance()->model()->totalStockableArea(); // is in ha
}

inline int DBHDistribution::classIndex(const float dbh) const
{
    return dbh>=100.f ? 20 : floor(dbh/5.f);
}

QString DBHDistribution::classLabel(const int class_index)
{

    if (class_index==0) return QString("<5");
    if (class_index==nClasses()-1) return QString("volume");
    if (class_index==nClasses()-2) return QString("basalArea");
    if (class_index==nClasses()-3) return QString(">=100");

    if (class_index>0 && class_index< nClasses()-2)
        return QString(">=%1 <%2").arg(class_index*5).arg((class_index+1)*5);
    return QString("out_of_bound");
}

void DBHDistribution::internalSetup()
{
    int n_species = GlobalSettings::instance()->model()->speciesSet()->count();
    qDeleteAll(mData);
    mData.clear();
    mSpeciesIds.clear();
    mLandscapeInfo = SStandInfo(); // reset
    for (int i=0;i<n_species;++i) {
        double *ptr = new double[ nClasses() ];
        for (int j=0;j<nClasses();++j)
            ptr[j] = 0.;

        mData.push_back(ptr);
        mSpeciesIds.push_back(GlobalSettings::instance()->model()->speciesSet()->species(i)->id());
    }
}

void DBHDistribution::addTree(const Tree *t, QVector<double *> &distribution)
{
     distribution[ t->species()->index() ][classIndex(t->dbh())] += 1. / mTotalArea;
     distribution[ t->species()->index() ][nClasses()-2] += t->basalArea() / mTotalArea;
     distribution[ t->species()->index() ][nClasses()-1] += t->volume() / mTotalArea;
}

void DBHDistribution::clear(QVector<double *> &distribution)
{
    if (mData.size() == 0)
        internalSetup();
    for (int is=0; is<distribution.size();++is)
        for (double *p=distribution[is], i=0; i<nClasses(); ++i)
            *p++ = 0.;
}

void DBHDistribution::addDistribution(const QVector<double *> &source, QVector<double *> &target)
{
    if (source.size() == 0 || target.size() == 0 || source.size() != target.size())
        return;
    for (int si=0;si<source.size();++si)
        for (int i=0;i<nClasses();++i)
            target[si][i] += source[si][i];
}
