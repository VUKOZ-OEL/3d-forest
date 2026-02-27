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

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H
#include "output.h"

class OutputManager
{
public:
    OutputManager(); ///< create all outputs
    ~OutputManager();
    void addOutput(Output *output); ///< add a (newly created) output category. freeing of memory is done by outputmanager
    void removeOutput(const QString &tableName); ///< remove (if available) the output with the name 'tableName', free the memory
    void setup(); ///< setup of the outputs + switch on/off (from project file)
    Output *find(const QString& tableName); ///< search for output and return pointer, NULL otherwise
    bool execute(const QString& tableName); ///< execute output with a given name. returns true if executed.
    void save(); ///< save transactions of all outputs
    void close(); ///< close all outputs
    QString wikiFormat(); ///< wiki-format of all outputs
private:
    QList<Output*> mOutputs; ///< list of outputs in system
    // transactions
    void startTransaction(); ///< start database transaction  (if output database is open, i.e. >0 DB outputs are active)
    void endTransaction(); ///< ends database transaction
    bool mTransactionOpen; ///< for database outputs: if true, currently a transaction is open
};

#endif // OUTPUTMANAGER_H
