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
#include "snapshot.h"
#include "global.h"
#include "globalsettings.h"
#include "model.h"
#include "resourceunit.h"
#include "species.h"
#include "tree.h"
#include "soil.h"
#include "snag.h"
#include "saplings.h"
#include "debugtimer.h"
#include "watercycle.h"
#include "permafrost.h"
#include "expressionwrapper.h"
#include "helper.h"
#include "gisgrid.h"
#include "mapgrid.h"

#include <QString>
#include <QtSql>
#include <QDataStream>
//#include <QVector3D>


class SnapshotItem {
public:
    void setTree(const Tree *tree, QPointF &offset)
    {
        id = tree->id();
        x = tree->position().x() + offset.x();
        y = tree->position().y() + offset.y();
        species = tree->species()->id();
        age = tree->age();
        height = tree->height();
        dbh = tree->dbh();
        leaf_area = tree->leafArea();
        opacity = tree->mOpacity;
        bmFoliage = tree->biomassFoliage();
        bmStem = tree->biomassStem() - tree->biomassReserve();
        bmFineRoot = tree->biomassFineRoot();
        bmCoarseRoot = tree->biomassCoarseRoot();
        npp_reserve = tree->biomassReserve();
        stress_index = tree->mStressIndex;

    }
    void setSapling(const SaplingTree &sap, QPointF &location) {
        x = location.x();
        y = location.y();
        species_index = sap.species_index;
        sap_age = sap.age;
        height = sap.height;
        sap_stress = sap.stress_years;
        sap_flags = sap.flags;
    }

    void insertTreeToDataStream( QDataStream& dataStream ) const
    {
        dataStream << id << x << y << species << age << height << dbh;
        dataStream << leaf_area << opacity << bmFoliage << bmStem << bmFineRoot << bmCoarseRoot;
        dataStream << npp_reserve << stress_index;
    }
    void extractTreeFromDataStream( QDataStream& dataStream ) {
        dataStream >> id >> x >> y >> species >> age >> height >> dbh;
        dataStream >> leaf_area >> opacity >> bmFoliage >> bmStem >> bmFineRoot >> bmCoarseRoot;
        dataStream >> npp_reserve >> stress_index;
    }
    void insertSaplingToDataStream( QDataStream& dataStream ) const
    {
        dataStream << x << y << species_index << sap_age << height << sap_stress << sap_flags;
    }
    void extractSaplingFromDataStream( QDataStream& dataStream ) {
        dataStream >> x >> y >> species_index >> sap_age >> height >> sap_stress >> sap_flags;
    }

// variables
int id;
double x,y;
QString species;
int age;
float height, dbh;
float leaf_area;
float opacity;
float bmFoliage, bmStem, bmFineRoot, bmCoarseRoot;
float npp_reserve, stress_index;
// for saplings
int species_index;
unsigned short sap_age;
unsigned char sap_stress, sap_flags;
};


Snapshot::Snapshot()
{
}

bool Snapshot::openDatabase(const QString &file_name, const bool read)
{
    if (!GlobalSettings::instance()->setupDatabaseConnection("snapshot", file_name, read)) {
        throw IException("Snapshot:createDatabase: database could not be created / opened");
    }
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    if (!read) {
        // create tables
        QSqlQuery q(db);
        // trees
        q.exec("drop table trees");
        q.exec("create table trees (ID integer, RUindex integer, posX integer, posY integer, species text,  age integer, height real, dbh real, leafArea real, opacity real, foliageMass real, woodyMass real, fineRootMass real, coarseRootMass real, NPPReserve real, stressIndex real)");
        // soil
        q.exec("drop table soil");
        q.exec("create table soil (RUindex integer, kyl real, kyr real, inLabC real, inLabN real, inLabP real, inRefC real, inRefN real, inRefP real, " \
                "YLC real, YLN real, YLAGFrac real, YLP real, YRC real, YRN real, YRAGFrac real, YRP real, SOMC real, SOMN real, " \
               "WaterContent, SnowPack, MossBiomass real, DeepSoilTemp real, pfDepthFrozen real, pfWaterFrozen real real)");
        // snag
        q.exec("drop table snag");
        q.exec("create table snag(RUIndex integer, climateFactor real, SWD1C real, SWD1N real, SWD2C real, SWD2N real, SWD3C real, SWD3N real, " \
               "totalSWDC real, totalSWDN real, NSnags1 real, NSnags2 real, NSnags3 real, dbh1 real, dbh2 real, dbh3 real, height1 real, height2 real, height3 real, " \
               "volume1 real, volume2 real, volume3 real, tsd1 real, tsd2 real, tsd3 real, ksw1 real, ksw2 real, ksw3 real, halflife1 real, halflife2 real, halflife3 real, " \
               "branch1C real, branch1N real, branch2C real, branch2N real, branch3C real, branch3N real, branch4C real, branch4N real, branch5C real, branch5N real, branchIndex integer, branchAGFraction real)");
        // saplings/regeneration
        q.exec("drop table saplings");
        q.exec("create table saplings (RUindex integer, posx integer, posy integer, species_index integer, age integer, height float, stress_years integer, flags integer)");
        qDebug() << "Snapshot - tables created. Database" << file_name;
    }

    checkContent("snapshot");
    return true;
}

void Snapshot::checkContent(QString dbname)
{
    QSqlDatabase db=QSqlDatabase::database(dbname);
    QSqlRecord r = db.record("soil");
    dbcontent.permafrost = r.indexOf("MossBiomass")>=0; // permafrost columns included
    qDebug() << "Snapshot content: permafrost: " << dbcontent.permafrost;
}

bool Snapshot::openStandDatabase(const QString &file_name, bool read)
{
    if (!GlobalSettings::instance()->setupDatabaseConnection("snapshotstand", file_name, read)) {
        throw IException("Snapshot:createDatabase: database could not be created / opened");
    }
    checkContent("snapshotstand");
    return true;

}

bool Snapshot::createSnapshot(const QString &file_name)
{
    openDatabase(file_name, false);
    // save the trees
    saveTrees();
    // save soil pools
    saveSoil();
    // save snags / deadwood pools
    saveSnags();
    // save saplings
    saveSaplings();
    QSqlDatabase::database("snapshot").close();
    // save a grid of the indices
    QFileInfo fi(file_name);
    QString grid_file = fi.absolutePath() + "/" + fi.completeBaseName() + ".asc";
    Grid<double> index_grid;
    index_grid.setup( GlobalSettings::instance()->model()->RUgrid().metricRect(), GlobalSettings::instance()->model()->RUgrid().cellsize());
    RUWrapper ru_wrap;
    Expression ru_value("index", &ru_wrap);
    double *grid_ptr = index_grid.begin();
    for (ResourceUnit **ru = GlobalSettings::instance()->model()->RUgrid().begin(); ru!=GlobalSettings::instance()->model()->RUgrid().end(); ++ru, ++grid_ptr) {
        if (*ru) {
            ru_wrap.setResourceUnit(*ru);
            *grid_ptr = ru_value.execute();
        } else
            *grid_ptr = -1.;
    }
    QString grid_text = gridToESRIRaster(index_grid);
    Helper::saveToTextFile(grid_file, grid_text);
    qDebug() << "saved grid to " << grid_file;

    return true;
}

bool Snapshot::loadSnapshot(const QString &file_name)
{
    DebugTimer t("loadSnapshot");
    openDatabase(file_name, true);


    QFileInfo fi(file_name);
    QString grid_file = fi.absolutePath() + "/" + fi.completeBaseName() + ".asc";
    GisGrid grid;
    mRUHash.clear();

    if (!grid.loadFromFile(grid_file)) {
        qDebug() << "loading of snapshot: not a valid grid file (containing resource unit inidices) expected at:" << grid_file;
        qDebug() << "assuming the same size of the project area as for the saved snapshot.";
        for (ResourceUnit **ru = GlobalSettings::instance()->model()->RUgrid().begin(); ru!=GlobalSettings::instance()->model()->RUgrid().end();++ru) {
            if (*ru)
                mRUHash[ (*ru)->index() ] = *ru;
        }
    } else {
        // setup link between resource unit index and index grid:
        // store for each resource unit *in the snapshot database* the corresponding
        // resource unit index of the *current* simulation.
        QPointF to=GisGrid::worldToModel(grid.origin());

        if (fmod(to.x(), cRUSize) != 0. || fmod(to.y(), cRUSize) != 0.) {
            QPointF world_offset = GisGrid::modelToWorld(QPointF(0., 0.));
            throw IException(QString("Loading of the snapshot '%1' failed: The offset from the current location of the project (%4/%5) " \
                                     "is not a multiple of the resource unit size (100m) relative to grid of the snapshot (origin-x: %2, origin-y: %3).").arg(file_name)
                             .arg(grid.origin().x()).arg(grid.origin().y()).arg(world_offset.x()).arg(world_offset.y()));
        }


        const Grid<ResourceUnit*> &rugrid = GlobalSettings::instance()->model()->RUgrid();
        for (int i=0;i<rugrid.count();++i) {
            const ResourceUnit *ru = rugrid.constValueAtIndex(i);
            if (ru && ru->index()>-1) {
               int value = grid.value( rugrid.cellCenterPoint(i) );
               if (value>-1)
                   mRUHash[value] = const_cast<ResourceUnit*>(ru);
            }
        }

    }


    loadTrees();
    loadSoil();
    loadSnags();
    // load saplings only when regeneration is enabled (this can save a lot of time)
    if (GlobalSettings::instance()->model()->settings().regenerationEnabled) {
        loadSaplings();
        //loadSaplingsOld();
    }
    QSqlDatabase::database("snapshot").close();

    // after changing the trees, do a complete apply/read pattern cycle over the landscape...
    GlobalSettings::instance()->model()->onlyApplyLightPattern();
    qDebug() << "applied light pattern...";

    // refresh the stand statistics
    foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
         ru->recreateStandStatistics(true); // true: recalculate statistics
     }

    qDebug() << "created stand statistics...";
    qDebug() << "loading of snapshot completed.";

    return true;
}

bool Snapshot::saveStandSnapshot(const int stand_id, const MapGrid *stand_grid, const QString &file_name)
{
    DebugTimer t("saveStandSnapshot");
    // Check database
    QSqlDatabase db=QSqlDatabase::database("snapshotstand");
    if (!db.isOpen()) {
        openStandDatabase(GlobalSettings::instance()->path(file_name), false);
        db=QSqlDatabase::database("snapshotstand");
        // check if tree/sapling tables are already present
        if (!db.tables().contains("trees_stand") || !db.tables().contains("saplings_stand")) {
            // create tables
            QSqlQuery q(db);
            // trees
            q.exec("drop table trees_stand");
            //q.exec("create table trees_stand (standID integer, ID integer, posX integer, posY integer, species text,  age integer, height real, dbh real, leafArea real, opacity real, foliageMass real, woodyMass real, fineRootMass real, coarseRootMass real, NPPReserve real, stressIndex real)");
            //q.exec("create index on trees_stand (standID)");
            q.exec("create table trees_stand (standID integer, trees BLOB)");
            // saplings/regeneration
            q.exec("drop table saplings_stand");
            //q.exec("create table saplings_stand (standID integer, posx integer, posy integer, species_index integer, age integer, height float, stress_years integer, flags integer)");
            //q.exec("create index on saplings_stand (standID)");
            q.exec("create table saplings_stand (standID integer, saplings BLOB)");
            // soil
            // * add a primary key for RUindex, and then use INSERT OR REPLACE statements
            // * to overwrite the rows on subsequent saves.
            q.exec("drop table soil");
            q.exec("create table soil (RUindex integer primary key, kyl real, kyr real, inLabC real, inLabN real, inLabP real, inRefC real, inRefN real, inRefP real, " \
                                      "YLC real, YLN real, YLAGFrac real, YLP real, YRC real, YRN real, YRAGFrac real, YRP real, SOMC real, SOMN real, WaterContent, " \
                                      "SnowPack real, MossBiomass real, DeepSoilTemp real, pfDepthFrozen real, pfWaterFrozen real)");
            // snag
            q.exec("drop table snag");
            q.exec("create table snag(RUIndex integer primary key, climateFactor real, SWD1C real, SWD1N real, SWD2C real, SWD2N real, SWD3C real, SWD3N real, " \
                   "totalSWDC real, totalSWDN real, NSnags1 real, NSnags2 real, NSnags3 real, dbh1 real, dbh2 real, dbh3 real, height1 real, height2 real, height3 real, " \
                   "volume1 real, volume2 real, volume3 real, tsd1 real, tsd2 real, tsd3 real, ksw1 real, ksw2 real, ksw3 real, halflife1 real, halflife2 real, halflife3 real, " \
                   "branch1C real, branch1N real, branch2C real, branch2N real, branch3C real, branch3N real, branch4C real, branch4N real, branch5C real, branch5N real, branchIndex integer, branchAGFraction real)");


        }
    }
    // do nothing for negative standIds
    if (stand_id<0)
        return true;

    // save trees
    QSqlQuery q(db);
    q.exec(QString("delete from trees_stand where standID=%1").arg(stand_id));

    if (!q.prepare(QString("insert into trees_stand (standID, trees) " \
                           "values (:standid, :blob)")))
        throw IException(QString("Snapshot::saveTrees: prepare:") + q.lastError().text());

    db.transaction();
    q.addBindValue(stand_id);

    SnapshotItem tsn;
    QByteArray tree_container;
    QDataStream tree_writer(&tree_container, QIODevice::WriteOnly);
    tree_writer << (quint32)0xFFEEEEDD; // good old feed... the magic phrase

    QPointF offset = GisGrid::modelToWorld(QPointF(0.,0.));
    QList<Tree*> tree_list = stand_grid->trees(stand_id);
    QList<Tree*>::const_iterator it;
    for (it = tree_list.constBegin(); it!= tree_list.constEnd(); ++it) {
        Tree *t = *it;
        tsn.setTree(t, offset);
        tsn.insertTreeToDataStream(tree_writer); // stream tree information to the buffer
    }

    // store all trees as a blob
    q.addBindValue(tree_container);

    if (!q.exec()) {
        throw IException(QString("Snapshot::saveStandSnapshot, Trees: execute:") + q.lastError().text());
    }

    // save saplings
    // loop over all pixels, only when regeneration is enabled
    if (GlobalSettings::instance()->model()->settings().regenerationEnabled) {
        q.exec(QString("delete from saplings_stand where standID=%1").arg(stand_id));

        if (!q.prepare(QString("insert into saplings_stand (standID, saplings) " \
                               "values (?,?)")))
            throw IException(QString("Snapshot::saveSaplings: prepare:") + q.lastError().text());

        QByteArray sap_container;
        QDataStream sap_writer(&sap_container, QIODevice::WriteOnly);
        sap_writer << (quint32)0xFFEEEEDD; // good old feed... the magic phrase

        q.addBindValue(stand_id);

        SaplingCellRunner scr(stand_id, stand_grid);
        while (SaplingCell *sc = scr.next()) {
            for (int i=0;i<NSAPCELLS;++i)
                if (sc->saplings[i].is_occupied()) {
                    QPointF t = scr.currentCoord() + offset;
                    tsn.setSapling(sc->saplings[i], t);
                    tsn.insertSaplingToDataStream(sap_writer);
                }
        }
        q.addBindValue(sap_container);
        if (!q.exec()) {
            throw IException(QString("Snapshot::saveStandSnapshot, saplings: execute:") + q.lastError().text());
        }
    }

    db.commit();

    return true;
}

bool Snapshot::loadStandSnapshot(const int stand_id, const MapGrid *stand_grid, const QString &file_name)
{
    QSqlDatabase db=QSqlDatabase::database("snapshotstand");
    if (!db.isOpen()) {
        openStandDatabase(GlobalSettings::instance()->path(file_name), true);
        db=QSqlDatabase::database("snapshotstand");
    }
    // load trees
    // kill all living trees on the stand
    QList<Tree*> tree_list = stand_grid->trees(stand_id);
    QList<Tree*>::const_iterator it;
    int n_removed = tree_list.count();
    for (it = tree_list.constBegin(); it!= tree_list.constEnd(); ++it) {
        (*it)->remove(1., 1., 1.);
    }

    // load from database
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.exec(QString("select trees "
                   "from trees_stand where standID=%1").arg(stand_id));
    QRectF extent = GlobalSettings::instance()->model()->extent();
    int n=0, sap_n=0, n_sap_removed=0;
    if (q.next()) {
        QByteArray data = q.value(0).toByteArray();
        QDataStream st(data);
        quint32 magic;
        st >> magic;
        if (magic!=0xFFEEEEDD)
            throw IException(QString("loadStandSnapshot: invalid data for trees of stand").arg(stand_id));
        SnapshotItem item;
        while (!st.atEnd()) {
           // load
            item.extractTreeFromDataStream(st);

           QPointF coord(GisGrid::worldToModel(QPointF(item.x, item.y)));
           if (!extent.contains(coord))
               continue;
           ResourceUnit *ru = GlobalSettings::instance()->model()->ru(coord);
           if (!ru)
               continue;
           Tree &t = ru->newTree();
           t.setRU(ru);
           t.mId = item.id;
           t.setPosition(coord);
           Species *s = GlobalSettings::instance()->model()->speciesSet()->species(item.species);
           if (!s)
               throw IException("Snapshot::loadTrees: Invalid species");
           t.setSpecies(s);
           t.mAge = item.age;
           t.mHeight = item.height;
           t.mDbh = item.dbh;
           t.mLeafArea = item.leaf_area;
           t.mOpacity = item.opacity;
           t.mFoliageMass = item.bmFoliage;
           t.mStemMass = item.bmStem;
           t.mFineRootMass = item.bmFineRoot;
           t.mCoarseRootMass = item.bmCoarseRoot;
           t.mNPPReserve = item.npp_reserve;
           t.mStressIndex = item.stress_index;
           t.mStamp = s->stamp(t.mDbh, t.mHeight);
           n++;

        }
    }

    // now the saplings
    if (GlobalSettings::instance()->model()->settings().regenerationEnabled) {
        // (1) remove all saplings:
        SaplingCellRunner scr(stand_id, stand_grid);
        while (SaplingCell *sc = scr.next()) {
            n_sap_removed += sc->n_occupied();
            GlobalSettings::instance()->model()->saplings()->clearSaplings(sc, scr.ru(),true, false);
        }

        // (2) load saplings from database
        q.exec(QString("select saplings "
                       "from saplings_stand where standID=%1").arg(stand_id));
        if (q.next()) {
            QByteArray data = q.value(0).toByteArray();
            QDataStream st(data);
            quint32 magic;
            st >> magic;
            if (magic!=0xFFEEEEDD)
                throw IException(QString("loadStandSnapshot: invalid data for saplings of stand").arg(stand_id));
            SnapshotItem item;
            while (!st.atEnd()) {
                item.extractSaplingFromDataStream(st);

                QPointF coord(GisGrid::worldToModel(QPointF(item.x, item.y)));
                if (!extent.contains(coord))
                    continue;
                SaplingCell *sc = GlobalSettings::instance()->model()->saplings()->cell(GlobalSettings::instance()->model()->grid()->indexAt(coord));
                if (!sc)
                    continue;
                if (SaplingTree *st = sc->addSapling(item.height,
                                                     item.sap_age,
                                                     item.species_index)) {
                    st->stress_years = item.sap_stress;
                    st->flags = item.sap_flags;
                }
                sap_n++;
            }
        }
    }

    // clean up
    GlobalSettings::instance()->model()->cleanTreeLists(true);

    qDebug() << "load stand snapshot for stand "<< stand_id << ": trees (removed/loaded): " <<n_removed<<"/" << n <<", saplings (removed/loaded):" << n_sap_removed << "/" << sap_n;

    return true;
}

bool Snapshot::saveStandCarbon(const int stand_id,  QList<int> ru_ids, bool rid_mode)
{
    QSqlDatabase db=QSqlDatabase::database("snapshotstand");
    if (!db.isOpen()) {
        throw IException("Snapshot::saveStandCarbon: stand snapshot data base is not open. Please use 'saveStandSnapshot' to set up the data base connection.");
    }
    qDebug() << "Trying to save snags and soil pools for" << ru_ids.size() << "resource units. stand_id:" << stand_id << "using:" << (rid_mode ? "RID":"ruindex");
    saveSoilRU(ru_ids, rid_mode);
    saveSnagRU(ru_ids, rid_mode);
    return true;
}

bool Snapshot::loadStandCarbon()
{
    QSqlDatabase db=QSqlDatabase::database("snapshotstand");
    if (!db.isOpen()) {
        throw IException("Snapshot::loadStandCarbon: stand snapshot data base is not open. Please use 'saveStandSnapshot' to set up the data base connection.");
    }
    qDebug() << "loading snags/carbon pools from the stand snapshot...";

    mRUHash.clear();
    for (ResourceUnit **ru = GlobalSettings::instance()->model()->RUgrid().begin(); ru!=GlobalSettings::instance()->model()->RUgrid().end();++ru) {
        if (*ru)
            mRUHash[ (*ru)->index() ] = *ru;
    }

    // now load soil carbon and snags from the standsnapshot databse
    loadSoil(db);
    loadSnags(db);
    qDebug() << "finished loading stand carbon...";
    return true;

}

void Snapshot::saveTrees()
{
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    AllTreeIterator at(GlobalSettings::instance()->model());
    QSqlQuery q(db);
    if (!q.prepare(QString("insert into trees (ID, RUindex, posX, posY, species,  age, height, dbh, leafArea, opacity, foliageMass, woodyMass, fineRootMass, coarseRootMass, NPPReserve, stressIndex) " \
                      "values (:id, :index, :x, :y, :spec, :age, :h, :d, :la, :opa, :mfol, :mwood, :mfr, :mcr, :npp, :si)")))
        throw IException(QString("Snapshot::saveTrees: prepare:") + q.lastError().text());

    int n = 0;
    db.transaction();
    while (Tree *t = at.next()) {
        // loop over all trees; the AllTreeIterator returns all trees of the first RU, then all of the next, etc.
        q.addBindValue(t->id());
        q.addBindValue(t->ru()->index());
        q.addBindValue(t->mPositionIndex.x());
        q.addBindValue(t->mPositionIndex.y());
        q.addBindValue(t->species()->id());
        q.addBindValue(t->age());
        q.addBindValue(t->height());
        q.addBindValue(t->dbh());
        q.addBindValue(t->leafArea());
        q.addBindValue(t->mOpacity);
        q.addBindValue(t->biomassFoliage());
        q.addBindValue(t->biomassStem()-t->biomassReserve());
        q.addBindValue(t->biomassFineRoot());
        q.addBindValue(t->biomassCoarseRoot());
        q.addBindValue(t->mNPPReserve);
        q.addBindValue(t->mStressIndex);
        if (!q.exec()) {
            throw IException(QString("Snapshot::saveTrees: execute:") + q.lastError().text());
        }
        if (++n % 10000 == 0) {
            qDebug() << n << "trees saved...";
            QCoreApplication::processEvents();
        }
    }
    db.commit();
    qDebug() << "Snapshot: finished trees. N=" << n;
}

void Snapshot::loadTrees()
{
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    QSqlQuery q(db);
    // setForwardOnly() -> helps avoiding that the query caches all the data
    // during iterating
    q.setForwardOnly(true);
    q.exec("select ID, RUindex, posX, posY, species,  age, height, dbh, leafArea, opacity, foliageMass, woodyMass, fineRootMass, coarseRootMass, NPPReserve, stressIndex from trees");
    int ru_index = -1;
    int new_ru;
    int offsetx=0, offsety=0;
    ResourceUnit *ru = 0;
    HeightGrid *hg = GlobalSettings::instance()->model()->heightGrid();
    FloatGrid *lif_grid = GlobalSettings::instance()->model()->grid();

    int n=0, ntotal=0;
    try {
        // clear all trees on the landscape
        foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList())
            ru->trees().clear();
        // load the trees from the database
        while (q.next()) {
            new_ru = q.value(1).toInt();
            ++ntotal;
            if (new_ru != ru_index) {
                ru_index = new_ru;
                ru = mRUHash[ru_index];
                if (ru) {
                    offsetx = ru->cornerPointOffset().x();
                    offsety = ru->cornerPointOffset().y();
                }

            }
            if (!ru)
                continue;
            QPoint tree_idx(offsetx + q.value(2).toInt() % cPxPerRU, offsety + q.value(3).toInt() % cPxPerRU);
            // check if pixel is valid in the height grid
            if (!hg->valueAtIndex(lif_grid->index5(lif_grid->index(tree_idx))).isValid())
                continue;

            // add a new tree to the tree list
            Tree &t = ru->newTree();
            t.setRU(ru);
            t.mId = q.value(0).toInt();
            t.mPositionIndex = tree_idx;
            //t.mPositionIndex.setX(offsetx + q.value(2).toInt() % cPxPerRU);
            //t.mPositionIndex.setY(offsety + q.value(3).toInt() % cPxPerRU);
            Species *s = GlobalSettings::instance()->model()->speciesSet()->species(q.value(4).toString());
            if (!s)
                throw IException("Snapshot::loadTrees: Invalid species");
            t.setSpecies(s);
            t.mAge = q.value(5).toInt();
            t.mHeight = q.value(6).toFloat();
            t.mDbh = q.value(7).toFloat();
            t.mLeafArea = q.value(8).toFloat();
            t.mOpacity = q.value(9).toFloat();
            t.mFoliageMass = q.value(10).toFloat();
            t.mStemMass = q.value(11).toFloat();
            t.mBranchMass = static_cast<float>(s->biomassBranch(t.dbh())); // TODO: include branch also in snapshot
            t.mFineRootMass = q.value(12).toFloat();
            t.mCoarseRootMass = q.value(13).toFloat();
            t.mNPPReserve = q.value(14).toFloat();
            t.mStressIndex = q.value(15).toFloat();
            t.mStamp = s->stamp(t.mDbh, t.mHeight);

            ++n;
            if (n % 10000 == 0 )
                if ( (n<100000 ) ||   // until 100,000 every 10k
                     (n<1000000 && n % 100000 == 0) ||   // until 1M every 100k
                     ( n % 1000000 == 0) ) {             // then every M
                    qDebug() << n << "trees loaded...";
                    QCoreApplication::processEvents();
                }

        }
    } catch (const std::bad_alloc &) {
        throw IException(QString("bad_alloc exception after %1 trees!!!!").arg(n));
    }


    qDebug() << "Snapshot: finished trees. N=" << n << "from trees in snapshot:" << ntotal;
}


void Snapshot::saveSoil()
{
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    QSqlQuery q(db);
    if (!q.prepare(QString("insert into soil (RUindex, kyl, kyr, inLabC, inLabN, inLabP, inRefC, inRefN, inRefP, YLC, YLN, YLAGFrac, YLP, YRC, YRN, YRAGFrac, YRP, SOMC, SOMN, WaterContent, SnowPack, MossBiomass, DeepSoilTemp, pfDepthFrozen, pfWaterFrozen) " \
                      "values (:idx, :kyl, :kyr, :inLabC, :iLN, :iLP, :iRC, :iRN, :iRP, :ylc, :yln, :ylag, :ylp, :yrc, :yrn, :yrag, :yrp, :somc, :somn, :wc, :snowpack, :moss, :pftemp, :pfdepth, :pfwater)")))
        throw IException(QString("Snapshot::saveSoil: prepare:") + q.lastError().text());

    int n = 0;
    db.transaction();
    foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        Soil *s = ru->soil();
        if (s) {

            saveSoilCore(ru, s, q);

            if (++n % 1000 == 0) {
                qDebug() << n << "soil resource units saved...";
                QCoreApplication::processEvents();
            }
        }
    }

    db.commit();
    qDebug() << "Snapshot: finished Soil. N=" << n;
}

void Snapshot::saveSoilRU(QList<int> stand_ids, bool ridmode)
{
    QSqlDatabase db=QSqlDatabase::database("snapshotstand");
    QSqlQuery q(db);
    if (!q.prepare(QString("insert or replace into soil (RUindex, kyl, kyr, inLabC, inLabN, inLabP, inRefC, inRefN, inRefP, YLC, YLN, YLAGFrac, YLP, YRC, YRN, YRAGFrac, YRP, SOMC, SOMN, WaterContent, SnowPack, MossBiomass, DeepSoilTemp, pfDepthFrozen, pfWaterFrozen) " \
                      "values (:idx, :kyl, :kyr, :inLabC, :iLN, :iLP, :iRC, :iRN, :iRP, :ylc, :yln, :ylag, :ylp, :yrc, :yrn, :yrag, :yrp, :somc, :somn, :wc, :snowpack, :moss, :pftemp, :pfdepth, :pfwater)")))
        throw IException(QString("Snapshot::saveSoil: prepare:") + q.lastError().text());

    int n = 0;
    for (int i=0;i<stand_ids.size();++i) {
        // ridmode = true: numbers are Ids, false: numbers are ruindex
        ResourceUnit *ru = ridmode ? GlobalSettings::instance()->model()->ruById(stand_ids[i]) : GlobalSettings::instance()->model()->ru(stand_ids[i]);
        if (ru)
            if (ru->soil()) {
                // save .....
                saveSoilCore(ru, ru->soil(), q);
                ++n;
            }
    }
    qDebug() << "Snapshot: finished Soil. N=" << n;

}

void Snapshot::saveSoilCore(ResourceUnit *ru, Soil *s, QSqlQuery &q)
{
    q.addBindValue(s->mRU->index());
    q.addBindValue(s->mKyl);
    q.addBindValue(s->mKyr);
    q.addBindValue(s->mInputLab.C);
    q.addBindValue(s->mInputLab.N);
    q.addBindValue(s->mInputLab.parameter());
    q.addBindValue(s->mInputRef.C);
    q.addBindValue(s->mInputRef.N);
    q.addBindValue(s->mInputRef.parameter());
    q.addBindValue(s->mYL.C);
    q.addBindValue(s->mYL.N);
    q.addBindValue(s->mYLaboveground_frac);
    q.addBindValue(s->mYL.parameter());
    q.addBindValue(s->mYR.C);
    q.addBindValue(s->mYR.N);
    q.addBindValue(s->mYRaboveground_frac);
    q.addBindValue(s->mYR.parameter());
    q.addBindValue(s->mSOM.C);
    q.addBindValue(s->mSOM.N);
    q.addBindValue(ru->waterCycle()->currentContent());
    q.addBindValue(ru->waterCycle()->currentSnowPack());
    if (ru->waterCycle()->permafrost()) {
         q.addBindValue(ru->waterCycle()->permafrost()->mossBiomass());
         q.addBindValue(ru->waterCycle()->permafrost()->groundBaseTemperature());
         q.addBindValue(ru->waterCycle()->permafrost()->depthFrozen());
         q.addBindValue(ru->waterCycle()->permafrost()->waterFrozen());
    } else {
        q.addBindValue(0); q.addBindValue(0);
        q.addBindValue(0); q.addBindValue(0);
    }

    if (!q.exec()) {
        throw IException(QString("Snapshot::saveSoil: execute:") + q.lastError().text());
    }

}

void Snapshot::loadSoil(QSqlDatabase db)
{
    // if 'db' is not a valid data base, take the 'snapshot' database
    if (!db.isValid())
        db=QSqlDatabase::database("snapshot");

    QSqlQuery q(db);
    q.exec(QString("select RUindex, kyl, kyr, inLabC, inLabN, inLabP, inRefC, inRefN, inRefP, YLC, YLN, YLAGFrac, YLP, YRC, " \
           "YRN, YRAGFrac, YRP, SOMC, SOMN, WaterContent, SnowPack %1 from soil")
           .arg(dbcontent.permafrost ? ", MossBiomass, DeepSoilTemp, pfDepthFrozen, pfWaterFrozen" : ""));
    int ru_index = -1;
    ResourceUnit *ru = 0;
    int n=0;
    while (q.next()) {
        ru_index = q.value(0).toInt();
        ru = mRUHash[ru_index];
        if (!ru)
            continue;
        Soil *s = ru->soil();
        if (!s) {
            throw IException("Snapshot::loadSoil: trying to load soil data but soil module is disabled.");
        }
        s->mKyl = q.value(1).toDouble();
        s->mKyr = q.value(2).toDouble();
        s->mInputLab.C = q.value(3).toDouble();
        s->mInputLab.N = q.value(4).toDouble();
        s->mInputLab.setParameter( q.value(5).toDouble());
        s->mInputRef.C = q.value(6).toDouble();
        s->mInputRef.N = q.value(7).toDouble();
        s->mInputRef.setParameter( q.value(8).toDouble());
        s->mYL.C = q.value(9).toDouble();
        s->mYL.N = q.value(10).toDouble();
        s->mYLaboveground_frac = q.value(11).toDouble();
        s->mYL.setParameter( q.value(12).toDouble());
        s->mYR.C = q.value(13).toDouble();
        s->mYR.N = q.value(14).toDouble();
        s->mYRaboveground_frac = q.value(15).toDouble();
        s->mYR.setParameter( q.value(16).toDouble());
        s->mSOM.C = q.value(17).toDouble();
        s->mSOM.N = q.value(18).toDouble();
        const_cast<WaterCycle*>(ru->waterCycle())->setContent(q.value(19).toDouble(), q.value(20).toDouble());
        if (dbcontent.permafrost && ru->waterCycle()->permafrost()) {
            Water::Permafrost *pf = const_cast<Water::Permafrost*>(ru->waterCycle()->permafrost());
            pf->setFromSnapshot(q.value(21).toDouble(),
                                q.value(22).toDouble(),
                                q.value(23).toDouble(),
                                q.value(24).toDouble());


        }

        if (++n % 1000 == 0) {
            qDebug() << n << "soil units loaded...";
            QCoreApplication::processEvents();
        }
    }
    qDebug() << "Snapshot: finished soil. N=" << n;

}


void Snapshot::saveSnags()
{
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    QSqlQuery q(db);
    if (!q.prepare(QString("insert into snag(RUIndex, climateFactor, SWD1C, SWD1N, SWD2C, SWD2N, SWD3C, SWD3N, " \
                           "totalSWDC, totalSWDN, NSnags1, NSnags2, NSnags3, dbh1, dbh2, dbh3, height1, height2, height3, " \
                           "volume1, volume2, volume3, tsd1, tsd2, tsd3, ksw1, ksw2, ksw3, halflife1, halflife2, halflife3, " \
                           "branch1C, branch1N, branch2C, branch2N, branch3C, branch3N, branch4C, branch4N, branch5C, branch5N, branchIndex, branchAGFraction) " \
                           "values (?,?,?,?,?,?,?,?, " \
                           "?,?,?,?,?,?,?,?,?,?,?," \
                           "?,?,?,?,?,?,?,?,?,?,?,?," \
                           "?,?,?,?,?,?,?,?,?,?,?,?)")))
        throw IException(QString("Snapshot::saveSnag: prepare:") + q.lastError().text());

    int n = 0;
    db.transaction();
    foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        Snag *s = ru->snag();
        if (!s)
            continue;

        saveSnagCore(s, q);
    }
    if (++n % 1000 == 0) {
        qDebug() << n << "snags saved...";
        QCoreApplication::processEvents();

    }

    db.commit();
    qDebug() << "Snapshot: finished Snags. N=" << n;
}

void Snapshot::saveSnagRU(QList<int> stand_ids, bool ridmode)
{
    QSqlDatabase db=QSqlDatabase::database("snapshotstand");
    QSqlQuery q(db);
    if (!q.prepare(QString("insert or replace into snag(RUIndex, climateFactor, SWD1C, SWD1N, SWD2C, SWD2N, SWD3C, SWD3N, " \
                           "totalSWDC, totalSWDN, NSnags1, NSnags2, NSnags3, dbh1, dbh2, dbh3, height1, height2, height3, " \
                           "volume1, volume2, volume3, tsd1, tsd2, tsd3, ksw1, ksw2, ksw3, halflife1, halflife2, halflife3, " \
                           "branch1C, branch1N, branch2C, branch2N, branch3C, branch3N, branch4C, branch4N, branch5C, branch5N, branchIndex, branchAGFraction) " \
                           "values (?,?,?,?,?,?,?,?, " \
                           "?,?,?,?,?,?,?,?,?,?,?," \
                           "?,?,?,?,?,?,?,?,?,?,?,?," \
                           "?,?,?,?,?,?,?,?,?,?,?,?)")))
        throw IException(QString("Snapshot::saveSnag: prepare:") + q.lastError().text());
    int n = 0;
    for (int i=0;i<stand_ids.size(); ++i) {
        // ridmode = true: numbers are Ids, false: numbers are ruindex
        ResourceUnit *ru = ridmode ? GlobalSettings::instance()->model()->ruById(stand_ids[i]) : GlobalSettings::instance()->model()->ru(stand_ids[i]);
        if (ru) {
            Snag *s = ru->snag();
            if (s) {
                saveSnagCore(s, q);
                ++n;
            }
        }
    }
    if (n % 1000 == 0) {
        qDebug() << n << "snags saved...";
        QCoreApplication::processEvents();

    }

    db.commit();
    qDebug() << "Snapshot: finished Snags. N=" << n;

}

void Snapshot::saveSnagCore(Snag *s, QSqlQuery &q)
{
    q.addBindValue(s->mRU->index());
    q.addBindValue(s->mClimateFactor);
    q.addBindValue(s->mSWD[0].C);
    q.addBindValue(s->mSWD[0].N);
    q.addBindValue(s->mSWD[1].C);
    q.addBindValue(s->mSWD[1].N);
    q.addBindValue(s->mSWD[2].C);
    q.addBindValue(s->mSWD[2].N);
    q.addBindValue(s->mTotalSWD.C);
    q.addBindValue(s->mTotalSWD.N);
    q.addBindValue(s->mNumberOfSnags[0]);
    q.addBindValue(s->mNumberOfSnags[1]);
    q.addBindValue(s->mNumberOfSnags[2]);
    q.addBindValue(s->mAvgDbh[0]);
    q.addBindValue(s->mAvgDbh[1]);
    q.addBindValue(s->mAvgDbh[2]);
    q.addBindValue(s->mAvgHeight[0]);
    q.addBindValue(s->mAvgHeight[1]);
    q.addBindValue(s->mAvgHeight[2]);
    q.addBindValue(s->mAvgVolume[0]);
    q.addBindValue(s->mAvgVolume[1]);
    q.addBindValue(s->mAvgVolume[2]);
    q.addBindValue(s->mTimeSinceDeath[0]);
    q.addBindValue(s->mTimeSinceDeath[1]);
    q.addBindValue(s->mTimeSinceDeath[2]);
    q.addBindValue(s->mKSW[0]);
    q.addBindValue(s->mKSW[1]);
    q.addBindValue(s->mKSW[2]);
    q.addBindValue(s->mHalfLife[0]);
    q.addBindValue(s->mHalfLife[1]);
    q.addBindValue(s->mHalfLife[2]);
    q.addBindValue(s->mOtherWood[0].C); q.addBindValue(s->mOtherWood[0].N);
    q.addBindValue(s->mOtherWood[1].C); q.addBindValue(s->mOtherWood[1].N);
    q.addBindValue(s->mOtherWood[2].C); q.addBindValue(s->mOtherWood[2].N);
    q.addBindValue(s->mOtherWood[3].C); q.addBindValue(s->mOtherWood[3].N);
    q.addBindValue(s->mOtherWood[4].C); q.addBindValue(s->mOtherWood[4].N);
    q.addBindValue(s->mBranchCounter);
    q.addBindValue(s->mOtherWoodAbovegroundFrac);

    if (!q.exec()) {
        throw IException(QString("Snapshot::saveSnag: execute:") + q.lastError().text());
    }

}

void Snapshot::loadSnags(QSqlDatabase db)
{
    // if 'db' is not a valid data base, take the 'snapshot' database
    if (!db.isValid())
        db=QSqlDatabase::database("snapshot");

    QSqlQuery q(db);
    q.exec("select RUIndex, climateFactor, SWD1C, SWD1N, SWD2C, SWD2N, SWD3C, SWD3N, totalSWDC, totalSWDN, NSnags1, NSnags2, NSnags3, dbh1, dbh2, dbh3, height1, height2, height3, volume1, volume2, volume3, tsd1, tsd2, tsd3, ksw1, ksw2, ksw3, halflife1, halflife2, halflife3, branch1C, branch1N, branch2C, branch2N, branch3C, branch3N, branch4C, branch4N, branch5C, branch5N, branchIndex, branchAGFraction from snag");
    int ru_index = -1;
    ResourceUnit *ru = 0;
    int n=0;

    while (q.next()) {
        int ci=0;
        ru_index = q.value(ci++).toInt();
        ru = mRUHash[ru_index];
        if (!ru)
            continue;
        Snag *s = ru->snag();
        if (!s)
            continue;
        s->mClimateFactor = q.value(ci++).toDouble();
        s->mSWD[0].C = q.value(ci++).toDouble();
        s->mSWD[0].N = q.value(ci++).toDouble();
        s->mSWD[1].C = q.value(ci++).toDouble();
        s->mSWD[1].N = q.value(ci++).toDouble();
        s->mSWD[2].C = q.value(ci++).toDouble();
        s->mSWD[2].N = q.value(ci++).toDouble();
        s->mTotalSWD.C = q.value(ci++).toDouble();
        s->mTotalSWD.N = q.value(ci++).toDouble();
        s->mNumberOfSnags[0] = q.value(ci++).toDouble();
        s->mNumberOfSnags[1] = q.value(ci++).toDouble();
        s->mNumberOfSnags[2] = q.value(ci++).toDouble();
        s->mAvgDbh[0] = q.value(ci++).toDouble();
        s->mAvgDbh[1] = q.value(ci++).toDouble();
        s->mAvgDbh[2] = q.value(ci++).toDouble();
        s->mAvgHeight[0] = q.value(ci++).toDouble();
        s->mAvgHeight[1] = q.value(ci++).toDouble();
        s->mAvgHeight[2] = q.value(ci++).toDouble();
        s->mAvgVolume[0] = q.value(ci++).toDouble();
        s->mAvgVolume[1] = q.value(ci++).toDouble();
        s->mAvgVolume[2] = q.value(ci++).toDouble();
        s->mTimeSinceDeath[0] = q.value(ci++).toDouble();
        s->mTimeSinceDeath[1] = q.value(ci++).toDouble();
        s->mTimeSinceDeath[2] = q.value(ci++).toDouble();
        s->mKSW[0] = q.value(ci++).toDouble();
        s->mKSW[1] = q.value(ci++).toDouble();
        s->mKSW[2] = q.value(ci++).toDouble();
        s->mHalfLife[0] = q.value(ci++).toDouble();
        s->mHalfLife[1] = q.value(ci++).toDouble();
        s->mHalfLife[2] = q.value(ci++).toDouble();
        s->mOtherWood[0].C = q.value(ci++).toDouble(); s->mOtherWood[0].N = q.value(ci++).toDouble();
        s->mOtherWood[1].C = q.value(ci++).toDouble(); s->mOtherWood[1].N = q.value(ci++).toDouble();
        s->mOtherWood[2].C = q.value(ci++).toDouble(); s->mOtherWood[2].N = q.value(ci++).toDouble();
        s->mOtherWood[3].C = q.value(ci++).toDouble(); s->mOtherWood[3].N = q.value(ci++).toDouble();
        s->mOtherWood[4].C = q.value(ci++).toDouble(); s->mOtherWood[4].N = q.value(ci++).toDouble();
        s->mBranchCounter = q.value(ci++).toInt();
        s->mOtherWoodAbovegroundFrac = q.value(ci++).toDouble();

        // these values are not stored in DB but updated here
        s->mTotalOther = s->mOtherWood[0] + s->mOtherWood[1] + s->mOtherWood[2] + s->mOtherWood[3] + s->mOtherWood[4];
        s->mTotalSnagCarbon = s->mSWD[0].C + s->mSWD[1].C + s->mSWD[2].C + s->mTotalOther.C;


        if (++n % 1000 == 0) {
            qDebug() << n << "snags loaded...";
            QCoreApplication::processEvents();
        }
    }
    qDebug() << "Snapshot: finished snags. N=" << n;

}

void Snapshot::saveSaplings()
{
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    QSqlQuery q(db);
    if (!q.prepare(QString("insert into saplings (RUindex, species_index, posx, posy, age, height, stress_years) " \
                           "values (?,?,?,?,?,?,?)")))
        throw IException(QString("Snapshot::saveSaplings: prepare:") + q.lastError().text());

    int n = 0;
    db.transaction();

    if (!q.prepare(QString("insert into saplings (RUIndex, posx, posy, species_index, age, height, stress_years, flags) " \
                           "values (?,?,?,?,?,?,?,?)")))
        throw IException(QString("Snapshot::saveSaplings: prepare:") + q.lastError().text());


    Saplings *saplings = GlobalSettings::instance()->model()->saplings();

    FloatGrid &lif_grid= *GlobalSettings::instance()->model()->grid();
    ResourceUnit *RU;
    for (int y=0; y<lif_grid.sizeY();++y) {
        for (int x=0;x<lif_grid.sizeX(); ++x) {
            SaplingCell *sc=saplings->cell(QPoint(x,y), true, &RU);
            if (sc){
                for (int i=0;i<NSAPCELLS;++i) {
                    if (sc->saplings[i].is_occupied()) {
                        q.addBindValue(RU->index());
                        q.addBindValue(x);
                        q.addBindValue(y);
                        q.addBindValue(sc->saplings[i].species_index);
                        q.addBindValue(sc->saplings[i].age);
                        q.addBindValue(sc->saplings[i].height);
                        q.addBindValue(sc->saplings[i].stress_years);
                        q.addBindValue(sc->saplings[i].flags);
                        if (!q.exec()) {
                            throw IException(QString("Snapshot::saveStandSnapshot, saplings: execute:") + q.lastError().text());
                        }
                        ++n;
                        if (n<10000000 && ++n % 10000 == 0) {
                           qDebug() << n << "saplings saved...";
                           QCoreApplication::processEvents();
                       }
                       if (n>=10000000 && ++n % 1000000 == 0) {
                           qDebug() << n << "saplings saved...";
                           QCoreApplication::processEvents();
                       }
                    }
                }
            }
        }
    }
    db.commit();
    qDebug() << "Snapshot: finished saplings. N=" << n;
}

void Snapshot::loadSaplings()
{
    QSqlDatabase db=QSqlDatabase::database("snapshot");
    QSqlQuery q(db);
    q.setForwardOnly(true); // avoid huge memory usage in query component
    if (!q.exec("select RUindex, posx, posy, species_index, age, height, stress_years, flags from saplings")) {
        qDebug() << "Error when loading from saplings table...." << q.lastError().text();
        return;
    }
    int ru_index = -1;


    ResourceUnit *ru = 0;
    int n=0, ntotal=0;
    int ci;
    int posx, posy;
    int offsetx=0, offsety=0;
    Saplings *saplings = GlobalSettings::instance()->model()->saplings();

    // clear all saplings in the model
    saplings->clearAllSaplings();


    while (q.next()) {
        ci = 0;
        ru_index = q.value(ci++).toInt();
        ru = mRUHash[ru_index];
        if (!ru)
            continue;

        offsetx = ru->cornerPointOffset().x();
        offsety = ru->cornerPointOffset().y();

        posx = offsetx + q.value(ci++).toInt() % cPxPerRU;
        posy = offsety + q.value(ci++).toInt() % cPxPerRU;

        int species_index = q.value(ci++).toInt();

        SaplingCell *sc = saplings->cell(QPoint(posx, posy));
        if (!sc)
            continue;

        int age=q.value(ci++).toInt();
        SaplingTree *st = sc->addSapling(q.value(ci++).toFloat(), age, species_index);
        if (!st)
            continue;
        st->stress_years = static_cast<unsigned char> (q.value(ci++).toInt());
        st->flags = static_cast<unsigned char> (q.value(ci++).toInt());
        ++ntotal;

        ++n;
        if (n % 10000 == 0 )
            if ( (n<100000 ) ||   // until 100,000 every 10k
                 (n<1000000 && n % 100000 == 0) ||   // until 1M every 100k
                 (n % 1000000 == 0) ) {             // then every M
                qDebug() << n << "saplings loaded...";
                QCoreApplication::processEvents();
            }


    }
    qDebug() << "Snapshot: finished loading saplings. N=" << n << "from N in snapshot:" << ntotal;

}


void Snapshot::loadSaplingsOld()
{
//    QSqlDatabase db=QSqlDatabase::database("snapshot");
//    QSqlQuery q(db);
//    q.setForwardOnly(true); // avoid huge memory usage in query component
//    if (!q.exec("select RUindex, species, posx, posy, age, height, stress_years from saplings")) {
//        qDebug() << "Error when loading from saplings table...." << q.lastError().text();
//        return;
//    }
//    int ru_index = -1;

//    // clear all saplings in the whole project area: added for testing/debugging
////    foreach( ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
////        foreach (ResourceUnitSpecies *rus, ru->ruSpecies()) {
////            rus->changeSapling().clear();
////            rus->changeSapling().clearStatistics();
////        }
////    }

//    ResourceUnit *ru = 0;
//    int n=0, ntotal=0;
//    int ci;
//    int posx, posy;
//    int offsetx=0, offsety=0;
//    Sapling *last_sapling = 0;

//    while (q.next()) {
//        ci = 0;
//        ru_index = q.value(ci++).toInt();
//        ++ntotal;
//        ru = mRUHash[ru_index];
//        if (!ru)
//            continue;
//        Species *species = ru->speciesSet()->species(q.value(ci++).toString());
//        if (!species)
//            throw IException("Snapshot::loadSaplings: Invalid species");
//        Sapling &sap = ru->resourceUnitSpecies(species).changeSapling();
//        if (last_sapling != &sap) {
//            last_sapling = &sap;
//            sap.clear(); // clears the trees and the bitmap
//            sap.clearStatistics();
//            offsetx = ru->cornerPointOffset().x();
//            offsety = ru->cornerPointOffset().y();
//        }
//        sap.mSaplingTrees.push_back(SaplingTreeOld());
//        SaplingTreeOld &t = sap.mSaplingTrees.back();
//        //posx = q.value(ci++).toInt();
//        //posy = q.value(ci++).toInt();
//        posx = offsetx + q.value(ci++).toInt() % cPxPerRU;
//        posy = offsety + q.value(ci++).toInt() % cPxPerRU;
//        if (GlobalSettings::instance()->model()->grid()->isIndexValid(posx, posy)) {
//            t.pixel = GlobalSettings::instance()->model()->grid()->ptr(posx,posy );
//        } else {
//            continue;
//        }
//        t.age.age = static_cast<short unsigned int>( q.value(ci++).toInt() );
//        t.height = q.value(ci++).toFloat();
//        t.age.stress_years = static_cast<short unsigned int> (q.value(ci++).toInt() );
//        sap.setBit(QPoint(posx, posy), true); // set the flag in the bitmap
//        if (n<10000000 && ++n % 10000 == 0) {
//            qDebug() << n << "saplings loaded...";
//            QCoreApplication::processEvents();
//        }
//        if (n>=10000000 && ++n % 1000000 == 0) {
//            qDebug() << n << "saplings loaded...";
//            QCoreApplication::processEvents();
//        }


//    }
//    qDebug() << "Snapshot: finished loading saplings. N=" << n << "from N in snapshot:" << ntotal;

}


