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

#ifndef TIMEEVENTS_H
#define TIMEEVENTS_H
#include <QtCore>
/** */
class TimeEvents
{
public:
    TimeEvents();
    // setup
    void clear() { mData.clear(); }
    bool loadFromString(const QString &source);
    bool loadFromFile(const QString &fileName);
    // excecute
    void run(); ///< execute all settings
    /// read value for key 'key' and year 'year' from the list of items. Return QVariant() if not found.
    QVariant value(int year, const QString &key) const;

private:
    QMultiMap<int, QPair<QString, QVariant> > mData;
};

#endif // TIMEEVENTS_H
