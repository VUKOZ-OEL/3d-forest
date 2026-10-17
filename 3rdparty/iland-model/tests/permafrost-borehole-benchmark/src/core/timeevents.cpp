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
#include "timeevents.h"
#include "helper.h"
#include "csvfile.h"
#include "model.h"

TimeEvents::TimeEvents()
{
}
static QString lastLoadedFile;
bool TimeEvents::loadFromFile(const QString &fileName)
{
    QString source = Helper::loadTextFile(GlobalSettings::instance()->path(fileName));
    if (source.isEmpty())
        throw IException(QString("TimeEvents: input file does not exist or is empty (%1)").arg(fileName));
    lastLoadedFile=fileName;
    return loadFromString(source);
}

bool TimeEvents::loadFromString(const QString &source)
{
    CSVFile infile;
    infile.loadFromString(source);
    QStringList captions = infile.captions();
    int yearcol = infile.columnIndex("year");
    if (yearcol==-1)
        throw IException(QString("TimeEvents: input file '%1' has no 'year' column.").arg(lastLoadedFile));

    int year;
    QVariantList line;
    QPair<QString, QVariant> entry;
    for (int row=0;row<infile.rowCount();row++) {
        year = infile.value(row, yearcol).toInt();
        line = infile.values(row);
        if (line.count()!=infile.colCount())
            throw IException("TimeEvents: invalid file (number of data columns different than head columns)");
        for (int col=0;col<line.count();col++) {
             if (col!=yearcol) {
                 entry.first = captions[col];
                 entry.second = line[col];
                 mData.insert(year, entry);
             }
        }
    } // for each row
    qDebug() << QString("loaded TimeEvents (file: %1). %2 items stored.").arg(lastLoadedFile).arg(mData.count());
    return true;
}

void TimeEvents::run()
{
    int current_year = GlobalSettings::instance()->currentYear();
    QList<QPair<QString, QVariant> > entries = mData.values(current_year);
    if (entries.count()==0)
        return;

    QString key;
    int values_set = 0;
    for (int i=0;i<entries.count();i++) {
        key = entries[i].first; // key
        // special values: if (key=="xxx" ->
        if (key=="script" || key=="javascript") {
            // execute as javascript expression within the management script context...
            if (!entries[i].second.toString().isEmpty()) {
                qDebug() << "executing Javascript time event:" << entries[i].second.toString();
                GlobalSettings::instance()->executeJavascript(entries[i].second.toString());
            }

        } else {
            // no special value: a xml node...
            if (! const_cast<XmlHelper&>(GlobalSettings::instance()->settings()).setNodeValue(key, entries[i].second.toString()))
                qDebug() << "TimeEvents: Error: Key " << key << "not found! (tried to set to" << entries[i].second.toString() << ")";
            else
                qDebug() << "TimeEvents: set" << key << "to" << entries[i].second.toString();
        }
        values_set++;
    }
    if (values_set)
        qDebug() << "TimeEvents: year" << current_year << ":" << values_set << "values set.";
}

// read value for key 'key' and year 'year' from the list of items.
// return a empty QVariant if for 'year' no value is set
QVariant TimeEvents::value(int year, const QString &key) const
{
    QMultiMap<int, QPair<QString, QVariant> >::ConstIterator it = mData.find(year);
    while (it!=mData.constEnd()) {
        if (it->first == key)
            return it->second;
        ++it;
    }
    return QVariant();
}
