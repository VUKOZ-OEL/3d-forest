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

#include "sqlhelper.h"
#include <QtSql>


/** @class SqlHelper
  @ingroup tools
  A helper class for simple execution of database commands.
*/
SqlHelper::SqlHelper()
{
}

/** execute 'query' against database 'database'. The first column of the first row are returned.
  A Null-Variant is returned, if the query has no results. */
QVariant SqlHelper::queryValue(const QString &query, const QSqlDatabase &database)
{
    QSqlQuery q(database);
    if (!q.exec(query)) {
        qDebug() << "query"<< query << " raised SQL-Error:" << q.lastError().text();
        return QVariant();
    }
    if (q.next()) {
        return q.value(0);
    }
    return QVariant();
}

/** execute 'query' against database 'database'.
Use for insert, update, ... statements without return values. */
bool SqlHelper::executeSql(const QString &query, const QSqlDatabase &database)
{
    QSqlQuery q(database);
    bool success = q.exec(query);
    if (!success)
        qDebug() << "query"<< query << " raised SQL-Error:" << q.lastError().text();
    return success;
}
