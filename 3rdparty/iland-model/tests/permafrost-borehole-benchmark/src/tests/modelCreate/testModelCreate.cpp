#include <QtTest/QtTest>
#include <QtXml>
#include <QtSql>

#include "global.h"
#include "model.h"
#include "ressourceunit.h"
#include "xmlhelper.h"
#include "speciesset.h"
#include "species.h"
class TestModelCreate: public QObject
{
    Q_OBJECT
private:
    Model *model;
private slots:
    void initTestCase();
    void cleanupTestCase();

    void speciesFormula();
    void speciesFormulaStraight();
};


void TestModelCreate::initTestCase()
{
    //XmlHelper xml("E:\\dev\\iland\\src\\tests\\modelCreate\\test.xml");
    try {
        model = new Model();
        GlobalSettings::instance()->loadProjectFile("E:\\dev\\iland\\src\\tests\\modelCreate\\test.xml");
        model->loadProject();
    } catch (const IException &e) {
        qDebug() << "Exception:" << e.asString();
    }
}

void TestModelCreate::cleanupTestCase()
{
    delete model;
}

void TestModelCreate::speciesFormula()
{
    Species *species = model->ru()->speciesSet()->species("piab");
    QVERIFY(species!=0);
    // equation: m = 1.2*d^1.5
    QCOMPARE(qRound(species->biomassLeaf(2)),3);
    QCOMPARE(qRound(species->biomassLeaf(20)),107);
    QCOMPARE(qRound(species->biomassLeaf(50)),424);
    QCOMPARE(qRound(species->biomassLeaf(100)),1200);

    double x, y;
    y=56.;
    QBENCHMARK {
             x = species->biomassLeaf(y);
    }

}
void TestModelCreate::speciesFormulaStraight()
{
    double x,y=56.;
    QBENCHMARK {
             x = 1.2 * pow(y, 1.5);
    }
}


QTEST_MAIN(TestModelCreate)
#include "testModelCreate.moc"


