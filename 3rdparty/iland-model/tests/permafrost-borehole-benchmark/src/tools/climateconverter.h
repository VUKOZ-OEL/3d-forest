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

#ifndef CLIMATECONVERTER_H
#define CLIMATECONVERTER_H
#include <QObject>
#include <QJSEngine>
#include <QJSValue>

#include "expression.h"

class ClimateConverter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName WRITE setFileName READ fileName);
    Q_PROPERTY(QString tableName WRITE setTableName READ tableName);
    Q_PROPERTY(QString database WRITE setDatabase READ database);
    Q_PROPERTY(bool captions WRITE setCaptions READ captions);


    Q_PROPERTY(QString year WRITE setYear READ year);
    Q_PROPERTY(QString month WRITE setMonth READ month);
    Q_PROPERTY(QString day WRITE setDay READ day);
    Q_PROPERTY(QString temp WRITE setTemp READ temp);
    Q_PROPERTY(QString minTemp WRITE setMinTemp READ minTemp);
    Q_PROPERTY(QString prec WRITE setPrec READ prec);
    Q_PROPERTY(QString rad WRITE setRad READ rad);
    Q_PROPERTY(QString vpd WRITE setVpd READ vpd);

public:
    ClimateConverter(QObject *parent=0);
    static void addToScriptEngine(QJSEngine &engine); ///< add this class to scripting engine
    // getters
    const QString fileName() const { return mFileName; }
    const QString tableName() const { return mTableName; }
    const QString database() const { return mDatabase; }
    bool captions() const { return mCaptions; }
    const QString year() const { return mYear; }
    const QString month() const { return mMonth; }
    const QString day() const { return mDay; }
    const QString temp() const { return mTemp; }
    const QString minTemp() const { return mMinTemp; }
    const QString prec() const { return mPrec; }
    const QString rad() const { return mRad; }
    const QString vpd() const { return mVpd; }

    // setters
    void setFileName(const QString fileName) { mFileName = fileName; }
    void setTableName(const QString tableName) { mTableName = tableName; }
    void setDatabase(const QString db) { mDatabase = db; }
    void setCaptions(const bool on) { mCaptions = on; }
    void setYear(const QString value) { mYear = value; }
    void setMonth(const QString value) { mMonth = value; }
    void setDay(const QString value) { mDay = value; }
    void setTemp(const QString value) { mTemp = value; }
    void setMinTemp(const QString value) { mMinTemp = value; }
    void setPrec(const QString value) { mPrec = value; }
    void setRad(const QString value) { mRad = value; }
    void setVpd(const QString value) { mVpd = value; }

public slots:
    void run();

private:
    void bindExpression(Expression &expr, int index);
    double *mVars[100];
    QString mFileName;
    QString mTableName;
    QString mDatabase;
    bool mCaptions;

    QString mYear;
    QString mMonth;
    QString mDay;
    QString mTemp;
    QString mMinTemp;
    QString mPrec;
    QString mRad;
    QString mVpd;

    Expression mExpYear;
    Expression mExpMonth;
    Expression mExpDay;
    Expression mExpTemp;
    Expression mExpMinTemp;
    Expression mExpPrec;
    Expression mExpRad;
    Expression mExpVpd;

};


#endif // CLIMATECONVERTER_H
