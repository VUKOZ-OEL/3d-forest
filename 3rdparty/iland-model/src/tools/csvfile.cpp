/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer
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

#include "csvfile.h"
#include <QtCore>
#include "helper.h"

/** @class CSVFile
  @ingroup tools
  Provides access to table data stored in text files (CSV style).
  Tables have optionally headers in first line (hasCaptions()) and can use various
  delimiters ("tab",";",","," "). If separated by spaces, consecuteive spaces are merged.
  Table dimensions can be accessed with colCount() and rowCount(), cell values as QVariant are retrieved
  by value(). full rows are retrieved using row().
  Files are loaded by loadFile() or by passing a filename to the constructor:
  @code
  CSVFile file(fileName);
  for (int row=0; row<file.rowCount(); row++)
     for (int col=0; col<file.colCount(); col++)
       value = file.value(row, col);
  @endcode
  Planned is also a "streaming" mode for large files (loadFile(), while(file.next()) file.value(x) ), but not finsihed yet.

*/
#include <QJSEngine>
#include <QJSValue>
//Q_SCRIPT_DECLARE_QMETAOBJECT(CSVFile, QObject*)
void CSVFile::addToScriptEngine(QJSEngine &engine)
{
    Q_UNUSED(engine); // remove this code?
    // about this kind of scripting magic see: http://qt.nokia.com/developer/faqs/faq.2007-06-25.9557303148
    //QJSValue cc_class = engine.scriptValueFromQMetaObject<CSVFile>();

    // TODO: solution for creating objects (on the C++ side...)

    // the script name for the object is "CSVFile".
    //engine.globalObject().setProperty("CSVFile", cc_class);
    QJSValue jsMetaObject = engine.newQMetaObject(&CSVFile::staticMetaObject);
    engine.globalObject().setProperty("CSVFile", jsMetaObject);

}

CSVFile::CSVFile(QObject *)
{
    mIsEmpty = true;
    mHasCaptions = true;
    mFlat = false;
    mFixedWidth=false;
    clear();
}

void CSVFile::clear()
{
    mColCount = mRowCount = -1;
    mCaptions.clear();
    mRows.clear();
    mIsEmpty = true;

}

bool CSVFile::processRows()
{
    if (mRows.count()==0)
        return false;

    mIsEmpty = false;
    // trimming of whitespaces is a problem
    // when having e.g. tabs as delimiters...
    //    if (!mFixedWidth) {
    //        for (int i=0;i<mRows.count();i++)
    //            mRows[i] = mRows[i].trimmed();
    //    }
    // drop comments (i.e. lines at the beginning that start with '#', also ignore '<' (are in tags of picus-ini-files)
    while (!mRows.isEmpty() && (mRows.front().startsWith('#') || mRows.front().startsWith('<')))
        mRows.pop_front();
    while (!mRows.isEmpty() && mRows.last().isEmpty())
        mRows.removeLast();

    mSeparator = ";"; // default
    QString first = mRows.first();
    if (!mFlat) {
        // detect separator
        int c_tab = first.count('\t');
        int c_semi = first.count(';');
        int c_comma = first.count(',');
        int c_space = first.count(' ');
        if (c_tab+c_semi+c_comma+c_space == 0) {
            qDebug() << "CSVFile::loadFile: cannot recognize separator. first line:" << first;
            return false;
        }
        mSeparator=" ";
        if (c_tab > c_semi && c_tab>c_comma) mSeparator="\t";
        if (c_semi > c_tab && c_semi>c_comma) mSeparator=";";
        if (c_comma > c_tab && c_comma>c_semi) mSeparator=",";
        //        if (mSeparator==" ") {
        //            for (int i=0;i<mRows.count();i++)
        //                mRows[i] = mRows[i].simplified();
        //            first = mRows.first();
        //        }
    } // !mFlat

    // captions
    if (mHasCaptions) {
        mCaptions = first.split(mSeparator, Qt::KeepEmptyParts).replaceInStrings("\"", ""); // drop "-characters
        mRows.pop_front(); // drop the first line
    } else {
        // create pseudo captions
        mCaptions = first.split(mSeparator, Qt::KeepEmptyParts);
        for (int i=0;i<mCaptions.count();i++)
            mCaptions[i] = QString("c%1").arg(i);
    }

    mColCount = mCaptions.count();
    mRowCount = mRows.count();
    mStreamingMode = false;
    return true;

}

bool CSVFile::loadFromString(const QString &content)
{
    clear();
    // split into rows: use either with windows or unix style delimiter
    if (content.left(1000).contains("\r\n"))
        mRows = content.split("\r\n", Qt::SkipEmptyParts);
    else
        mRows = content.split("\n", Qt::SkipEmptyParts);

    return processRows();
}

bool CSVFile::loadFromStringList(QStringList content)
{
    clear();
    mRows = content;

    return processRows();
}

bool CSVFile::loadFile(const QString &fileName)
{
    QStringList content = Helper::loadTextFileLines(fileName);
    if (content.isEmpty()) {
        qDebug() << "CSVFile::loadFile" << fileName << "does not exist or is empty.";
        mIsEmpty = true;
        return false;
    }
    return loadFromStringList(content);
}
QVariantList CSVFile::values(const int row) const
{
    QVariantList list;
    if (row<0 || row>=mRowCount) {
        qDebug() << "CSVFile::values: invalid row:" << row;
        return list;
    }
    QStringList line = mRows[row].split(mSeparator);
    foreach(QString item, line)
        list.append(item);
    return list;
}

QVariant CSVFile::value(const int row, const int col) const
{
    if (mStreamingMode)
        return QVariant();

    if (row<0 || row>=mRowCount || col<0 || col>=mColCount) {
        qDebug() << "CSVFile::value: invalid index: row col:" << row << col << ". Size is:" << mRowCount << mColCount;
        return QVariant();
    }

    if (mFixedWidth) {
        // special case with space (1..n) as separator
        QString s = mRows[row];
        QChar sep=mSeparator.at(0);
        QVariant result;
        if (col==mColCount-1) {
            // last element:
            result = s.mid(s.lastIndexOf(sep)+1);
            return result;
        }
        int sepcount=0;
        int lastsep=0;
        int i=0;
        while (s.at(i) == sep && i<s.size()) i++; // skip initial spaces
        for (;i<s.size();i++) {
            if (s.at(i) == sep) {
                // skip all spaces
                while (s.at(i)==sep)
                    i++;
                i--; // go back to last separator
                // count the separators up to the wanted column
                if (sepcount==col) {
                    result = s.mid(lastsep,i-lastsep);
                    return result;
                }
                sepcount++;
                lastsep=i+1;
            }
        }
        qDebug() << "CSVFile::value: found no result:" << row << col << ". Size is:" << mRowCount << mColCount;
        return QVariant();
    }

    // one-character separators....
    if (mSeparator.length()==1) {
        QString s = mRows[row];
        QChar sep=mSeparator.at(0);
        QVariant result;
        if (col==mColCount-1) {
            // last element:
            if (s.count(sep)==mColCount-1) {
                result =  s.mid(s.lastIndexOf(sep)+1);
                if (result.toString().startsWith('\"') && result.toString().endsWith('\"'))
                    result = result.toString().mid(1, result.toString().length()-2);
            }
            // if there are less than colcount-1 separators, then
            // the last columns is empty
            return result;
        }
        int sepcount=0;
        int lastsep=0;
        for (int i=0;i<s.size();i++) {
            if (s.at(i) == sep) {
                // count the separators up to the wanted column
                if (sepcount==col) {
                    if (s.at(lastsep)=='\"' && s.at(i-1)=='\"')
                        result = s.mid(lastsep+1,i-lastsep-2); // ignore "
                    else
                        result = s.mid(lastsep,i-lastsep).trimmed(); // remove whitespace

                    return result;
                }
                sepcount++;
                lastsep=i+1;
            }
        }
        if (sepcount==col)
            result = s.mid(s.lastIndexOf(sep)+1);
        //qDebug() << "CSVFile::value: found no result:" << row << col << ". Size is:" << mRowCount << mColCount;
        return result;

    }

    // fallback, if separator is more than one character. This is very slow approach.... (old)
    QStringList line = mRows[row].split(mSeparator);
    QVariant result;
    if (col<line.count()) {
        result = line[col];
    }
    return result;
}
QVariant CSVFile::row(const int row)
{
    if (mStreamingMode)
        return QVariant();

    if (row<0 || row>=mRowCount) {
        qDebug() << "CSVFile::row: invalid index: row " << row << ". Size is:" << mRowCount ;
        return QVariant();
    }

    QVariant result = mRows[row];
    return result;
}

QJSValue CSVFile::jsValue(const int row, const int col) const
{
    QVariant val = value(row, col);
    bool ok;
    int ival = val.toInt(&ok);
    if(ok) return QJSValue(ival);
    double dval = val.toDouble(&ok);
    if (ok) return QJSValue(dval);
    return QJSValue(val.toString());
}

bool CSVFile::openFile(const QString &fileName)
{
    (void)fileName; // silence compiler warning; the function makes no sense, nonetheless.
    mStreamingMode = true;
    return false;
}

QStringList CSVFile::column(const int col) const
{
    QStringList result;
    for (int row=0;row<rowCount();row++)
        result+=value(row,col).toString();
    return result;
}

void CSVFile::setValue(const int row, const int col, QVariant value)
{
    if (row<0 || row>=mRowCount || col<0 || col>mColCount) {
        qDebug() << "CSVFile::setValue: invalid index: row col:" << row << col << ". Size is:" << mRowCount << mColCount;
        return;
    }
    if (!mFixedWidth) {
        QStringList line = mRows[row].split(mSeparator);
        if (col<line.count()) {
            line[col] = value.toString();
        }
        mRows[row] = line.join(mSeparator);
    } else {
        // if sep=space, we assume a fixed format... and try to insert the new data right-padded
        QString data = value.toString();
        QString &s = mRows[row];
        int field=mColCount-1;
        QChar sep = mSeparator[0];
        for (int i=s.size()-1; i>=0; --i) {
            if (field == col) {
                // replace: first, replace data with spaces...
                int r = i; // position of last character of the data
                while (s.at(i)!=sep) {
                    s[i] = sep;
                    i--;
                }
                // insert data
                for (int j=0;j<data.size();j++)
                    s[r - data.size() + j + 1] = data.at(j);
                return; // finished
            }
            if (s.at(i)==sep) {
                field--;
                while (s.at(i)==sep && i>=0) i--; // skip multiple spaces
                i++;
            }
        }
        qDebug() << "CSVFile::saveFile: save value with fixed width: col not found.";
    }
}

/// save the contents of the CSVFile back to a file.
/// this removes all comments and uses the system line-end
void CSVFile::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "CSVFile::saveFile: could not create file" + fileName;
        return;
    }
    QTextStream str(&file);
    if (mHasCaptions)
        str << mCaptions.join(mSeparator) << Qt::endl;
    foreach(const QString s, mRows)
        str << s << Qt::endl;
}

