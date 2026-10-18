#include <QtTest/QtTest>
#include <QtXml>


#include "xmlhelper.h"
#include "globalsettings.h"

class TestXmlHelper: public QObject
{
    Q_OBJECT
private:
    XmlHelper xml;
private slots:
    void initTestCase();
    void cleanupTestCase();

    void toUpper();
    void traverse();
    void dump();
    // file path
    void filepath();
};


void TestXmlHelper::initTestCase()
{
    //model = new Model();
    //model->loadProject();
    xml.loadFromFile("E:\\dev\\iland\\src\\tests\\testXmlHelper\\xmlHelperTest.xml");
}

void TestXmlHelper::cleanupTestCase()
{
    //delete model;
    //QTest::qWait(5000);
}

void TestXmlHelper::traverse()
{
    QVERIFY( !xml.node("").isNull() ); // top node
    QVERIFY( !xml.node("test.block.a").isNull() ); // traverse
    xml.setCurrentNode("test.block");
    QVERIFY( !xml.node(".b").isNull() );
    QVERIFY( !xml.node(".b.c").isNull() );
    QVERIFY (!xml.node("test.block.b.c").isNull() );
    QVERIFY (!xml.node(".b[0]").isNull() );
    QVERIFY (!xml.node(".b[0].d").isNull() );
    QVERIFY (!xml.node("test.block[1].n[2].o").isNull() );

    QCOMPARE( xml.value("test.block.b.c"), QString("c"));
    QCOMPARE( xml.value("test.block.b.c.nonexistent", "0"), QString("0"));
    QCOMPARE( xml.value("test.block[1].n[2].o"), QString("o"));
}

void TestXmlHelper::dump()
{
    qDebug() << xml.dump("test");
    qDebug() << xml.dump("path");
    qDebug() << xml.dump("species");
}

void TestXmlHelper::toUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

void TestXmlHelper::filepath()
{
    // setup file paths...
    xml.dump("path");
    GlobalSettings::instance()->setupDirectories(xml.node("path"));

}


QTEST_MAIN(TestXmlHelper)
#include "testXmlHelper.moc"


