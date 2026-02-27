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

#ifndef CSVFILE_H
#define CSVFILE_H

#include <QObject>
#include <QStringList>
#include <QJSEngine>
class CSVFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool captions WRITE setHasCaptions READ hasCaptions) ///< if true, the first line are considered to be headers
    Q_PROPERTY(bool flat WRITE setFlat READ flat) ///< if true, there is only one column (a flat file)
    Q_PROPERTY(int colCount READ colCount)
    Q_PROPERTY(int rowCount READ rowCount)
public:
    Q_INVOKABLE CSVFile(QObject *parent=0);
    Q_INVOKABLE CSVFile(const QString &fileName) {     mHasCaptions = true; mFlat = false; mFixedWidth=false; loadFile(fileName);} ///< ctor, load @p fileName.
    // actions
    bool openFile(const QString &fileName); ///< open file in streaming mode.
    QVariant colValue(const int col); ///< get value of column with index @p col. Use in streaming mode.
    bool next(); ///< advance to next record (i.e. line). return false if end of file is reached.
    // properties
    bool streamingMode() const { return mStreamingMode; } ///< return true, if in "streaming mode" (for large files)
    bool hasCaptions() const { return mHasCaptions; } ///< true, if first line contains headers
    bool flat() const { return mFlat; } ///< simple list, not multiple columns
    int rowCount() const { return mRowCount; } ///< number or rows (excl. captions), or -1.
    int colCount() const { return mColCount; } ///< number of columns, or -1
    bool isEmpty() const { return mIsEmpty; } /// returns true when no valid file has been loaded (returns false when a file with 0 rows is loaded)
    QStringList captions() const { return mCaptions; } ///< retrieve (a copy) of column headers
    QStringList column(const int col) const; ///< retrieve a string list of a given column
    QVariantList values(const int row) const; ///< get a list of the values in row "row"
    // setters
    void setHasCaptions(const bool hasCaps) { mHasCaptions = hasCaps; }
    void setFixedWidth(const bool hasFixedWidth) { mFixedWidth = hasFixedWidth; }
    void setFlat(const bool isflat) { mFlat = isflat; }
    static void addToScriptEngine(QJSEngine &engine); // called during setup of ScriptEngine
public slots:
    bool loadFile(const QString &fileName); ///< load @p fileName. load the complete file at once.
    bool loadFromString(const QString &content); ///< load content from a given string.
    bool loadFromStringList(QStringList content); ///< load from a string list
    QString columnName(const int col) { if (col<mColCount) return mCaptions[col]; return QString(); } ///< get caption of ith column.
    int columnIndex(const QString &columnName) const { return mCaptions.indexOf(columnName); } ///< index of column or -1 if not available
    // value function with a column name
    QVariant value(const int row, const QString column_name) const { return value(row, columnIndex(column_name)); }

    /// get value of cell denoted by @p row and @p cell. Not available in streaming mode.
    QVariant value(const int row, const int col) const;
    QVariant row(const int row); ///< retrieve content of the full row @p row as a QJSValue
    QJSValue jsValue(const int row, const int col) const;
    QJSValue jsValue(const int row, const QString column_name) const { return jsValue(row, columnIndex(column_name)); }

    void setValue(const int row, const int col, QVariant value); ///< set the value of the column
    void saveFile(const QString &fileName); ///< save the current content to a file

private:
    void clear();
    bool processRows();
    bool mIsEmpty;
    bool mHasCaptions;
    bool mFixedWidth;
    bool mFlat;
    bool mStreamingMode;
    QStringList mCaptions;
    QStringList mRows;
    QString mSeparator;
    int mRowCount;
    int mColCount;
};

#endif // CSVFILE_H
