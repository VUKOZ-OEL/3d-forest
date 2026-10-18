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
#include "output.h"
#include <QtCore>
#include <QtSql>


/** @class Output
   The Output class abstracts output data (database, textbased, ...).
   To create a new output, create a class derived from Output and perform the following steps:
   - Overwrite constructor:
     Create columns and set fixed properties (e.g. table name)
   - overwrite setup()
     this function is called after the project file is read. You can access a XmlHelper calling settings()
     which is set to the top-node of the output (defined by tableName() which is set in the constructor). Access settings
     using relative xml-pathes (see example).
   - overwrite exec()
     add data using the stream operators or add() function of Output. Call writeRow() after each row. Each invokation
     of exec() is a database transaction.
   - Add the output to the constructor of @c OutputManager

   @par Example
   @code
   // (1) Overwrite constructor and set name, description and columns
   TreeOut::TreeOut()
    {
        setName("Tree Output", "tree");
        setDescription("Output of indivdual trees.");
        columns() << OutputColumn("id", "id of the tree", OutInteger)
                 << OutputColumn("name", "tree species name", OutString)
                 << OutputColumn("v1", "a double value", OutDouble);
     }
    // (2) optionally: some special settings (here: filter)
    void TreeOut::setup()
    {
        QString filter = settings().value(".filter","");
        if (filter!="")
            mFilter = QSharedPointer<Expression>(new Expression(filter));
    }

    // (3) the execution
    void TreeOut::exec()
    {
        AllTreeIterator at(GlobalSettings::instance()->model());
        while (Tree *t=at.next()) {
            if (mFilter && !mFilter->execute()) // skip if filter present
                continue;
            *this << t->id() << t->species()->id() << t->dbh(); // stream operators
            writeRow(); // executes DB insert
        }
    }
    // in outputmanager.cpp:
    OutputManager::OutputManager()
    {
        ...
        mOutputs.append(new TreeOut); // add the output
        ...
    }
    @endcode

*/
const GlobalSettings *Output::gl = GlobalSettings::instance();


void Output::exec()
{
    qDebug() << "Output::exec() called! (should be overrided!)";
}

void Output::setup()
{
}

Output::~Output()
{
    mInserter.clear();
}

Output::Output()
{
    mCount=0;
    mMode = OutDatabase;
    mOpen = false;
    mEnabled = false;
    newRow();
}


/** create the database table and opens up the output.
  */
void Output::openDatabase()
{
    QSqlDatabase db = GlobalSettings::instance()->dbout();
    // create the "create table" statement
    QString sql = "create table " +mTableName + "(";
    QString insert="insert into " + mTableName + " (";
    QString values;

    foreach(const OutputColumn &col, columns()) {
        switch(col.mDatatype) {
            case OutInteger: sql+=col.mName + " integer"; break;
            case OutDouble: sql+=col.mName + " real"; break;
            case OutString: sql+=col.mName + " text"; break;
        }
        insert+=col.mName+",";
        values+=QString(":")+col.mName+",";

        sql+=",";
    }
    sql[sql.length()-1]=')'; // replace last "," with )
    //qDebug()<< sql;
    if (mInserter.isValid())
        mInserter.clear();
    QSqlQuery creator(db);
    QString drop=QString("drop table if exists %1").arg(tableName());
    creator.exec(drop); // drop table (if exists)
    creator.exec(sql); // (re-)create table
    //creator.exec("delete from " + tableName()); // clear table??? necessary?

    if (creator.lastError().isValid()){
        throw IException(QString("Error creating output: %1 \n Statement: %2").arg( creator.lastError().text()).arg(sql) );
    }
    insert[insert.length()-1]=')';
    values[values.length()-1]=')';
    insert += QString(" values (") + values;
    //qDebug() << insert;
    mInserter = QSqlQuery(db);
    mInserter.prepare(insert);
    if (mInserter.lastError().isValid()){
        throw IException(QString("Error creating output: %1 \n Statement: %2").arg( mInserter.lastError().text()).arg(insert) );
    }
    for (int i=0;i<columns().count();i++)
        mInserter.bindValue(i,mRow[i]);

    mOpen = true;
}

void Output::openFile()
{
    QString path = GlobalSettings::instance()->path(mTableName + ".csv", "output");
    mOutputFile.setFileName(path);
    if (!mOutputFile.open(QIODevice::WriteOnly | QIODevice::Text))
          throw IException(QString("The file '%1' for output '%2' cannot be opened!").arg(path, name()) );

    // create header
    mFileStream.setDevice(&mOutputFile);
    QString line; bool first=true;
    foreach(const OutputColumn &col, columns()) {
        if (first) {
            line = col.name();
            first = false;
        }  else {
            line += ";" + col.name();
        }
    }
    mFileStream << line << endl;

}

void Output::newRow()
{
    mIndex = 0;
}



void Output::writeRow()
{
    DBG_IF(mIndex!=mCount, "Output::save()", "received invalid number of values!");
    if (!isOpen())
        open();
    switch(mMode) {
        case OutDatabase:
            saveDatabase(); break;
    case OutFile:
            saveFile(); break;
        default: throw IException("Invalid output mode");
    }

}

static QMutex __protectWriteRow;
void Output::singleThreadedWriteRow()
{
    QMutexLocker l(&__protectWriteRow);
    writeRow();
}

/// delete columns that were added after the column 'find_name'
bool Output::clearColumnsAfter(QString find_name)
{
    if (mColumns.isEmpty() || mColumns.last().name() == find_name)
        return false; // nothing to do

    QList<OutputColumn>::iterator i=mColumns.begin();
    while (i!=mColumns.end()) {
        if (i->name()==find_name)
            break;
        ++i;
    }
    mColumns.erase(++i, mColumns.end());
    mCount = mColumns.size();
    return true;
}

void Output::truncateTable()
{
    QSqlDatabase db = GlobalSettings::instance()->dbout();
    QSqlQuery query(db);
    QString stmt=QString("delete from %1").arg(tableName());
    query.exec(stmt); //
    qDebug() << "truncated table" << tableName() << "(=delete all records from output database)";

}

void Output::open()
{
    if (isOpen())
        return;
    // setup columns
    mCount = columns().count();
    mRow.resize(mCount);
    mOpen = true;
    newRow();
    // setup output
    switch(mMode) {
        case OutFile:
            openFile(); break;
        case OutDatabase:
            openDatabase(); break;
        default: throw IException("Invalid output mode");
    }
}

void Output::close()
{
    if (!isOpen())
        return;
    mOpen = false;
    switch (mMode) {
        case OutDatabase:
            // calling finish() ensures, that the query and all locks are freed.
            // having (old) locks on database connections, degrades insert performance.
            if (mInserter.isValid())
                mInserter.finish();
            mInserter = QSqlQuery(); // clear inserter
         break;
    case OutFile:
        mOutputFile.close();
        break;
        default:
         qWarning() << "Output::close with invalid mode";
    }

}


void Output::saveDatabase()
{
   for (int i=0;i<mCount;i++)
        mInserter.bindValue(i,mRow[i]);
    mInserter.exec();
    if (mInserter.lastError().isValid()){
        throw IException(QString("Error during saving of output tables: '%1'' (native code: '%2', driver: '%3')")
                         .arg( mInserter.lastError().text())
                         .arg(mInserter.lastError().nativeErrorCode())
                         .arg(mInserter.lastError().driverText()) );
    }

    newRow();
}

void Output::saveFile()
{
    for (int i=0;i<mCount;++i) {
        mFileStream << mRow[i].toString();
        if (i!=mCount-1)
            mFileStream << ";";
    }
    mFileStream << endl;
    newRow();
}

QString Output::wikiFormat() const
{
    QString result=QString("!!%1\nTable Name: %2\n%3\n\n").arg(name(), tableName(), description());
    // loop over columns...
    result += "||__caption__|__datatype__|__description__\n"; // table begin
    foreach(const OutputColumn &col, mColumns)
        result+=QString("%1|%2|%3\n").arg(col.name(), col.datatype(), col.description());
    result[result.length()-1]=' '; // clear last newline
    result+="||\n";
    return result;
}
