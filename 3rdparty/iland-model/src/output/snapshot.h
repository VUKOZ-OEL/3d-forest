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
#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QString>
#include <QHash>
#include <QSqlQuery>
/** @class Snapshot provides a way to save/load the current state of the model to a database.
 *  A snapshot contains trees, saplings, snags and soil (carbon/nitrogen pools), i.e. a
 *   snapshot allows to replicate all state variables of a landscape system.
  */
class ResourceUnit; // forward
class MapGrid; // forward
class Snag; // forward
class Soil; // forward

class Snapshot
{
public:
    Snapshot();
    bool createSnapshot(const QString &file_name);
    bool loadSnapshot(const QString &file_name);
    // snapshot functions for single stands
    /// save the trees/saplings from a single stand (given by 'stand_id' and 'stand_grid' to a database in 'file_name'
    bool saveStandSnapshot(const int stand_id, const MapGrid *stand_grid, const QString &file_name);
    /// load the trees/saplings from a single stand (given by 'stand_id' and 'stand_grid' from a database in 'file_name'
    bool loadStandSnapshot(const int stand_id, const MapGrid *stand_grid, const QString &file_name);
    /// save the carbon/snag pools of a set of resource units
    bool saveStandCarbon(const int stand_id, QList<int> ru_ids, bool rid_mode);
    /// load the carbon/snags pools from the current (stand) snapshot
    bool loadStandCarbon();
private:
    bool openDatabase(const QString &file_name, const bool read);
    // analyze which columns are in the snapshot db
    void checkContent(QString dbname);
    bool openStandDatabase(const QString &file_name, bool read);
    void saveTrees();
    void saveSoil();
    void saveSoilRU(QList<int> stand_ids, bool ridmode);
    void saveSoilCore(ResourceUnit *ru, Soil *s, QSqlQuery &q);
    void saveSnags();
    void saveSnagRU(QList<int> stand_ids, bool ridmode);
    void saveSnagCore(Snag *s, QSqlQuery &q);
    void saveSaplings();
    void saveDeadTrees();
    void loadTrees();
    void loadSoil(QSqlDatabase db=QSqlDatabase());
    void loadSnags(QSqlDatabase db=QSqlDatabase());
    void loadSaplings();
    void loadDeadTrees();
    void loadSaplingsOld();
    QHash<int, ResourceUnit* > mRUHash;
    struct sContent {
        sContent(): permafrost(false),deadtrees(false) {}
        bool permafrost;
        bool deadtrees;
    } dbcontent;
};

#endif // SNAPSHOT_H
