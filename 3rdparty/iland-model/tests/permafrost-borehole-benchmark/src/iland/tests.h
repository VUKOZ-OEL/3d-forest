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

#ifndef TESTS_H
#define TESTS_H
#include <QString>
#include <QObject>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif


class Climate;
class Tests
{
public:
    Tests(QObject *wnd);
    void speedOfExpression();
    void clearTrees();
    void killTrees();
    // climate
    void climate();
    void climateResponse();
    // light based tests for multiple stands
    void multipleLightRuns(const QString &fileName);
    void testWater();
    void testCSVFile();
    void testXml();
    void testRandom();
    void testSeedDispersal();
    void testMultithreadExecute();
    void testLinearExpressions();
    void testEstablishment();
    void testGridRunner();
    void testSoil();
    void testMap();
    void testDEM();
    void testFire();
    void testWind();
    void testRumple();
    void testFOMEsetup();
    void testFOMEstep();
    void testDbgEstablishment();
    void testGridIndexHack();
    private:
    QString dumpTreeList();
    QObject *mParent;
 private:
    void testSun();
    void testPheno(const Climate *clim);


};

#endif // TESTS_H
