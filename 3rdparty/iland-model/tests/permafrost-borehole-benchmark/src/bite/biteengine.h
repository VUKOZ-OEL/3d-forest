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
#ifndef BITEENGINE_H
#define BITEENGINE_H

#include "biteagent.h"
#include "bitescript.h"


namespace BITE {

class BiteOutput; // forward
class BiteEngine
{
public:
    /// get singleton of the BiteEngine
    static BiteEngine *instance() {if (mInstance) return mInstance;  mInstance=new BiteEngine(); return mInstance; }
    ~BiteEngine();

    void setup();
    void addAgent(BiteAgent *new_agent);
    /// get agent by name
    BiteAgent *agentByName(QString name);
    QStringList agentNames();

    // properties
    QJSEngine *scriptEngine();

    // the current simulation year
    int currentYear() const {return mYear; }
    void setYear(int year) {mYear = year; }

    Grid<double>* preparePaintGrid(QObject *handler, QString name);
    QStringList evaluateClick(QObject *handler, const QPointF coord, const QString &grid_name);
    double variableValueAt(QObject *handler, const QPointF coord, const int layer_id);

    // functions
    void run();

    /// called from agents/items if an error occured during script execution
    void error(QString error_msg);

    /// safe guard calls to the JS engine (only 1 thread allowed)
    QMutex *serializeJS() { return &mSerialize; }

    /// called by iLand for every tree that is removed/harvested/died due to disturbance.
    void notifyTreeRemoval(Tree* tree, int reason);
    void addTreeRemovalFunction(int reason, BiteAgent* agent) { mTreeRemovalNotifiers.insert(reason, agent); }


    // static functions
    static QJSValue valueFromJs(const QJSValue &js_value, const QString &key, const QString default_value=QLatin1Literal(""), const QString &errorMessage=QLatin1Literal(""));
private:
    void resetErrors();
    BiteEngine(); // private ctor
    static BiteEngine *mInstance;
    QList<BiteAgent*> mAgents;
    BiteScript mScript;
    QStringList mErrorStack;
    bool mHasScriptError;
    QMutex mSerialize;
    int mYear;
    bool mRunning;
    QMultiHash<int, BiteAgent*> mTreeRemovalNotifiers;

    friend class BiteOutput;
};

} // end namespaec
#endif // BITEENGINE_H
