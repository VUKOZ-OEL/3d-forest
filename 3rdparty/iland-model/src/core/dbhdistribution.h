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
#include <QObject>
#include <QString>
#include <QHash>
#include <QVector>
#include <QJSEngine>
#ifndef DBHDISTRIBUTION_H
#define DBHDISTRIBUTION_H

class ResourceUnit; // forward
class Tree; // forward

/** The class DBHDistribution encapsulates a DBH distribution on the landscape level. It
 * provides functions to add/remove parts of the landscape (e.g., stands or resource units). */
class DBHDistribution: public QObject
{
    Q_OBJECT
public slots:
    /// renew the full DBH distribution over the full landscape (by iterating over all trees)
    void calculateFromLandscape();
    /// calculate the total DBH distribution on the landscape (by summing up over stand wise DBH distributions)
    void calculate();
    /// add/replace the DBH distribution for a stand denoted by `standId`. Use all trees of the stand.
    void addStand(const int standId);

    void saveToTextFile(QString filename);
    void saveStandInfo(QString filename);

public:
    Q_INVOKABLE DBHDistribution(QObject *parent=0);
    ~DBHDistribution();
    static void addToScriptEngine(QJSEngine &engine);



    /// add the trees of the resource unit `ru`.
    void addResourceUnit(const ResourceUnit *ru);
    /// add/replace the DBH distribution for a stand denoted by `standId`. Use the trees
    /// in `tree_list`.
    void addStand(const int standId, const double area, QVector<Tree *> &tree_list);

    /// get trees/ha of species `speciesID` in class in which `dbh` falls into
    double dbhClass(const QString &speciesId, const float dbh) const;
    /// get trees/ha of species `speciesID` in class `classIndex`(0..nClasses()-1)
    double dbhClass(const QString &speciesId, const int class_index) const;
    /// return the number of DBH classes
    int nClasses() const { return mNClasses; }
    /// return the index of the class where `dbh` falls into (0..nClasses()-1)
    inline int classIndex(const float dbh) const;
    /// return the label of the class `class_index`. The label caption
    /// is human readable and valid for database column names (eg. includes no spaces)
    QString classLabel(const int class_index);

    // getters and setters
    void setTotalArea(const double total_area_ha) { mTotalArea = total_area_ha; }
    double totalArea() const { return mTotalArea; }
private:
    struct SStandInfo {
        SStandInfo(): standId(0), area(0.), volume(0.), basalarea(0.), stems(0) {}
        int standId;
        double area;
        double volume;
        double basalarea;
        int stems;
    };

    double mTotalArea; ///< landscape area in ha, used to scale tree density
    QVector< double* > mData; ///< the internal data structure; row [indexed by speciesIndex] points to an integer array
    QVector< QString > mSpeciesIds;
    SStandInfo mLandscapeInfo;

    QHash<int, QVector<double*> > mStands;
    QHash<int, SStandInfo > mStandInfo;

    void internalSetup();
    void addTree(const Tree *t) {addTree(t, mData);} ///< add to main DBH distribution
    void addTree(const Tree *t, QVector< double* > &distribution); ///< add to a specific dbh distribution
    void clear(QVector< double* > &distribution); ///< clear all data from `distribution`
    /// add dbh distribution `source` to `target`
    void addDistribution(const QVector<double *> &source, QVector<double*> &target);
    static int mNClasses;
};

#endif // DBHDISTRIBUTION_H
