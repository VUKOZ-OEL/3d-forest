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
#ifndef BITESCRIPT_H
#define BITESCRIPT_H
#include <QJSValue>
#include <QObject>
namespace BITE {

class BiteEngine;
class BiteAgent;

class BiteScript: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList agents READ agents)

public:
    BiteScript(QObject *parent=nullptr);
    void setup(BiteEngine *biteengine);

    QStringList agents();

    // static members
    static QString JStoString(QJSValue value);

public slots:
    BiteAgent *agent(QString agent_name);

    void log(QString msg);
    void log(QJSValue obj);

    void run(int year);

private:

    BiteEngine *mEngine;
};


} // end name space
#endif // BITESCRIPT_H
