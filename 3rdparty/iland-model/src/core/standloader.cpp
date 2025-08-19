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
#include "global.h"
#include "standloader.h"


#include "grid.h"
#include "model.h"
#include "resourceunit.h"
#include "speciesset.h"
#include "species.h"

#include "helper.h"
#include "random.h"
#include "expression.h"
#include "expressionwrapper.h"
#include "environment.h"
#include "csvfile.h"
#include "mapgrid.h"
#include "snapshot.h"
#include "grasscover.h"

/** @class StandLoader
    @ingroup tools
    loads (initializes) trees for a "stand" from various sources.
    StandLoader initializes trees on the landscape. It reads (usually) from text files, creates the
    trees and distributes the trees on the landscape (on the ResoureceUnit or on a stand defined by a grid).

    See https://iland-model.org/initialize+trees
  */
// provide a mapping between "Picus"-style and "iLand"-style species Ids
static QVector<int> picusSpeciesIds = QVector<int>() << 0 << 1 << 17;
static QStringList iLandSpeciesIds = QStringList() << "piab" << "piab" << "fasy";

StandLoader::~StandLoader()
{
    if (mRandom)
        delete mRandom;
    if (mHeightGridResponse)
        delete mHeightGridResponse;
    if (mInitHeightGrid)
        delete mInitHeightGrid;
}


void StandLoader::copyTrees()
{
    // we assume that all stands are equal, so wie simply COPY the trees and modify them afterwards
    const Grid<ResourceUnit*> &ruGrid=mModel->RUgrid();
    ResourceUnit **p = ruGrid.begin();
    if (!p)
        throw IException("Standloader: invalid resource unit pointer!");
    ++p; // skip the first...
    const QVector<Tree> &tocopy = mModel->ru()->trees();
    for (; p!=ruGrid.end(); ++p) {
        QRectF rect = (*p)->boundingBox();
        foreach(const Tree& tree, tocopy) {
            Tree &newtree = (*p)->newTree();
            newtree = tree; // copy tree data...
            newtree.setPosition(tree.position()+rect.topLeft());
            newtree.setRU(*p);
            newtree.setNewId();
        }
    }
    if (logLevelInfo()) qDebug() << Tree::statCreated() << "trees loaded / copied.";
}

/** main routine of the stand setup.
*/
void StandLoader::processInit()
{
    GlobalSettings *g = GlobalSettings::instance();
    XmlHelper xml(g->settings().node("model.initialization"));

    QString copy_mode = xml.value("mode", "copy");
    QString type = xml.value("type", "");
    QString  fileName = xml.value("file", "");

    bool height_grid_enabled = xml.valueBool("heightGrid.enabled", false);
    mHeightGridTries = xml.valueInt("heightGrid.maxTries", 10);
    mInitHeightGrid = nullptr;
    //QScopedPointer<const MapGrid> height_grid; // use a QScopedPointer to guarantee that the resource is freed at the end of the processInit() function
    const MapGrid *height_grid;
    if (height_grid_enabled) {
        QString init_height_grid_file = GlobalSettings::instance()->path(xml.value("heightGrid.fileName"), "init");
        qDebug() << "initialization: using predefined tree heights map" << init_height_grid_file;

        height_grid = new MapGrid(init_height_grid_file, false);
        if (!height_grid->isValid()) {
            delete height_grid;
            throw IException(QString("Error when loading grid with tree heights for stand initalization: file %1 not found or not valid.").arg(init_height_grid_file));
        }
        mInitHeightGrid = height_grid;

        QString expr=xml.value("heightGrid.fitFormula", "polygon(x, 0,0, 0.8,1, 1.1, 1, 1.25,0)");
        mHeightGridResponse = new Expression(expr);
        mHeightGridResponse->linearize(0., 2.);
    }

    Tree::resetStatistics();

    // one global init-file for the whole area:
    if (copy_mode=="single") {
        // useful for 1ha simulations only...
        //if (GlobalSettings::instance()->model()->ruList().size()>1)
        //    throw IException("Error initialization: 'mode' is 'single' but more than one resource unit is simulated (consider using another 'mode').");
        // -> is now ok for multiple resource units in a single file

        loadInitFile(fileName, type);
        evaluateDebugTrees();
        return;
    }


    // call a single tree init for each resource unit
    if (copy_mode=="unit") {
        foreach( const ResourceUnit *const_ru, g->model()->ruList()) {
            ResourceUnit *ru = const_cast<ResourceUnit*>(const_ru);
            // set environment
            g->model()->environment()->setPosition(ru->boundingBox().center());
            type = xml.value("type", "");
            fileName = xml.value("file", "");
            if (fileName.isEmpty())
                continue;
            loadInitFile(fileName, type, 0, ru);
            if (logLevelInfo()) qDebug() << "loaded" << fileName << "on" << ru->boundingBox() << "," << ru->trees().count() << "trees.";
        }
        evaluateDebugTrees();
        return;
    }

    // map-modus: load a init file for each "polygon" in the standgrid
    if (copy_mode=="map") {
        if (!g->model()->standGrid() || !g->model()->standGrid()->isValid())
            throw IException(QString("Stand-Initialization: model.initialization.mode is 'map' but there is no valid stand grid defined (model.world.standGrid)"));
        QString map_file_name = GlobalSettings::instance()->path(xml.value("mapFileName"), "init");

        CSVFile map_file(map_file_name);
        if (map_file.rowCount()==0)
            throw IException(QString("Stand-Initialization: the map file %1 is empty or missing!").arg(map_file_name));
        int ikey = map_file.columnIndex("id");
        int ivalue = map_file.columnIndex("filename");
        if (ikey<0 || ivalue<0)
            throw IException(QString("Stand-Initialization: the map file %1 does not contain the mandatory columns 'id' and 'filename'!").arg(map_file_name));
        QString file_name;
        for (int i=0;i<map_file.rowCount();i++) {
            int key = map_file.value(i, ikey).toInt();
            if (key>0) {
                file_name = map_file.value(i, ivalue).toString();
                if (logLevelInfo()) qDebug() << "loading" << file_name << "for grid id" << key;
                if (!file_name.isEmpty())
                    loadInitFile(file_name, type, key, NULL);
            }
        }
        mInitHeightGrid = nullptr;
        evaluateDebugTrees();
        return;
    }

    // standgrid mode: load one large init file
    if (copy_mode=="standgrid") {
        if (fileName.isEmpty()) {
            qDebug() << "Stand initialization: no file specifed, starting with bare ground.";
            return;
        }
        fileName = GlobalSettings::instance()->path(fileName, "init");

        if (!QFile::exists(fileName))
            throw IException(QString("load-ini-file: file '%1' does not exist.").arg(fileName));
        QStringList content = Helper::loadTextFileLines(fileName);
        // this processes the init file (also does the checking) and
        // stores in a QHash datastrucutre
        parseInitFile(content, fileName);

        // setup the random distribution
        QString density_func = xml.value("model.initialization.randomFunction", "1-x^2");
        if (logLevelInfo())  qDebug() << "density function:" << density_func;
        if (!mRandom || (mRandom->densityFunction()!= density_func)) {
            if (mRandom)
                delete mRandom;
            mRandom=new RandomCustomPDF(density_func);
            if (logLevelInfo()) qDebug() << "new probabilty density function:" << density_func;
        }

        if (mStandInitItems.isEmpty()) {
            qDebug() << "Initialize trees ('standgrid'-mode): no items to process (empty landscape)(no 'stand_id' column present?).";
            return;
            //throw IException("StandLoader::processInit: 'mode' is 'standgrid' but the init file is either empty or contains no 'stand_id'-column.");
        }
        QHash<int, QVector<InitFileItem> >::const_iterator it = mStandInitItems.constBegin();
        while (it!=mStandInitItems.constEnd()) {
            mInitItems = it.value(); // copy the items...
            executeiLandInitStand(it.key());
            ++it;
        }
        qDebug() << "finished setup of trees.";
        evaluateDebugTrees();
        return;

    }
    if (copy_mode=="snapshot") {
        // load a snapshot from a file
        GlobalSettings::instance()->model()->setCurrentTask("loading snapshot database");
        Snapshot shot;

        QString input_db = GlobalSettings::instance()->path(fileName);
        shot.loadSnapshot(input_db);
        return;
    }
    throw IException("StandLoader::processInit: invalid initalization.mode!");
}

void StandLoader::processAfterInit()
{
    XmlHelper xml(GlobalSettings::instance()->settings().node("model.initialization"));

    QString mode = xml.value("mode", "copy");
    if (mode=="standgrid") {
        // load a file with saplings per polygon
        QString  filename = xml.value("saplingFile", "");
        if (filename.isEmpty())
            return;
        filename = GlobalSettings::instance()->path(filename, "init");
        if (!QFile::exists(filename))
            throw IException(QString("load-sapling-ini-file: file '%1' does not exist.").arg(filename));
        CSVFile init_file(filename);
        int istandid = init_file.columnIndex("stand_id");
        if (istandid==-1)
            throw IException("Sapling-Init: the init file contains no 'stand_id' column (required in 'standgrid' mode).");

        int stand_id = -99999;
        int ilow = -1, ihigh = 0;
        int total = 0;
        for (int i=0;i<init_file.rowCount();++i) {
            int row_stand = init_file.value(i, istandid).toInt();
            if (row_stand != stand_id) {
                if (stand_id>=0) {
                    // process stand
                    ihigh = i-1; // up to the last
                    total += loadSaplingsLIF(stand_id, init_file, ilow, ihigh);
                }
                ilow = i; // mark beginning of new stand
                stand_id = row_stand;
            }
        }
        if (stand_id>=0)
            total += loadSaplingsLIF(stand_id, init_file, ilow, init_file.rowCount()-1); // the last stand
        qDebug() << "initialization of sapling: total created:" << total;

    }

}

void StandLoader::evaluateDebugTrees()
{
    // evaluate debugging
    QString dbg_str = GlobalSettings::instance()->settings().paramValueString("debug_tree");
    int counter=0;
    if (!dbg_str.isEmpty()) {
        if (dbg_str == "debugstamp") {
            qDebug() << "debug_tree = debugstamp: try touching all trees...";
            // try to force an error if a stamp is invalid
            AllTreeIterator at(GlobalSettings::instance()->model());
            double total_offset=0.;
            while (Tree *t=at.next()) {
                total_offset += t->stamp()->offset();
                if (!GlobalSettings::instance()->model()->grid()->isIndexValid(t->positionIndex()))
                    qDebug() << "evaluateDebugTrees: debugstamp: invalid position found!";
            }
            qDebug() << "debug_tree = debugstamp: try touching all trees finished..." << total_offset;
            return;
        }
        TreeWrapper tw;
        Expression dexp(dbg_str, &tw); // load expression dbg_str and enable external model variables
        AllTreeIterator at(GlobalSettings::instance()->model());
        double result;
        while (Tree *t = at.next()) {
            tw.setTree(t);
            result = dexp.execute();
            if (result) {
                t->enableDebugging();
                counter++;
            }
        }
        qDebug() << "evaluateDebugTrees: enabled debugging for" << counter << "trees.";
    }
}


/// load a single init file. Calls loadPicusFile() or loadiLandFile()
/// @param fileName file to load
/// @param type init mode. allowed: "picus"/"single" or "iland"/"distribution"
int StandLoader::loadInitFile(const QString &fileName, const QString &type, int stand_id, ResourceUnit *ru)
{
    QString pathFileName = GlobalSettings::instance()->path(fileName, "init");
    if (!QFile::exists(pathFileName))
        throw IException(QString("StandLoader::loadInitFile: File '%1' does not exist!").arg(pathFileName));

    if (type=="picus" || type=="single") {
        //if (stand_id>0)
        //    throw IException(QString("StandLoader::loadInitFile: initialization type '%1' currently not supported for stand initilization mode!").arg(type));
        return loadPicusFile(pathFileName, ru, stand_id);
    }
    if (type=="iland" || type=="distribution")
        return loadiLandFile(pathFileName, ru, stand_id);

    throw IException(QString("StandLoader::loadInitFile: unknown initalization.type: '%1'").arg(type));
}

int StandLoader::loadPicusFile(const QString &fileName, ResourceUnit *ru, int stand_id)
{
    QStringList content = Helper::loadTextFileLines(fileName);
    if (content.isEmpty()) {
        qDebug() << "file not found: " + fileName;
        return 0;
    }
    return loadSingleTreeList(content, ru, stand_id, fileName);
}

/** load a list of trees (given by content) to a resource unit. Param fileName is just for error reporting.
  returns the number of loaded trees.
  */
int StandLoader::loadSingleTreeList(QStringList content, ResourceUnit *ru_offset, int stand_id, const QString &fileName)
{
    QPointF offset;
    if (ru_offset && stand_id<0) {
        offset = ru_offset->boundingBox().topLeft();
    }

    ResourceUnit *ru;

//    QPointF offset = ru->boundingBox().topLeft();
    SpeciesSet *speciesSet = GlobalSettings::instance()->model()->speciesSet();
    const Grid<ResourceUnit*> &rugrid = GlobalSettings::instance()->model()->RUgrid();

    int lineno=0;
    for (const auto &line : content) {
        if (line.contains("<trees>")) {
            // there is a Picus-style trees tag - remove and search for the closing tag
            // not tested!
            content.removeAt(lineno);
            size_t l = content.indexOf("</trees>");
            if (l>0)
                content.removeAt(l);
            break;
        }
        if (lineno++ > 100)
            break;
    }

    CSVFile infile;
    infile.loadFromStringList(content);


    int iID = infile.columnIndex("id");
    int iX = infile.columnIndex("x");
    int iY = infile.columnIndex("y");
    int iBhd = infile.columnIndex("bhdfrom");
    if (iBhd<0)
        iBhd = infile.columnIndex("dbh");
    double height_conversion = 100.;
    int iHeight =infile.columnIndex("treeheight");
    if (iHeight<0) {
        iHeight = infile.columnIndex("height");
        height_conversion = 1.; // in meter
    }
    int iSpecies = infile.columnIndex("species");
    int iAge = infile.columnIndex("age");
    if (iX==-1 || iY==-1 || iBhd==-1 || iSpecies==-1 || iHeight==-1)
        throw IException(QString("Initfile %1 is not valid!\nRequired columns are: x,y, bhdfrom or dbh, species, treeheight or height.").arg(fileName));

    double dbh;
    bool ok;
    int cnt=0;
    QString speciesid;
    for (int i=0;i<infile.rowCount();i++) {
        dbh = infile.value(i, iBhd).toDouble();

        QPointF f;
        f.setX( infile.value(i, iX).toDouble() );
        f.setY( infile.value(i, iY).toDouble() );
        f += offset; // if the input is relative to a given resource unit

        // position valid?
        if (!rugrid.coordValid(f))
            continue;

        if (!mModel->heightGrid()->valueAt(f).isValid())
            continue;

        // get resource unit
        ru = rugrid.constValueAt(f);
        if (!ru)
            continue;

        Tree &tree = ru->newTree();
        tree.setPosition(f);
        if (iID>=0)
            tree.setId(infile.value(i, iID).toInt());

        tree.setDbh(dbh);
        tree.setHeight(infile.value(i,iHeight).toDouble()/height_conversion); // convert from Picus-cm to m if necessary

        speciesid = infile.value(i,iSpecies).toString();
        int picusid = speciesid.toInt(&ok);
        if (ok) {
            int idx = picusSpeciesIds.indexOf(picusid);
            if (idx==-1)
                throw IException(QString("Loading init-file: invalid Picus-species-id. Species: %1").arg(picusid));
            speciesid = iLandSpeciesIds[idx];
        }
        Species *s = speciesSet->species(speciesid);
        if (!ru || !s)
            throw IException(QString("Loading init-file: either resource unit or species invalid. Species: %1").arg(speciesid));
        tree.setSpecies(s);

        ok = true;
        if (iAge>=0)
           tree.setAge(infile.value(i, iAge).toInt(&ok), tree.height()); // this is a *real* age
        if (iAge<0 || !ok || tree.age()==0)
           tree.setAge(0, tree.height()); // no real tree age available

        tree.setRU(ru);
        tree.setup();
        cnt++;
    }
    return cnt;
    //qDebug() << "loaded init-file contained" << lines.count() <<"lines.";
    //qDebug() << "lines: " << lines;
}

/** initialize trees on a resource unit based on dbh distributions.
  use a fairly clever algorithm to determine tree positions.
  see https://iland-model.org/initialize+trees
  @param content tree init file (including headers) in a string
  @param ru resource unit
  @param fileName source file name (for error reporting)
  @return number of trees added
  */
int StandLoader::loadDistributionList(const QStringList &content, ResourceUnit *ru, int stand_id, const QString &fileName)
{
    int total_count = parseInitFile(content, fileName, ru);
    if (total_count==0)
        return 0;


    // setup the random distribution
    QString density_func = GlobalSettings::instance()->settings().value("model.initialization.randomFunction", "1-x^2");
    if (logLevelInfo())  qDebug() << "density function:" << density_func;
    if (!mRandom || (mRandom->densityFunction()!= density_func)) {
        if (mRandom)
            delete mRandom;
        mRandom=new RandomCustomPDF(density_func);
        if (logLevelInfo()) qDebug() << "new probabilty density function:" << density_func;
    }
    if (stand_id>0) {
        // execute stand based initialization
        executeiLandInitStand(stand_id);
    } else {
        // exeucte the initialization based on single resource units (in this case we care for the ru parameter)
        ResourceUnit *ru_load = ru ? ru : GlobalSettings::instance()->model()->ru();
        executeiLandInit(ru_load);
        ru_load->cleanTreeList();
    }
    return total_count;

}

int StandLoader::parseInitFile(const QStringList &content, const QString &fileName, ResourceUnit* ru)
{
    if (!ru)
        ru = mModel->ru();
    Q_ASSERT(ru!=0);
    SpeciesSet *speciesSet = ru->speciesSet(); // of default RU
    Q_ASSERT(speciesSet!=0);

    //DebugTimer t("StandLoader::loadiLandFile");
    CSVFile infile;
    infile.loadFromStringList(content);

    int icount = infile.columnIndex("count");
    int ispecies = infile.columnIndex("species");
    int idbh_from = infile.columnIndex("dbh_from");
    int idbh_to = infile.columnIndex("dbh_to");
    int ihd = infile.columnIndex("hd");
    int iage = infile.columnIndex("age");
    int idensity = infile.columnIndex("density");
    if (icount<0 || ispecies<0 || idbh_from<0 || idbh_to<0 || ihd<0 || iage<0)
        throw IException(QString("load-ini-file: file '%1' containts not all required fields (count, species, dbh_from, dbh_to, hd, age).").arg(fileName));

    int istandid = infile.columnIndex("stand_id");
    mInitItems.clear();
    mStandInitItems.clear();

    InitFileItem item;
    bool ok;
    int total_count = 0;
    for (int row=0;row<infile.rowCount();row++) {
        item.count = infile.value(row, icount).toDouble();
        total_count += item.count;
        item.dbh_from = infile.value(row, idbh_from).toDouble();
        item.dbh_to = infile.value(row, idbh_to).toDouble();
        item.hd = infile.value(row, ihd).toDouble();
        if (item.hd==0. || item.dbh_from / 100. * item.hd < cSapHeight)
            qWarning() << QString("load init file: file '%1' tries to init trees below minimum height. hd=%2, dbh=%3.").arg(fileName).arg(item.hd).arg(item.dbh_from) ;
            //throw IException(QString("load init file: file '%1' tries to init trees below minimum height. hd=%2, dbh=%3.").arg(fileName).arg(item.hd).arg(item.dbh_from) );
        ok = true;
        if (iage>=0)
            item.age = infile.value(row, iage).toInt(&ok);
        if (iage<0 || !ok)
            item.age = 0;

        item.species = speciesSet->species(infile.value(row, ispecies).toString());
        if (idensity>=0)
            item.density = infile.value(row, idensity).toDouble();
        else
            item.density = 0.;
        if (item.density<-1)
            throw IException(QString("load-ini-file: invalid value for density. Allowed range is -1..1: '%1' in file '%2', line %3.")
                             .arg(item.density)
                             .arg(fileName)
                             .arg(row));
        if (!item.species) {
            throw IException(QString("load-ini-file: unknown speices '%1' in file '%2', line %3.")
                             .arg(infile.value(row, ispecies).toString())
                             .arg(fileName)
                             .arg(row));
        }
        if (istandid>=0) {
            int standid = infile.value(row,istandid).toInt();
            mStandInitItems[standid].push_back(item);
        } else {
            mInitItems.push_back(item);
        }
    }
    return total_count;

}


int StandLoader::loadiLandFile(const QString &fileName, ResourceUnit *ru, int stand_id)
{
    if (!QFile::exists(fileName))
        throw IException(QString("load-ini-file: file '%1' does not exist.").arg(fileName));
    QStringList content = Helper::loadTextFileLines(fileName);
    return loadDistributionList(content, ru, stand_id, fileName);
}

// evenlist: tentative order of pixel-indices (within a 5x5 grid) used as tree positions.
// e.g. 12 = centerpixel, 0: upper left corner, ...
int evenlist[25] = { 12, 6, 18, 16, 8, 22, 2, 10, 14, 0, 24, 20, 4,
                     1, 13, 15, 19, 21, 3, 7, 11, 17, 23, 5, 9};
int unevenlist[25] = { 11,13,7,17, 1,19,5,21, 9,23,3,15,
                       6,18,2,10,4,24,12,0,8,14,20,22};



// sort function
bool sortPairLessThan(const QPair<int, double> &s1, const QPair<int, double> &s2)
{
    return s1.second < s2.second;
}

struct SInitPixel {
    double basal_area; // accumulated basal area
    QPoint pixelOffset; // location of the pixel
    ResourceUnit *resource_unit; // pointer to the resource unit the pixel belongs to
    double h_max; // predefined maximum height at given pixel (if available from LIDAR or so)
    bool locked; // pixel is dedicated to a single species
    SInitPixel(): basal_area(0.), resource_unit(0), h_max(-1.), locked(false) {}
};

bool sortInitPixelLessThan(const SInitPixel &s1, const SInitPixel &s2)
{
    return s1.basal_area < s2.basal_area;
}

bool sortInitPixelUnlocked(const SInitPixel &s1, const SInitPixel &s2)
{
    return !s1.locked && s2.locked;
}

/**
*/

void StandLoader::executeiLandInit(ResourceUnit *ru)
{

    QPointF offset = ru->boundingBox().topLeft();
    QPoint offsetIdx = GlobalSettings::instance()->model()->grid()->indexAt(offset);

    // a multimap holds a list for all trees.
    // key is the index of a 10x10m pixel within the resource unit
    QMultiMap<int, int> tree_map;
    //QHash<int,SInitPixel> tcount;

    QVector<QPair<int, double> > tcount; // counts
    for (int i=0;i<100;i++)
        tcount.push_back(QPair<int,double>(i,0.));

    int key;
    double rand_val, rand_fraction;
    int total_count = 0;
    foreach(const InitFileItem &item, mInitItems) {
        rand_fraction = fabs(double(item.density));
        for (int i=0;i<item.count;i++) {
            // create trees
            int tree_idx = ru->newTreeIndex();
            Tree &tree = ru->trees()[tree_idx]; // get reference to modify tree
            tree.setDbh(nrandom(item.dbh_from, item.dbh_to));
            tree.setHeight(tree.dbh()/100. * item.hd); // dbh from cm->m, *hd-ratio -> meter height
            tree.setSpecies(item.species);
            if (item.age<=0)
                tree.setAge(0,tree.height());
            else
                tree.setAge(item.age, tree.height());
            tree.setRU(ru);
            tree.setup();
            total_count++;

            // calculate random value. "density" is from 1..-1.
            rand_val = mRandom->get();
            if (item.density<0)
                rand_val = 1. - rand_val;
            rand_val = rand_val * rand_fraction + drandom()*(1.-rand_fraction);

            // key: rank of target pixel
            // first: index of target pixel
            // second: sum of target pixel
            key = limit(int(100*rand_val), 0, 99); // get from random number generator
            tree_map.insert(tcount[key].first, tree_idx); // store tree in map
            tcount[key].second+=tree.basalArea(); // aggregate the basal area for each 10m pixel
            if ( (total_count < 20 && i%2==0)
                || (total_count<100 && i%10==0 )
                || (i%30==0) ) {
                std::sort(tcount.begin(), tcount.end(), sortPairLessThan);
            }
        }
        std::sort(tcount.begin(), tcount.end(), sortPairLessThan);
    }

    unsigned int bits;
    int index, pos;
    int c;
    QList<int> trees;
    QPoint tree_pos;

    for (int i=0;i<100;i++) {
        trees = tree_map.values(i);
        c = trees.count();
        QPointF pixel_center = ru->boundingBox().topLeft() + QPointF((i/10)*10. + 5., (i%10)*10. + 5.);
        if (!mModel->heightGrid()->valueAt(pixel_center).isValid()) {
            // no trees on that pixel: let trees die
            foreach(int tree_idx, trees) {
                ru->trees()[tree_idx].die();
            }
            continue;
        }

        bits = 0;
        index = -1;
        double r;
        foreach(int tree_idx, trees) {
            if (c>18) {
                index = (index + 1)%25;
            } else {
                int stop=1000;
                index = 0;
                do {
                    //r = drandom();
                    //if (r<0.5)  // skip position with a prob. of 50% -> adds a little "noise"
                    //    index++;
                    //index = (index + 1)%25; // increase and roll over

                    // search a random position
                    r = drandom();
                    index = limit(int(25 *  r*r), 0, 24); // use rnd()^2 to search for locations -> higher number of low indices (i.e. 50% of lookups in first 25% of locations)
                } while (isBitSet(bits, index)==true && stop--);
                if (!stop)
                    qDebug() << "executeiLandInit: found no free bit.";
                setBit(bits, index, true); // mark position as used
            }
            // get position from fixed lists (one for even, one for uneven resource units)
            pos = ru->index()%2?evenlist[index]:unevenlist[index];
            tree_pos = offsetIdx  // position of resource unit
                       + QPoint(5*(i/10), 5*(i%10)) // relative position of 10x10m pixel
                       + QPoint(pos/5, pos%5); // relative position within 10x10m pixel
            //qDebug() << tree_no++ << "to" << index;
            ru->trees()[tree_idx].setPosition(tree_pos);
        }
    }
}



// Initialization routine based on a stand map.
// Basically a list of 10m pixels for a given stand is retrieved
// and the filled with the same procedure as the resource unit based init
// see https://iland-model.org/initialize+trees
void StandLoader::executeiLandInitStand(int stand_id)
{

    const MapGrid *grid = GlobalSettings::instance()->model()->standGrid();
    if (mCurrentMap)
        grid = mCurrentMap;

    // get a list of positions of all pixels that belong to our stand
    QList<int> indices = grid->gridIndices(stand_id);
    if (indices.isEmpty()) {
        qDebug() << "stand" << stand_id << "not in project area. No init performed.";
        return;
    }
    // a multiHash holds a list for all trees.
    // key is the location of the 10x10m pixel
    QMultiHash<QPoint, int> tree_map;
    QList<SInitPixel> pixel_list; // working list of all 10m pixels
    pixel_list.reserve(indices.size());

    foreach (int i, indices) {
        SInitPixel p;
        p.pixelOffset = grid->grid().indexOf(i); // index in the 10m grid
        p.resource_unit = GlobalSettings::instance()->model()->ru( grid->grid().cellCenterPoint(p.pixelOffset));
        if (mInitHeightGrid)
            p.h_max = mInitHeightGrid->grid().constValueAtIndex(p.pixelOffset);
        if (p.resource_unit)
            pixel_list.append(p);
        else
            qDebug() << "Init: no valid resource unit at" << grid->grid().cellCenterPoint(p.pixelOffset) << "for stand" << stand_id << GlobalSettings::instance()->model()->ru( grid->grid().cellCenterPoint(p.pixelOffset));
    }
    if (pixel_list.isEmpty()) {
        qDebug() << "Init: skipping stand" << stand_id << ", no valid pixels.";
        return;
    }
    double area_factor = grid->area(stand_id) / cRUArea;

    int key=0;
    double rand_val, rand_fraction;
    int total_count = 0;
    int total_tries = 0;
    int total_misses = 0;
    if (mInitHeightGrid && !mHeightGridResponse)
        throw IException("executeiLandInitStand: trying to initialize with height grid but without response function.");

    Species *last_locked_species=0;
    foreach(const InitFileItem &item, mInitItems) {
        if (item.density>1.) {
            // special case with single-species-area
            if (total_count==0) {
                // randomize the pixels
                for (QList<SInitPixel>::iterator it=pixel_list.begin();it!=pixel_list.end();++it)
                    it->basal_area = drandom();
                std::sort(pixel_list.begin(), pixel_list.end(), sortInitPixelLessThan);
                for (QList<SInitPixel>::iterator it=pixel_list.begin();it!=pixel_list.end();++it)
                    it->basal_area = 0.;
            }

            if (item.species != last_locked_species) {
                last_locked_species=item.species;
                std::sort(pixel_list.begin(), pixel_list.end(), sortInitPixelUnlocked);
            }
        } else {
            std::sort(pixel_list.begin(), pixel_list.end(), sortInitPixelLessThan);
            last_locked_species=0;
        }
        rand_fraction = item.density;
        int count = item.count * area_factor + 0.5; // round
        double init_max_height = item.dbh_to/100. * item.hd;
        for (int i=0;i<count;i++) {

            bool found = false;
            int tries = mHeightGridTries;
            while (!found &&--tries) {
                // calculate random value. "density" is from 1..-1.
                if (item.density <= 1.) {
                    rand_val = mRandom->get();
                    if (item.density<0)
                        rand_val = 1. - rand_val;
                    rand_val = rand_val * rand_fraction + drandom()*(1.-rand_fraction);
                } else {
                    // limited area: limit potential area using the "density" input parameter
                    rand_val = drandom() * qMin(item.density/100., 1.);
                }
                ++total_tries;

                // key: rank of target pixel
                key = limit(int(pixel_list.count()*rand_val), 0, pixel_list.count()-1); // get from random number generator

                if (mInitHeightGrid) {
                    // calculate how good the selected pixel fits w.r.t. the predefined height
                    double p_value = pixel_list[key].h_max>0.?mHeightGridResponse->calculate(init_max_height/pixel_list[key].h_max):0.;
                    if (drandom() < p_value)
                        found = true;
                } else {
                    found = true;
                }
                if (!last_locked_species && pixel_list[key].locked)
                    found = false;
            }
            if (tries<0) ++total_misses;

            // create a tree
            ResourceUnit *ru = pixel_list[key].resource_unit;
            int tree_idx = ru->newTreeIndex();
            Tree &tree = ru->trees()[tree_idx]; // get reference to modify tree
            tree.setDbh(nrandom(item.dbh_from, item.dbh_to));
            tree.setHeight(tree.dbh()/100. * item.hd); // dbh from cm->m, *hd-ratio -> meter height
            tree.setSpecies(item.species);
            if (item.age<=0)
                tree.setAge(0,tree.height());
            else
                tree.setAge(item.age, tree.height());
            tree.setRU(ru);
            tree.setup();
            total_count++;

            // store in the multiHash the position of the pixel and the tree_idx in the resepctive resource unit
            tree_map.insert(pixel_list[key].pixelOffset, tree_idx);
            pixel_list[key].basal_area+=tree.basalArea(); // aggregate the basal area for each 10m pixel
            if (last_locked_species)
                pixel_list[key].locked = true;

            // resort list
            if (last_locked_species==0 && ((total_count < 20 && i%2==0)
                || (total_count<100 && i%10==0 )
                || (i%30==0)) ) {
                std::sort(pixel_list.begin(), pixel_list.end(), sortInitPixelLessThan);
            }
        }
    }
    if (total_misses>0 || total_tries > total_count) {
        if (logLevelInfo()) qDebug() << "init for stand" << stand_id << "treecount:" << total_count << ", tries:" << total_tries << ", misses:" << total_misses << ", %miss:" << qRound(total_misses*100 / (double)total_count);
    }

    unsigned int bits;
    int index, pos;
    int c;
    QList<int> trees;
    QPoint tree_pos;

    foreach(const SInitPixel &p, pixel_list) {
        trees = tree_map.values(p.pixelOffset);
        c = trees.count();
        bits = 0;
        index = -1;
        double r;
        foreach(int tree_idx, trees) {
            if (c>18) {
                index = (index + 1)%25;
            } else {
                int stop=1000;
                index = 0;
                do {
                    // search a random position
                    r = drandom();
                    index = limit(int(25 *  r*r), 0, 24); // use rnd()^2 to search for locations -> higher number of low indices (i.e. 50% of lookups in first 25% of locations)
                } while (isBitSet(bits, index)==true && stop--);
                if (!stop)
                    qDebug() << "executeiLandInit: found no free bit.";
                setBit(bits, index, true); // mark position as used
            }
            // get position from fixed lists (one for even, one for uneven resource units)
            pos = p.resource_unit->index()%2?evenlist[index]:unevenlist[index];
            tree_pos = p.pixelOffset * cPxPerHeight; // convert to LIF index
            tree_pos += QPoint(pos/cPxPerHeight, pos%cPxPerHeight);

            p.resource_unit->trees()[tree_idx].setPosition(tree_pos);
            // test if tree position is valid..
            if (!GlobalSettings::instance()->model()->grid()->isIndexValid(tree_pos))
                qDebug() << "Standloader: invalid position!";
        }
    }
    if (logLevelInfo())
        qDebug() << "init for stand" << stand_id << "with area" << "area (m2)" << grid->area(stand_id) << "count of 10m pixels:"  << indices.count() << "initialized trees:" << total_count;

}

/// a (hacky) way of adding saplings of a certain age to a stand defined by 'stand_id'.
int StandLoader::loadSaplings(const QString &content, int stand_id, const QString &fileName)
{
    Q_UNUSED(fileName);
    const MapGrid *stand_grid;
    if (mCurrentMap)
        stand_grid = mCurrentMap; // if set
    else
        stand_grid = GlobalSettings::instance()->model()->standGrid(); // default

    QList<int> indices = stand_grid->gridIndices(stand_id); // list of 10x10m pixels
    if (indices.isEmpty()) {
        qDebug() << "stand" << stand_id << "not in project area. No init performed.";
        return -1;
    }
    double area_factor = stand_grid->area(stand_id) / cRUArea; // multiplier for grid (e.g. 2 if stand has area of 2 hectare)

    // parse the content of the init-file
    // species
    CSVFile init;
    init.loadFromString(content);
    int ispecies = init.columnIndex("species");
    int icount = init.columnIndex("count");
    int iheight = init.columnIndex("height");
    int iage = init.columnIndex("age");
    if (ispecies==-1 || icount==-1)
        throw IException("Error while loading saplings: columns 'species' or 'count' are missing!!");

    const SpeciesSet *set = GlobalSettings::instance()->model()->ru()->speciesSet();
    double height, age;
    int total = 0;
    for (int row=0;row<init.rowCount();++row) {
        int pxcount = qRound(init.value(row, icount).toDouble() * area_factor + 0.5); // no. of pixels that should be filled (sapling grid is the same resolution as the lif-grid)
        const Species *species = set->species(init.value(row, ispecies).toString());
        if (!species)
            throw IException(QString("Error while loading saplings: invalid species '%1'.").arg(init.value(row, ispecies).toString()));
        height = iheight==-1?0.05: init.value(row, iheight).toDouble();
        age = iage==-1?1:init.value(row,iage).toDouble();

        int misses = 0;
        int hits = 0;
        while (hits < pxcount) {
           int rnd_index = irandom(0, indices.count());
           QPoint offset=stand_grid->grid().indexOf(indices[rnd_index]);
           //
           offset = offset * cPxPerHeight; // index of 10m patch -> to lif pixel coordinates
           int in_p = irandom(0, cPxPerHeight*cPxPerHeight); // index of lif-pixel
           offset += QPoint(in_p / cPxPerHeight, in_p % cPxPerHeight);
           SaplingCell *sc = GlobalSettings::instance()->model()->saplings()->cell(offset);
           if (sc){
               if (sc->max_height()>height || sc->saplingOfSpecies(species->index())) {
               //if (!ru || ru->saplingHeightForInit(offset) > height) {
                   misses++;
               } else {
                   // ok
                   hits++;
                   sc->addSapling(height, age, species->index());
               }
               if (misses > 3*pxcount) {
                   qDebug() << "tried to add" << pxcount << "saplings at stand" << stand_id << "but failed in finding enough free positions. Added" << hits << "and stopped.";
                   break;
               }
           }
        }
        total += hits;

    }
    return total;
}

bool LIFValueHigher(const float *a, const float *b)
{
    return *a > *b;
}

int StandLoader::loadSaplingsLIF(int stand_id, const CSVFile &init, int low_index, int high_index)
{
    const MapGrid *stand_grid;
    if (mCurrentMap)
        stand_grid = mCurrentMap; // if set
    else
        stand_grid = GlobalSettings::instance()->model()->standGrid(); // default

    if (!stand_grid->isValid(stand_id))
        return 0;

    if (!GlobalSettings::instance()->model()->saplings()) {
        return 0;
    }

    QList<int> indices = stand_grid->gridIndices(stand_id); // list of 10x10m pixels
    if (indices.isEmpty()) {
        qDebug() << "stand" << stand_id << "not in project area. No init performed.";
        return 0;
    }
    // prepare space for LIF-pointers (2m Pixel)
    QVector<float*> lif_ptrs;
    lif_ptrs.reserve(indices.size() * cPxPerHeight * cPxPerHeight);
    for (int l=0;l<indices.size();++l){
        QPoint offset=stand_grid->grid().indexOf(indices[l]);
        offset = offset * cPxPerHeight; // index of 10m patch -> to lif pixel coordinates
        for (int y=0;y<cPxPerHeight;++y)
            for(int x=0;x<cPxPerHeight;++x)
                lif_ptrs.push_back( GlobalSettings::instance()->model()->grid()->ptr(offset.x()+x, offset.y()+y) );
    }
    // sort based on LIF-Value
    std::sort(lif_ptrs.begin(), lif_ptrs.end(), LIFValueHigher); // higher: highest values first


    double area_factor = stand_grid->area(stand_id) / cRUArea; // multiplier for grid (e.g. 2 if stand has area of 2 hectare)

    // parse the content of the init-file
    // species
    int ispecies = init.columnIndex("species");
    int icount = init.columnIndex("count");
    int iheight = init.columnIndex("height");
    int iheightfrom = init.columnIndex("height_from");
    int iheightto = init.columnIndex("height_to");
    int iage = init.columnIndex("age");
    int itopage = init.columnIndex("age4m");
    int iminlif = init.columnIndex("min_lif");
    if ((iheightfrom==-1) ^ (iheightto==-1))
        throw IException("Error while loading saplings: height not correctly provided. Use either 'height' or 'height_from' and 'height_to'.");
    if (ispecies==-1 || icount==-1)
        throw IException("Error while loading saplings: columns 'species' or 'count' are missing!!");

    const SpeciesSet *set = GlobalSettings::instance()->model()->ru()->speciesSet();
    double height, age;
    int total = 0;
    for (int row=low_index;row<=high_index;++row) {
        double pxcount = init.value(row, icount).toDouble() * area_factor; // no. of pixels that should be filled (sapling grid is the same resolution as the lif-grid)
        const Species *species = set->species(init.value(row, ispecies).toString());
        if (!species)
            throw IException(QString("Error while loading saplings: invalid species '%1'.").arg(init.value(row, ispecies).toString()));
        height = iheight==-1?0.05: init.value(row, iheight).toDouble();
        age = iage==-1?1:init.value(row,iage).toDouble();
        double age4m = itopage==-1?10:init.value(row, itopage).toDouble();
        double height_from = iheightfrom==-1?-1.: init.value(row, iheightfrom).toDouble();
        double height_to = iheightto==-1?-1.: init.value(row, iheightto).toDouble();
        double min_lif = iminlif==-1?1.: init.value(row, iminlif).toDouble();
        // find LIF-level in the pixels
        int min_lif_index = 0;
        if (min_lif < 1.) {
            for (QVector<float*>::ConstIterator it=lif_ptrs.constBegin(); it!=lif_ptrs.constEnd(); ++it, ++min_lif_index) {
                if (**it <= min_lif)
                    break;
            }
            if (pxcount > min_lif_index) {
                // not enough LIF pixels available
                // try the brightest pixels (ie with the largest value for the LIF), but limit to the maximum
                // number of cells
                min_lif_index = std::min(static_cast<qsizetype>(pxcount),  lif_ptrs.size());
            }
        } else {
            // No LIF threshold: the full range of pixels is valid
            min_lif_index = lif_ptrs.size();
        }



        double hits = 0.;
        int misses = 0;
        int n_added = 0;
        while (hits < pxcount) {
            int rnd_index = irandom(0, min_lif_index);
            if (iheightfrom!=-1) {
                height = limit(nrandom(height_from, height_to), 0.05, cSapHeight);
                if (age<=1.)
                    age = qMax(qRound(height/cSapHeight * age4m),1); // assume a linear relationship between height and age
            }
            QPoint offset = GlobalSettings::instance()->model()->grid()->indexOf(lif_ptrs[rnd_index]);
            ResourceUnit *ru;
            SaplingCell *sc = GlobalSettings::instance()->model()->saplings()->cell(offset, true, &ru);
            // Look for an unoccupied slot. If the species is already there, do not add a second cohort.
            if (sc && !sc->saplingOfSpecies(species->index())) {
                if (SaplingTree *st=sc->addSapling(static_cast<float>(height), static_cast<int>(age), species->index())) {
                    hits+=std::max(1., ru->resourceUnitSpecies(st->species_index)->species()->saplingGrowthParameters().representedStemNumberH(st->height));
                    ++n_added;
                } else {
                    if (++misses > pxcount)
                        break; // avoid an infinite loop
                }
            } else {
                if (++misses > pxcount)
                    break; // avoid an infinite loop
            }


        }
        total += pxcount;
        if (misses > pxcount)
            qDebug() << "tried to add" << pxcount << "saplings of species " << species->id() << "to stand" << stand_id << "but failed in finding enough free positions. Added" << n_added << "cohorts (Nrepr=" << hits << ") and stopped.";

    }

    // initialize grass cover
    if (init.columnIndex("grass_cover")>-1) {
        int grass_cover_value = init.value(low_index, "grass_cover").toInt();
        if (grass_cover_value<0 || grass_cover_value>100)
            throw IException(QString("The grass cover percentage (column 'grass_cover') for stand '%1' is '%2', which is invalid (expected: 0-100)").arg(stand_id).arg(grass_cover_value));
        GlobalSettings::instance()->model()->grassCover()->setInitialValues(lif_ptrs, grass_cover_value);
    }


    return total;
}
