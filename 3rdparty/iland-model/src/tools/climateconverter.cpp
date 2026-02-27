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
#include "climateconverter.h"
#include "csvfile.h"

#include <QJSEngine>
#include <QJSValue>
#include <QtSql>

/** @class ClimateConverter
  Converts text-file-based data into the iLand climate data format.
  For the iLand format see the wiki (ClimateFormat). For each column (i.e. year,month, day,
  temp, prec, rad, vpd), an expression providing access to the columns of the input file calculates
  the respective output value. Propertes tableName and fileName define the input file and the
  name of the output table (located in the "climate"-database of iLand) respectively.
*/
/** @group script
  @class ClimateConverter
  This is the Scripting related documentation for the ClimateConverter tool.
*/
//Q_SCRIPT_DECLARE_QMETAOBJECT(ClimateConverter, QObject*)
void ClimateConverter::addToScriptEngine(QJSEngine &engine)
{
    QJSValue jsMetaObject = engine.newQMetaObject(&ClimateConverter::staticMetaObject);
    engine.globalObject().setProperty("ClimateConverter", jsMetaObject);
}

ClimateConverter::ClimateConverter(QObject *)
{
    mCaptions = true;
    bindExpression(mExpYear, 0);
    bindExpression(mExpMonth, 1);
    bindExpression(mExpDay, 2);

    bindExpression(mExpTemp, 3);
    bindExpression(mExpMinTemp, 4);

    bindExpression(mExpPrec, 5);
    bindExpression(mExpRad, 6);
    bindExpression(mExpVpd, 7);
}



void ClimateConverter::bindExpression(Expression &expr, int index)
{
    expr.setExpression(QString("c%1").arg(index) ); // "cX" is the default expression
    for (int i=0;i<10;i++)
        mVars[index*10 + i] = expr.addVar( QString("c%1").arg(i) );
}

void ClimateConverter::run()
{
    mExpYear.setExpression(mYear);
    mExpMonth.setExpression(mMonth);
    mExpDay.setExpression(mDay);

    mExpTemp.setExpression(mTemp);
    mExpMinTemp.setExpression(mMinTemp);
    mExpPrec.setExpression(mPrec);
    mExpRad.setExpression(mRad);
    mExpVpd.setExpression(mVpd);
    // prepare output database
    if (mDatabase.isEmpty()) {
        qDebug() << "ClimateConverter: database is empty!";
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","cctemp");
    db.setDatabaseName(mDatabase);
    if (!db.open()) {
        qDebug() << "ClimateConverter: invalid database: " << db.lastError().text();
        return;
    }

    QSqlQuery creator(db);
    if (mTableName.isEmpty()) {
        qDebug() << "ClimateConverter::run: invalid climate database or table name.";
    }
    QString sql=QString("CREATE TABLE %1 ( " \
                "year INTEGER, month INTEGER, day INTEGER, " \
                "temp REAL, min_temp REAL, prec REAL, rad REAL, vpd REAL)").arg(mTableName);

    QString drop=QString("drop table if exists %1").arg(mTableName);

    creator.exec(drop); // drop table (if exists)
    if (creator.lastError().isValid()) {
        qDebug() << "ClimateConverter: Sql-Error (drop):" << creator.lastError().text();
        return;
    }
    creator.exec(sql); // (re-)create table
    if (creator.lastError().isValid()) {
        qDebug() << "ClimateConverter: Sql-Error (create table):" << creator.lastError().text();
        return;
    }

    // prepare insert statement
    sql = QString("insert into %1 (year, month, day, temp, min_temp, prec, rad, vpd) values (?,?,?, ?,?,?,?,?)").arg(mTableName);
    creator.prepare(sql);
    if (creator.lastError().isValid()) {
        qDebug() << "ClimateConverter: Sql-Error (prepare):" << creator.lastError().text();
        return;
    }
    // load file
    if (mFileName.isEmpty()) {
        qDebug() << "ClimateConverter::run: empty filename.";
        return;
    }
    CSVFile file;
    file.setHasCaptions(mCaptions);
    file.loadFile(mFileName);
    if (!file.rowCount()) {
        qDebug() << "ClimateConverter::run: cannot load file:" << mFileName;
        return;
    }

    // do this for each row
    double value;
    int year, month, day;
    double temp, min_temp, prec, rad, vpd;
    int rows=0;
    db.transaction();
    for (int row=0;row<file.rowCount(); row++) {
        // fetch values from input file
        for (int col=0;col<file.colCount(); col++) {
            value = file.value(row, col).toDouble();
            // store value in each of the expression variables
            for (int j=0;j<8;j++)
                *(mVars[j*10 + col]) = value; // store in the locataion mVars[x] points to.
        }
        // calculate new values....
        year = (int)mExpYear.execute();
        month = (int)mExpMonth.execute();
        day = (int)mExpDay.execute();
        temp = mExpTemp.execute();
        min_temp = mExpMinTemp.execute();
        prec = mExpPrec.execute();
        rad = mExpRad.execute();
        vpd = mExpVpd.execute();
        //qDebug() << year << month << day << temp << prec << rad << vpd;
        // bind values
        creator.bindValue(0,year);
        creator.bindValue(1,month);
        creator.bindValue(2,day);
        creator.bindValue(3,temp);
        creator.bindValue(4,min_temp);
        creator.bindValue(5,prec);
        creator.bindValue(6,rad);
        creator.bindValue(7,vpd);
        creator.exec();
        rows++;
        if (creator.lastError().isValid()) {
            qDebug() << "ClimateConverter: Sql-Error:" << creator.lastError().text();
            return;
        }
    }
    db.commit();
    creator.clear();
    db.close();
    QSqlDatabase::removeDatabase("cctemp");
    qDebug() << "ClimateConverter::run: processing complete." << rows << "rows inserted.";

}
