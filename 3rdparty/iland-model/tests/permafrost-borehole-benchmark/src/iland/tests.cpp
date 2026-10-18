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
#include "tests.h"

#include "helper.h"
#include "debugtimer.h"
#include "random.h"
#include "model.h"
#include "resourceunit.h"
#include "expressionwrapper.h"
#include "expression.h"
///
#include "climate.h"
#include "species.h"
#include "speciesresponse.h"
#include "watercycle.h"
#include "csvfile.h"
#include "xmlhelper.h"
#include "environment.h"
#include "exception.h"
#include "seeddispersal.h"
#include "establishment.h"
#include "saplings.h"
//
#include "standloader.h"
#include "soil.h"

#include "mapgrid.h"
#include "management.h"
#include "dem.h"
#include "modelcontroller.h"

#include "modules.h"
#include "../plugins/fire/fireplugin.h"
#include "../plugins/fire/firemodule.h"
#include "../plugins/wind/windmodule.h"
#include "../plugins/wind/windplugin.h"
#include <QInputDialog>
#include <QtConcurrent/QtConcurrent>

#include "spatialanalysis.h"

#include "forestmanagementengine.h"
#include "fmstp.h"

Tests::Tests(QObject *wnd)
{
    //QObject::connect(this, SIGNAL(needRepaint),wnd, SLOT(repaint));
    mParent=wnd;

}


void Tests::testXml()
{
    XmlHelper xml( GlobalSettings::instance()->settings().node("model.species") );
    qDebug() << xml.value("source");
    QDomElement e = xml.node("source");
    xml.setNodeValue(e, "chief wiggum");
    qDebug() <<  xml.value("source"); // should print chief wiggum
    xml.setNodeValue("model.species.source","species");
    qDebug() << "and back:" << xml.value("source");

    // Test of environment
    Environment environment;
    environment.loadFromFile(GlobalSettings::instance()->path("envtest.txt"));
    try {
        environment.setPosition(QPointF(34,10)); // 0/0
        environment.setPosition(QPointF(150,10)); // 1/0
        environment.setPosition(QPointF(250,10)); // 2/0
        Climate *c = environment.climate();
        SpeciesSet *s = environment.speciesSet();
        qDebug() << s->count();
        qDebug() << c->daylength_h(160);
        environment.setPosition(QPointF(20,200)); // 0/2
        environment.setPosition(QPointF(-34,10)); // exception
    } catch (const IException &ex) {
        qDebug() << ex.message();
    }
}

void Tests::speedOfExpression()
{
    Q_ASSERT(1==0);
//    int *p;
//    p = 0;
//    *p = 1; --> signal handler does not really work
    // (1) for each
    double sum;
    int count;
    {
        DebugTimer t("plain loop");
        for (int i=0;i<10;i++) {
            sum=0;
            count=0;
            foreach(const ResourceUnit *ru,GlobalSettings::instance()->model()->ruList()) {
                foreach(const Tree &tree, ru->constTrees()) {
                    sum+=tree.volume();
                    count++;
                }
            }

        }
        qDebug() << "Sum of volume" << sum << "count" << count;
    }
    {
        DebugTimer t("plain loop (iterator)");
        for (int i=0;i<10;i++) {
            AllTreeIterator at(GlobalSettings::instance()->model());
            sum = 0.;
            count = 0;
            while (Tree *tree=at.next()) {
                sum += pow(tree->dbh(),2.1f); count++;
            }
        }
        qDebug() << "Sum of volume" << sum << "count" << count;
    }
    {
        TreeWrapper tw;
        Expression expr("dbh^2.1", &tw);
        DebugTimer t("Expression loop");
        for (int i=0;i<10;i++) {

            AllTreeIterator at(GlobalSettings::instance()->model());
            sum = 0.;
            while (Tree *tree=at.next()) {
                tw.setTree(tree);
                sum += expr.execute();
            }
        }
        qDebug() << "Sum of volume" << sum;
    }
}


void Tests::clearTrees()
{

    ResourceUnit *ru = GlobalSettings::instance()->model()->ru();
    int tc = ru->trees().count();
    // kill n percent...
    ru->trees().last().die();
    ru->trees().first().die();
    if (tc>20) {
        ru->trees()[15].die();
        ru->trees()[16].die();
        ru->trees()[17].die();
        ru->trees()[18].die();
        ru->trees()[19].die();
        ru->trees()[20].die();
    }
    qDebug() << "killed 8 trees";
    ru->cleanTreeList();
    ru->cleanTreeList();
}

void Tests::killTrees()
{
    AllTreeIterator at(GlobalSettings::instance()->model());
    int count=0, totalcount=0, idc=0;
    while (Tree *t = at.next()) {
        totalcount++;
        if (drandom() < 0.20) {
            t->die();
            count++;
        }
    }

    qDebug() << "killed" << count << "of" << totalcount << "left:" << totalcount-count;
    { DebugTimer t("count living");
      at.reset();
      count=0;
      idc=0;
      while (at.nextLiving()) {
          count++; idc+=at.current()->id();
          //qDebug() << at.current()->id();
      }
    }
    qDebug() << count << "living trees idsum" << idc;

    {
    DebugTimer t("clear trees");
    foreach(ResourceUnit *ru,GlobalSettings::instance()->model()->ruList())
        ru->cleanTreeList();
    }
    // count all trees
    at.reset();
    count=0;
    idc=0;
    while (at.next())
    {
        count++; idc+=at.current()->id();
        //qDebug() << (*at)->id();
    }
    qDebug() << count << "trees left idsum" << idc;
}

void Tests::climate()
{
    Climate clim;
    try {
    clim.setup();
    DebugTimer t("climate 100yrs");
    const ClimateDay *begin, *end;
    int mon=0;
    for (int i=0;i<100;i++) {
        clim.monthRange(mon, &begin, &end);
        clim.nextYear();
        mon=(mon+1)%12;
    }
    } catch (IException &e) {
        Helper::msg(e.message());
    }
}


void Tests::testSun()
{
    // solar radiation
    Sun sun;
    sun.setup(RAD(47));
    qDebug()<<sun.dump();
    sun.setup(RAD(70));
    qDebug()<<sun.dump();
    sun.setup(RAD(-20));
    qDebug()<<sun.dump();
}

void Tests::testWater()
{
    Model *model = GlobalSettings::instance()->model();
    model->createStandStatistics(); // force this!
    WaterCycle wc;
    wc.setup(model->ru());
    wc.run();
    for (int i=0;i<12;i++)
        qDebug() << wc.referenceEvapotranspiration()[i];
}

void Tests::testPheno(const Climate *clim)
{
    Phenology pheno(1,clim,0.9, 4.1, 10., 11., 2. ,9.);
    pheno.calculate();
    qDebug() << "Phenology is the key:";
    for (int i=0;i<12;i++)
        qDebug() << i << pheno.month()[i];
}

void Tests::climateResponse()
{

    try {

    DebugTimer t("climate Responses");

    // get a climate response object....
    Model *model = GlobalSettings::instance()->model();
    //model->ru()->climate()->setup(); // force setup

    ResourceUnitSpecies *rus = model->ru()->ruSpecies().first();

    rus->calculate();
    const SpeciesResponse *sr = rus->speciesResponse();
    QString line;
    for (int mon=0;mon<12;mon++) {
        line = QString("%1;%2").arg(mon)
               .arg(sr->tempResponse()[mon]);
        qDebug() << line;
    }
    // test nitrogen response
    line="Nitrogen response\n";
    for (double navailable=0.; navailable<100;navailable+=10) {
        for (double cls=1.; cls<=3.; cls+=0.2) {
            double res = model->ru()->speciesSet()->nitrogenResponse(navailable,cls);
            line+=QString("%1;%2;%3\n").arg(navailable).arg(cls).arg(res);
        }
    }
    qDebug() << line;

    line="CO2 response\n";
    for (double nresponse=0.; nresponse<=1;nresponse+=0.1) {
        for (double h2o=0.; h2o<=1; h2o+=0.1) {
            for (double co2=250; co2<600; co2+=50) {
                double res = model->ru()->speciesSet()->co2Response(co2, nresponse, h2o);
                line+=QString("%1;%2;%3;%4\n").arg(nresponse).arg(h2o).arg(co2).arg(res);
            }
        }
    }
    qDebug() << line;

    // sun
    //testSun();
    testPheno(model->ru()->climate());

    // test phenology
    } catch (IException &e) {
        Helper::msg(e.message());
    }
}

void Tests::multipleLightRuns(const QString &fileName)
{
    XmlHelper xml(fileName);
    Model *model = GlobalSettings::instance()->model();
    if (!model || !model->ru())
        return;
    QVector<Tree> &mTrees =  model->ru()->trees();

    QString outPath = xml.value("outputpath");
    QString inPath = xml.value("inputpath");
    QString inFile = xml.value("stands");
    qDebug() << "standlist:" << inFile << "inpath:"<<inPath << "save to:"<<outPath;
    QStringList fileList = Helper::loadTextFile(inFile).remove('\r').split('\n', QString::SkipEmptyParts);

    StandLoader loader(model);
    try {
        foreach (QString file, fileList) {
            file = inPath + "\\" + file;
            qDebug() << "processing" << file;
            Tree::resetStatistics();
            mTrees.clear();
            loader.loadPicusFile(file);
            // do a cycle...
            model->runYear();

            // create output file
            QFileInfo fi(file);
            QString outFileName = QString("%1\\out_%2.csv").arg(outPath, fi.baseName());
            Helper::saveToTextFile(outFileName, dumpTreeList() );
            qDebug() << mTrees.size() << "trees loaded, saved to" << outFileName;
            mParent->metaObject()->invokeMethod(mParent,"repaint");
            QApplication::processEvents();

        }
    } catch (IException &e) {
        Helper::msg(e.message());
    }
}

QString Tests::dumpTreeList()
{

    Model *model = GlobalSettings::instance()->model();

    AllTreeIterator at(model);
    DebugList treelist;
    QString line;
    QStringList result;
    result << "id;species;dbh;height;x;y;RU#;LRI;mWoody;mRoot;mFoliage;LA";

    while (Tree *tree = at.next()) {
        treelist.clear();
        tree->dumpList(treelist);
        line = "";
        foreach(QVariant value, treelist)
            line+=value.toString() + ";";
        *(line.end()-1)=' ';
        result << line;
    }

    QString resStr = result.join("\n");
    return resStr;
}

void Tests::testCSVFile()
{
    CSVFile file;
    file.loadFile("e:\\csvtest.txt");
    for (int row=0;row<file.rowCount(); row++)
        for (int col=0;col<file.colCount(); col++)
            qDebug() << "row col" << row << file.columnName(col) << "value" << file.value(row, col);
}

#include "../3rdparty/MersenneTwister.h"
void Tests::testRandom()
{

    QStringList list;
    for (int i=0;i<1000;i++)
        list << QString::number(irandom(0,5));
    qDebug() << "irandom test (0,5): " << list;
    return;


    RandomGenerator::setup(RandomGenerator::ergMersenneTwister, 1);
//    RandomCustomPDF pdf("x^2");
//    RandomCustomPDF *pdf2 = new RandomCustomPDF("x^3");
//    QStringList list;
//    for (int i=0;i<1000;i++)
//        list << QString::number(pdf.get());
//    qDebug() << list.join("\n");
//    delete pdf2;
//    // simple random test
//    list.clear();
//    for (int i=0;i<1000;i++)
//        list << QString::number(irandom(0,5));
//    qDebug() << "irandom test (0,5): " << list;

    // random generator timings
    { DebugTimer t("mersenne");
        RandomGenerator::setup(RandomGenerator::ergMersenneTwister, 0);
        int n8=0, n7=0, n6=0, n5=0, n4=0, n3=0;
        for (unsigned int i=0;i<4000000000U;i++) {
            double r = drandom();
            if (r< 0.00000001) ++n8;
            if (r< 0.0000001) ++n7;
            if (r< 0.000001) ++n6;
            if (r< 0.00001) ++n5;
            if (r< 0.0001) ++n4;
            if (r< 0.001) ++n3;
        }
        qDebug() << "Mersenne" << n3 << n4 << n5 << n6 << n7 << n8;
    }

    { DebugTimer t("mersenne");
        RandomGenerator::setup(RandomGenerator::ergMersenneTwister, 0);
        int n8=0, n7=0, n6=0, n5=0, n4=0, n3=0;
        for (int j=0;j<1000;j++) {
            RandomGenerator::seed(0); // new seed
            for (int i=0;i<4000000;i++) {
                double r = drandom();
                if (r< 0.00000001) ++n8;
                if (r< 0.0000001) ++n7;
                if (r< 0.000001) ++n6;
                if (r< 0.00001) ++n5;
                if (r< 0.0001) ++n4;
                if (r< 0.001) ++n3;
            }
        }
        qDebug() << "Mersenne2" << n3 << n4 << n5 << n6 << n7 << n8;
    }
    { DebugTimer t("XORShift");
        RandomGenerator::setup(RandomGenerator::ergXORShift96, 0);
        int n8=0, n7=0, n6=0, n5=0, n4=0, n3=0;
        for (unsigned int i=0;i<4000000000U;i++) {
            double r = drandom();
            if (r< 0.00000001) ++n8;
            if (r< 0.0000001) ++n7;
            if (r< 0.000001) ++n6;
            if (r< 0.00001) ++n5;
            if (r< 0.0001) ++n4;
            if (r< 0.001) ++n3;
        }
        qDebug() << "XORShift" << n3 << n4 << n5 << n6 << n7 << n8;
    }
    { DebugTimer t("WellRNG");
        RandomGenerator::setup(RandomGenerator::ergWellRNG512, 0);
        int n8=0, n7=0, n6=0, n5=0, n4=0, n3=0;
        for (unsigned int i=0;i<4000000000U;i++) {
            double r = drandom();
            if (r< 0.00000001) ++n8;
            if (r< 0.0000001) ++n7;
            if (r< 0.000001) ++n6;
            if (r< 0.00001) ++n5;
            if (r< 0.0001) ++n4;
            if (r< 0.001) ++n3;
        }
        qDebug() << "WellRNG" << n3 << n4 << n5 << n6 << n7 << n8;
    }
    { DebugTimer t("FastRandom");
        RandomGenerator::setup(RandomGenerator::ergFastRandom, 0);
        int n8=0, n7=0, n6=0, n5=0, n4=0, n3=0;
        for (unsigned int i=0;i<4000000000U;i++) {
            double r = drandom();
            if (r< 0.00000001) ++n8;
            if (r< 0.0000001) ++n7;
            if (r< 0.000001) ++n6;
            if (r< 0.00001) ++n5;
            if (r< 0.0001) ++n4;
            if (r< 0.001) ++n3;
        }
        qDebug() << "FastRandom" << n3 << n4 << n5 << n6 << n7 << n8;
    }
    { DebugTimer t("MersenneTwister - Reference");
        MTRand rand;
        rand.seed(1);

        double sum = 0;
        for (int i=0;i<100000000;i++) {
            if (drandom()<0.0000001) ++sum;
        }
        qDebug() << "Mersenne Reference" << sum;
    }
}

void Tests::testGridRunner()
{
    Grid<int> test_grid(10,5,5); // 5x5 grid
    test_grid.initialize(1);
    GridRunner<int> r(&test_grid);
    int sum = 0;
    while (int *p=r.next())
        ++sum;
    qDebug() << "runner all, expected: 25:" << sum;
    sum=0;
    for (int *p=test_grid.begin();p!=test_grid.end();++p)
        ++sum;
    qDebug() << "loop expected: 25:" << sum;
    sum=0;
    r=GridRunner<int>(test_grid,test_grid.rectangle());
    sum = 0;
    while (int *p=r.next())
        ++sum;
    qDebug() << "runner rectangle, expected: 25:" << sum;

    r=GridRunner<int>(test_grid,test_grid.metricRect());
    sum = 0;
    while (int *p=r.next())
        ++sum;
    qDebug() << "runner rectangle metric, expected: 25:" << sum;

    // smaller parts
    QRect r1(0,0,3,3);
    r=GridRunner<int>(test_grid,r1);
    sum = 0;
    while (int *p=r.next()) {
        ++sum;
        qDebug() << r.currentIndex();
    }
    qDebug() << "runner QRect(0,0,3,3), expected: 4:" << sum;


    r1=QRect(0,0,1,1);
    r=GridRunner<int>(test_grid,r1);
    sum = 0;
    while (int *p=r.next()) {
        ++sum;
        qDebug() << r.currentIndex();
    }
    qDebug() << "runner QRect(0,0,1,1), expected: 1:" << sum;

    r1=QRect(0,0,2,2);
    r=GridRunner<int>(test_grid,r1);
    sum = 0;
    while (int *p=r.next()) {
        ++sum;
        qDebug() << r.currentIndex();
    }
    qDebug() << "runner QRect(0,0,2,2), expected: 1:" << sum;

    r=GridRunner<int>(test_grid,QRect(QPoint(0,0), QPoint(3,3)));
    sum = 0;
    while (int *p=r.next()) {
        ++sum;
        qDebug() << r.currentIndex();
    }
    qDebug() << "runner QRect(QPoint(0,0), QPoint(3,3)), expected: 4:" << sum;

    r=GridRunner<int>(test_grid,QRect(QPoint(0,0), QPoint(0,0)));
    sum = 0;
    while (int *p=r.next()) {
        ++sum;
        qDebug() << r.currentIndex();
    }
    qDebug() << "runner QRect(QPoint(0,0), QPoint(0,0)), expected: 1:" << sum;

    r=GridRunner<int>(test_grid,QRect(QPoint(0,0), QPoint(1,1)));
    sum = 0;
    while (int *p=r.next()) {
        ++sum;
        qDebug() << r.currentIndex();
    }
    qDebug() << "runner QRect(QPoint(0,0), QPoint(1,1)), expected: 1:" << sum;


    if (!GlobalSettings::instance()->model())
        return;

    Grid<float> &lif = *GlobalSettings::instance()->model()->grid();
    //QRectF box = GlobalSettings::instance()->model()->ru(0)->boundingBox();
    QRectF box2 = QRectF(10,10,10,10);
    GridRunner<float> runner(lif, box2);
    int i=0;
    while (float *p=runner.next()) {
        QPoint point = lif.indexOf(p);
        qDebug() << i++ << point.x() << point.y() << *p << p;
    }
    QRect index_rect = QRect(lif.indexAt(QPointF(10., 10.)), lif.indexAt(QPointF(20., 20)));
    GridRunner<float> runner2(lif, index_rect);
    qDebug() << "index variables for rect" << index_rect;
    i = 0;
    while (float *p=runner2.next()) {
        QPoint point = lif.indexOf(p);
        qDebug() << i++ << point.x() << point.y() << *p << p;
    }
    index_rect = QRect(0,0,1,1);
    runner2 = GridRunner<float>(lif, index_rect);
    qDebug() << "test index: rect" << index_rect;
    i = 0;
    while (float *p=runner2.next()) {
        QPoint point = lif.indexOf(p);
        qDebug() << i++ << point.x() << point.y() << *p << p;
    }

    index_rect = QRect(0,0,3,3);
    runner2 = GridRunner<float>(lif, index_rect);
    qDebug() << "test index: rect" << index_rect;
    i = 0;
    while (float *p=runner2.next()) {
        QPoint point = lif.indexOf(p);
        qDebug() << i++ << point.x() << point.y() << *p << p;
    }

    for (int i=0;i<GlobalSettings::instance()->model()->ruList().size();++i) {
        if (i==10) break;
        ResourceUnit *ru = GlobalSettings::instance()->model()->ruList()[i];
        GridRunner<float> runner(lif, ru->boundingBox());
        int n=0;
        while (runner.next())
            ++n;
        qDebug() << "RU" <<  ru->index() << ru->boundingBox() << lif.indexOf(runner.first()) << lif.indexOf(runner.last()) << "n:" << n;
    }
}

void Tests::testSeedDispersal()
{
    SeedDispersal sd;
    sd.setup();
    sd.loadFromImage("e:\\temp\\test.jpg");
    QImage img = gridToImage(sd.seedMap(), true, -1., 1.);
    img.save("e:\\temp\\seedmap.png");
    sd.execute();
//    sd.edgeDetection();
//    gridToImage(sd.seedMap(), true, -1., 1.).save("seedmap_edge.png");
//    sd.distribute();
    img = gridToImage(sd.seedMap(), true, -1., 1.);
    img.save("e:\\temp\\seedmap_e.png");

}

Expression tme_exp;
int tme_count;
double tme_test1(const double &x) {
    double result = tme_exp.calculate(x);
    if (result!=1)
        tme_count++;
    return result;
}
double tme_test2(const double &x) {
    tme_exp.setVar("x",x);
    double result = tme_exp.execute();
    if (result!=1)
        tme_count++;
    return result;
}
QMutex tme_mutex;
double tme_test3(const double &x) {
    QMutexLocker l(&tme_mutex);
    tme_exp.setVar("x",x);
    double result = tme_exp.execute();
    if (result!=1)
        tme_count++;
    return result;
}

static double testf_sum = 0.;
void testF(double *begin, double *end) {
    double sum = 0.;
    for (double *p=begin;p!=end;++p) {
        sum += *p;
    }
    QMutexLocker l(&tme_mutex);
    testf_sum += sum;
    //qDebug() << "testf min:" << *begin << "end:" << *(end-1);
}

void Tests::testMultithreadExecute()
{
    // test of threadrunner
    int l=12345;
    double *dat=new double[l];
    for (int i=0;i<l;++i)
        dat[i] = i+1;
    ThreadRunner tr;
    tr.runGrid(testF, dat, &dat[l], true, 100);
    qDebug() << "sum" << testf_sum << l*(l+1)/2. - testf_sum;

    testf_sum = 0.;
    tr.runGrid(testF, dat, &dat[l], true, 17);
    qDebug() << "sum" << testf_sum << l*(l+1)/2. - testf_sum;

    testf_sum = 0.;
    tr.runGrid(testF, dat, &dat[l], true, 170000);
    qDebug() << "sum" << testf_sum << l*(l+1)/2. - testf_sum;

    testf_sum = 0.;
    tr.runGrid(testF, dat, &dat[l], true);
    qDebug() << "sum" << testf_sum << l*(l+1)/2. - testf_sum;
    return;

    tme_exp.setExpression("(x+x+x+x)/(sqrt(x*x)*4)");
    tme_exp.setStrict(false);
    try {
        tme_exp.parse();
    } catch(const IException &e) {
        QString error_msg = e.message();
        Helper::msg(error_msg);
        qDebug() << error_msg;
    }
    QVector<double> data;
    for (int i=1;i<1000000;i++)
        data.push_back(nrandom(1,2000));
    qDebug() << "test multiple threads, part 1 (thread safe):";
    tme_count = 0;
    { DebugTimer t("threadsafe");
        QtConcurrent::blockingMap(data,(*tme_test1)); }
    qDebug() << "finished: errors: " << tme_count;

    tme_exp.setStrict(false);
    tme_exp.addVar("x");
    try {
        tme_exp.parse();
    } catch(const IException &e) {
        QString error_msg = e.message();
        Helper::msg(error_msg);
        qDebug() << error_msg;
    }
    qDebug() << "test multiple threads, part 2 (non thread safe):";
    tme_count = 0;
    { DebugTimer t("no_thread");
    QtConcurrent::blockingMap(data,(*tme_test2));
    }
    qDebug() << "finished: errors: " << tme_count;

    qDebug() << "test multiple threads, part 3 (mutex locker):";
    tme_count = 0;
    { DebugTimer t("mutex");
        QtConcurrent::blockingMap(data,(*tme_test3));
    }
    qDebug() << "finished: errors: " << tme_count;
}

void Tests::testEstablishment()
{
    Model *model = GlobalSettings::instance()->model();
    //model->saplings()->clearStats();

    {
    DebugTimer test("test establishment");
    foreach (ResourceUnit *ru, model->ruList())
        model->saplings()->establishment(ru);
    }
    //qDebug() << "pixel tested" << model->saplings()->pixelTested() << "saplings added" << model->saplings()->saplingsAdded();

    {
    DebugTimer test("test sapling growth");
    foreach (ResourceUnit *ru, model->ruList())
        model->saplings()->saplingGrowth(ru);
    }
    //qDebug() << "pixel tested" << model->saplings()->pixelTested() << "saplings added" << model->saplings()->saplingsAdded();

    //model->ru(0)
    //Establishment est(model->ru(0)->climate(),model->ru(0)->ruSpecies().first());
    //est.calculate();
}

void Tests::testLinearExpressions()
{
    Expression::setLinearizationEnabled(true); // enable
    Expression a("40*(1-(1-(x/40)^(1/3))*exp(-0.03))^3"); // test function: sapling growth
    Expression b("40*(1-(1-(x/40)^(1/3))*exp(-0.03))^3");

    b.linearize(2., 140.);
    b.calculate(58.);
    for (double x=5.; x<150.; x++) {
        double ra = a.calculate(x);
        double rb = b.calculate(x);
        qDebug() << x << ra << rb << rb-ra;
    }
    // test b
    { DebugTimer t("straight");
        for (int i=0;i<1000000;i++) {
            a.calculate(3.4);
        }
    }
    { DebugTimer t("linear");
        for (int i=0;i<1000000;i++) {
            b.calculate(3.4);
        }
    }
    { DebugTimer t("nativ");
        double x=3;
        double y=0;
        for (int i=0;i<1000000;i++) {
           y+= 40.*pow(1.-(1.-pow((x/40.),1./3.))*exp(-0.03),3.);
        }
        qDebug() << y;
    }
    /// test matrix-case
//    Expression c("x*x+exp(y*y)");
//    Expression d("x*x+exp(y*y)");
    Expression c("exp(2*ln(x)*(1-y/2))");
    Expression d("exp(2*ln(x)*(1-y/2))");
    d.linearize2d(0., 1., 0., 1.);
    qDebug() << d.calculate(0, 0.8);
    for (double x=0.; x<1.; x+=0.1) {
        for (double y=0.; y<1.; y+=0.1) {
            double ra = c.calculate(x,y);
            double rb = d.calculate(x,y);
            qDebug() << x << y << ra << rb << rb-ra;
        }
    }
    qDebug() << "random test";
    for (int i=0;i<100;i++) {
        double x = drandom();
        double y = drandom();
        double ra = c.calculate(x,y);
        double rb = d.calculate(x,y);
        qDebug() << x << y << ra << rb << rb-ra;

    }
    qDebug() << "performance";
    { DebugTimer t("straight");
        for (int i=0;i<1000000;i++) {
            c.calculate(0.33, 0.454);
        }
    }
    { DebugTimer t("linear");
        for (int i=0;i<1000000;i++) {
            d.calculate(0.33, 0.454);
        }
    }
}

void Tests::testSoil()
{
//    Y0l=33 # young C labile, i.e. litter
//    Y0Nl=0.45 # young N labile
//    Y0r=247 # young C refractory , i.e. dwd
//    Y0Nr=0.99 # young N refractory
//    O0=375 # old C, i.e soil organic matter (SOM)
//    O0N=15 # old N
//    kyl=0.15 # litter decomposition rate
//    kyr=0.0807 # downed woody debris (dwd) decomposition rate
    Soil soil;
    qDebug() << "soil object has" << sizeof(soil) << "bytes.";
    // values from RS R script script_snagsoil_v5.r
    soil.setInitialState(CNPool(33000., 450., 0.15), // young lab
                         CNPool(247000., 990., 0.0807), // young ref
                         CNPair(375000., 15000.), 0.5, 0.5); // SOM
    CNPool in_l(3040, 3040/75., 0.15);
    CNPool in_r(11970, 11970./250., 0.0807);
    double re = 1.1;

    QStringList out;
    out << "year;iLabC;iLabN;ikyl;iRefC;iRefN;ikyr;RE;kyl;kyr;ylC;ylN;yrC;yrN;somC;somN;NAvailable";

    for (int i=0;i<100;i++) {
        // run the soil model
        soil.setClimateFactor(re);
        if (i==1) {
           soil.setSoilInput(in_l*5., in_r*5., 0.5, 0.5); // pulse in year 2
        } else if (i>1 && i<10) {
            soil.setSoilInput(CNPool(), CNPool(), 0.5, 0.5); // no input for years 3..10
        } else{
            soil.setSoilInput(in_l, in_r, 0.5, 0.5); // normal input
        }

        soil.calculateYear();
        QList<QVariant> list = soil.debugList();
        QString line=QString::number(i)+";";
        foreach(QVariant v, list)
            line+=v.toString() + ";";
        line.chop(1);
        out << line;
    }
    if (fabs(soil.availableNitrogen() - 77.2196456179288) < 0.000001)
        qDebug() << "PASSED.";
    else
        qDebug() << "ERROR";
    qDebug() << "ICBM/2N run: saved to e:/soil.txt";
    //qDebug() << out;
    Helper::saveToTextFile("e:/soil.txt", out.join("\r\n"));

    qDebug()<< "test weighting of CNPool (C/N/rate):";
    CNPool c(1,1,1);
    for (int i=0;i<10;i++) {
        c.addBiomass(1,1,2);
        qDebug() << c.C << c.N << c.parameter();
    }

    out.clear();
    out << "year;iLabC;iLabN;ikyl;iRefC;iRefN;ikyr;RE;kyl;kyr;ylC;ylN;yrC;yrN;somC;somN;NAvailable";
    // test soil with debug output from iLand ...
    CSVFile dbg_iland(GlobalSettings::instance()->path("debug_carboncycle.csv"));
    Soil *model_soil = GlobalSettings::instance()->model()->ru()->soil(); // should now have the 'right' parameters...

    model_soil->setInitialState(CNPool(dbg_iland.value(0, dbg_iland.columnIndex("ylC")).toDouble()*1000.,
                                dbg_iland.value(0, dbg_iland.columnIndex("ylN")).toDouble()*1000.,
                                dbg_iland.value(0, dbg_iland.columnIndex("kyl")).toDouble()), // young lab
                         CNPool(dbg_iland.value(0, dbg_iland.columnIndex("yrC")).toDouble()*1000.,
                                dbg_iland.value(0, dbg_iland.columnIndex("yrN")).toDouble()*1000.,
                                dbg_iland.value(0, dbg_iland.columnIndex("kyr")).toDouble()), // young ref
                         CNPair(dbg_iland.value(0, dbg_iland.columnIndex("somC")).toDouble()*1000.,
                                dbg_iland.value(0, dbg_iland.columnIndex("somN")).toDouble()*1000.), 0.5, 0.5); // SOM

    for (int i=1;i<dbg_iland.rowCount();i++) {
        // run the soil model
        model_soil->setClimateFactor(dbg_iland.value(i, dbg_iland.columnIndex("re")).toDouble());
        model_soil->setSoilInput(CNPool(dbg_iland.value(i, dbg_iland.columnIndex("iLabC")).toDouble()*1000.,
                                 dbg_iland.value(i, dbg_iland.columnIndex("iLabN")).toDouble()*1000.,
                                 dbg_iland.value(i, dbg_iland.columnIndex("iKyl")).toDouble()),
                          CNPool(dbg_iland.value(i, dbg_iland.columnIndex("iRefC")).toDouble()*1000.,
                                 dbg_iland.value(i, dbg_iland.columnIndex("iRefN")).toDouble()*1000.,
                                 dbg_iland.value(i, dbg_iland.columnIndex("iKyr")).toDouble()), 0.5, 0.5);
        model_soil->calculateYear();
        QList<QVariant> list = model_soil->debugList();
        QString line=QString::number(i)+";";
        foreach(QVariant v, list)
            line+=v.toString() + ";";
        line.chop(1);
        out << line;
    }
    Helper::saveToTextFile("e:/soil2.txt", out.join("\r\n"));
}

void Tests::testMap()
{
    QString fileName = GlobalSettings::instance()->path("gis/montafon_grid.txt");

    int test_id = 127;
    // test the map-grid class
    MapGrid map(fileName);
    QList<ResourceUnit*> ru_list = map.resourceUnits(test_id);
    qDebug() << "total bounding box" << map.boundingBox(test_id) << "has an area of" << map.area(test_id);
    foreach (const ResourceUnit *ru, ru_list)
        qDebug() << ru->index() << ru->boundingBox();

    qDebug() << "neighbors of" << test_id << "are" << map.neighborsOf(test_id);
    return;
    HeightGrid *hgrid = GlobalSettings::instance()->model()->heightGrid();
    for (int i=0;i<map.grid().count();i++)
        hgrid->valueAtIndex(i).height = map.grid().constValueAtIndex(map.grid().indexOf(i));

    QList<Tree*> tree_list = map.trees(test_id);
    qDebug() << "no of trees:" << tree_list.count();
    Management mgmt;

    mgmt.loadFromTreeList(tree_list);
    mgmt.killAll();
}

DEM *_dem = 0;
void Tests::testDEM()
{
    QString fileName = GlobalSettings::instance()->path("gis/dtm1m_clip.txt");

    if (_dem) {
        int choice = QInputDialog::getInt(0, "enter type", "Type to show: 0: dem, 1: slope, 2: aspect, 3: view, 4: exit");
        switch (choice) {
        case 0: GlobalSettings::instance()->controller()->addGrid(_dem, "dem height", GridViewRainbow, 0, 1000); break;
        case 1: GlobalSettings::instance()->controller()->addGrid(_dem->slopeGrid(), "slope", GridViewRainbow, 0, 3); break;
        case 2: GlobalSettings::instance()->controller()->addGrid(_dem->aspectGrid(), "aspect", GridViewRainbow, 0, 360); break;
        case 3: GlobalSettings::instance()->controller()->addGrid(_dem->viewGrid(), "dem", GridViewGray, 0, 1); break;
        default: return;
        }
        return;
    }

    try {
        if (!_dem)
            _dem = new DEM(fileName);

        qDebug() << "slope grid: avg max" << _dem->slopeGrid()->avg()  << _dem->slopeGrid()->max();
        qDebug() << "aspect grid: avg max" << _dem->aspectGrid()->avg()  << _dem->aspectGrid()->max();
        qDebug() << "view grid: avg max" << _dem->viewGrid()->avg()  << _dem->viewGrid()->max();

        // note: dem gets not released! (to be still there when painting happens)
        float slope, aspect;
        for (float y=0.;y<1000.;y+=100.)
            for (float x=0.;x<1000.;x+=100.) {
                float h = _dem->orientation(x, y, slope, aspect );
                qDebug() << "at point "<< x << y << "height"<< h  <<"slope" << slope << "aspect" << aspect;
            }

    } catch (IException &e) {
        Helper::msg(e.message());
    }
    GlobalSettings::instance()->controller()->addGrid(_dem, "dem height", GridViewRainbow, 0, 1000);
    GlobalSettings::instance()->controller()->addGrid(_dem->slopeGrid(), "slope", GridViewRainbow, 0, 3);
    GlobalSettings::instance()->controller()->addGrid(_dem->aspectGrid(), "aspect", GridViewRainbow, 0, 360);
    GlobalSettings::instance()->controller()->addGrid(_dem->viewGrid(), "dem", GridViewGray, 0, 1);


}

void Tests::testFire()
{
    // get fire module
    FirePlugin *plugin =dynamic_cast<FirePlugin *>(GlobalSettings::instance()->model()->modules()->module("fire"));
    if (plugin) {
        FireModule *fire = plugin->fireModule();
        try {
            fire->testSpread();
        } catch (const IException &e) {
            Helper::msg(e.message());
        }
    }
    GlobalSettings::instance()->controller()->repaint();

}

void Tests::testWind()
{
    // get fire module
    WindPlugin *plugin = dynamic_cast<WindPlugin *>(GlobalSettings::instance()->model()->modules()->module("wind"));
    if (plugin) {
        static int iteration = 0;
        WindModule *wind = plugin->windModule();
        if (iteration>0) {
            if (Helper::question("next iteration (yes) or stop (no)?")) {
                qDebug() << ".... iteration ..." << iteration+1;
                wind->run(iteration++);
                return;
            } else {
                iteration = 0;
            }
        }
        double direction = Helper::userValue("direction (degrees), -1 for roundabout", "100").toDouble();
        bool loop = false;
        if (direction==-1)
            loop = true;
        try {
            if (loop) {
                for (direction = 0.; direction<360; direction+=10.) {
                    wind->run();
                    wind->testFetch(direction);
                    GlobalSettings::instance()->controller()->repaint();
                    qDebug() << "looping..." << direction;
                }
            } else {
                DebugTimer t;

                if (direction!=0)
                    wind->setSimulationMode(true);
                double speed = Helper::userValue("wind speed (m/2)", "300").toDouble();
                wind->setWindProperties(direction * M_PI / 180., speed);
                wind->run(iteration++);

                qDebug() << "fetch finished. ms:" << t.elapsed();
            }
        } catch (const IException &e) {
            Helper::msg(e.message());
        }

    }
    GlobalSettings::instance()->controller()->repaint();
}

void Tests::testRumple()
{
    RumpleIndex rumple_index;
    double result = rumple_index.value();
    qDebug() << "rumple index test: " << result;

    Helper::saveToTextFile("rumple_test.txt", gridToESRIRaster(rumple_index.rumpleGrid()) );
    qDebug() << "surface area test triangle" << rumple_index.test_triangle_area();

}

ABE::ForestManagementEngine *fome=0;
void Tests::testFOMEsetup()
{

    fome = GlobalSettings::instance()->model()->ABEngine();
    if (!fome)
        fome = new ABE::ForestManagementEngine();
    //fome.test();
    try {

        ABE::FMSTP::setVerbose(true);
        fome->setup();
    } catch(const IException &e) {
       Helper::msg(e.message());
    }
    // todo: re-enable delete!!!
    // delete fome;
}

void Tests::testFOMEstep()
{
    if (!fome)
        return;
    int n = Helper::userValue("how many years?", "1").toInt();
    for (int i=0;i<n;++i) {
        qDebug()<< "running ABE year" << i;
        fome->run(1);
    }
}

void Tests::testDbgEstablishment()
{
    // test speeed of debugtimer
    DebugTimer::clearAllTimers();
    DebugTimer total("total");
    // 4x1,000,000 timers: ca 1.5 seks...
    for (int i=0;i<100000;i++) {
        { DebugTimer a("aasdf asdfasdf"); }
        { DebugTimer a("basdflk asdf"); }
        { DebugTimer a("bas asdfasdf"); }
        { DebugTimer a("dasdflkj asdfalskfj"); }
    }

    qDebug() << "finsihed timers";

//    const Grid<float> &seed_map = GlobalSettings::instance()->model()->speciesSet()->activeSpecies()[0]->seedDispersal()->seedMap();

//    int n_established = 0;
//    int n_tested = 0, n_seed=0, n_dropped=0;
//    double mPAbiotic = 0.8;
//    DebugTimer runt("run est");
//    int dbg_numbers = RandomGenerator::debugNRandomNumbers();
//    // define a height map for the current resource unit on the stack
//    float sapling_map[cPxPerRU*cPxPerRU];
    // set the map and initialize it:

//    for (int i=0;i<100;i++)
//        foreach ( ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {

//             ru->setSaplingHeightMap(sapling_map);
//             ResourceUnitSpecies *mRUS = ru->ruSpecies()[0];

//            const QRectF &ru_rect = ru->boundingBox();

//            // test the establishment code....
//            GridRunner<float> seed_runner(seed_map, ru_rect);
//            Grid<float> *lif_map = GlobalSettings::instance()->model()->grid();

//            // check every pixel inside the bounding box of the pixel with
//            while (float *p = seed_runner.next()) {
//                ++n_tested;
//                if (*p>0.f) {
//                    ++n_seed;
//                    //double p_establish = drandom(mRUS->ru()->randomGenerator());
//                    //if (p_establish > mPAbiotic)
//                    //    continue;
//                    // pixel with seeds: now really iterate over lif pixels
//                    GridRunner<float> lif_runner(lif_map, seed_map.cellRect(seed_runner.currentIndex()));
//                    while (float *lif_px = lif_runner.next()) {
//                        DBGMODE(
//                                    if (!ru_rect.contains(lif_map->cellCenterPoint(lif_map->indexOf(lif_px))))
//                                    qDebug() << "(b) establish problem:" << lif_map->indexOf(lif_px) << "point: " << lif_map->cellCenterPoint(lif_map->indexOf(lif_px)) << "not in" << ru_rect;
//                                );
//                        double p_establish = drandom();
//                        if (p_establish < mPAbiotic) {
//                            //if (establishTree(lif_map->indexOf(lif_px), *lif_px ,*p))
//                            QPoint pos_lif = lif_map->indexOf(lif_px);

//                            if (ru->saplingHeightAt(pos_lif) > 1.3f)
//                                ++n_dropped;

//                            // check if sapling of the current tree species is already established -> if so, no establishment.
//                            if (mRUS->hasSaplingAt(pos_lif))
//                                ++n_dropped;

//                            const HeightGridValue &hgv = GlobalSettings::instance()->model()->heightGrid()->constValueAtIndex(pos_lif.x()/cPxPerHeight, pos_lif.y()/cPxPerHeight);
//                            if (hgv.count()>0) n_dropped++;
//                            double h_height_grid = hgv.height;
//                            double rel_height = 4. / h_height_grid;

//                             double lif_corrected = mRUS->species()->speciesSet()->LRIcorrection(*lif_px, rel_height);
//                             if (lif_corrected < drandom())
//                                 ++n_dropped;


//                            n_established++;
//                        }
//                    }
//                }
//            }
//        }
//    qDebug() << "Orig tested " << n_tested << "with seed" << n_seed << "est: " << n_established << "time" << runt.elapsed() << "debug numbers used:" << RandomGenerator::debugNRandomNumbers()-dbg_numbers << "n_dropped" << n_dropped;

//    runt.start();
//    n_established = 0;
//    n_tested = 0; n_seed=0; n_dropped=0;
//    for (int i=0;i<100;i++)
//        foreach ( ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {

//             ru->setSaplingHeightMap(sapling_map);
//             ResourceUnitSpecies *mRUS = ru->ruSpecies()[0];
//             const std::bitset<cPxPerRU*cPxPerRU> &pos_bitset = mRUS->sapling().presentPositions();

//            const QRectF &ru_rect = ru->boundingBox();
//            //DebugTimer estasd("establish:from_search");
//            // 3rd step: check actual pixels in the LIF grid

//            // a large part has available seeds. simply scan the pixels...
//            QPoint lif_index;
//            Grid<float> *lif_map = GlobalSettings::instance()->model()->grid();
//            GridRunner<float> lif_runner(lif_map, ru_rect);
//            float *sap_height = sapling_map;
//            size_t bit_idx=0;
//            while (float *lif_px = lif_runner.next()) {
//                ++n_tested;
//                // check for height of sapling < 1.3m (for all species
//                // and for presence of a sapling of the given species
//                if (*sap_height < 1.3 && pos_bitset[bit_idx]==false) {
//                    ++n_seed;
//                    lif_index = lif_map->indexOf(lif_px);
//                    double sap_random_number = drandom();
//                    const float seed_map_value = seed_map.constValueAt( lif_runner.currentCoord() );

//                    if (seed_map_value*mPAbiotic > sap_random_number)
//                        ++n_dropped;


////                    DBGMODE(
////                                if (!ru_rect.contains(lif_map->cellCenterPoint(lif_index)))
////                                qDebug() << "(a) establish problem:" << lif_index << "point: " << lif_map->cellCenterPoint(lif_index) << "not in" << ru_rect;
////                            );


//                    const HeightGridValue &hgv = GlobalSettings::instance()->model()->heightGrid()->constValueAtIndex(lif_index.x()/cPxPerHeight, lif_index.y()/cPxPerHeight);

//                    double h_height_grid = hgv.height;
//                    double rel_height = 4. / h_height_grid;

//                     double lif_corrected = mRUS->species()->speciesSet()->LRIcorrection(*lif_px, rel_height);
//                     if (lif_corrected < sap_random_number)
//                         ++n_dropped;

//                     if (seed_map_value*mPAbiotic*lif_corrected < sap_random_number)
//                         n_established++;
//                }
//                ++sap_height;
//                ++bit_idx;
//            }


//        }
//    qDebug() << "Upd tested " << n_tested << "with seed" << n_seed << "est: " << n_established << "time" << runt.elapsed() << "debug numbers used:" << RandomGenerator::debugNRandomNumbers()-dbg_numbers << "n_dropped" << n_dropped;

//    DebugTimer::printAllTimers();
}

void Tests::testGridIndexHack()
{
    Grid<float> m2, m10;
    const int n=10000;
    m2.setup(2,n,n);
    m10.setup(10, n/5, n/5 );
    m10.wipe();

    for (float *f = m2.begin(), i=0.; f!=m2.end(); ++f, ++i)
        *f = i;

//    for (int i=0;i<m2.count();++i) {
//        qDebug() << i << m2.index5(i);
//    }
//    return;

    int el;
    { DebugTimer t("optimized");
        for (float *f = m2.begin(); f!=m2.end(); ++f) {
            m10[ m2.index5(f-m2.begin()) ] += *f;
        }
        el = t.elapsed();
    }

    float s=m10.sum() / m10.count();
    qDebug() << "test average value (new):" << s << "time" << el;

    m10.wipe();

    { DebugTimer t("old");
        for (float *f = m2.begin(); f!=m2.end(); ++f) {
            m10.valueAt(m2.cellCenterPoint(m2.indexOf(f))) += *f;
        }
        el = t.elapsed();
    }
    s=m10.sum() / m10.count();
    qDebug() << "test average value (old):" << s << "time" << el;

     int errors=0;
     { DebugTimer t("compare");
         for (float *f = m2.begin(); f!=m2.end(); ++f) {
             if ( m10.valueAtIndex( m2.index5(f-m2.begin()) ) != m10.valueAt(m2.cellCenterPoint(m2.indexOf(f))))
                     errors++;
         }
     }
      qDebug() << "test e:differenczes" << errors;

      { DebugTimer t("optimized");
          for (int i=0;i<m2.count();++i) {
              m10[ m2.index5(i)] += m2[i];
          }
          el = t.elapsed();
      }

      s=m10.sum() / m10.count();
      qDebug() << "test average value (square brackets):" << s << "time" << el;

}
