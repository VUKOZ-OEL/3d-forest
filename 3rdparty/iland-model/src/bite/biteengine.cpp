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
#include "bite_global.h"
#include "biteengine.h"


// BITE
#include "biteagent.h"
#include "bitescript.h"
#include "biteoutput.h"

// iLand specific
#include "globalsettings.h"
#include "helper.h"
#include "modelcontroller.h"
#include "debugtimer.h"
#include "outputmanager.h"

Q_LOGGING_CATEGORY(bite, "bite")

Q_LOGGING_CATEGORY(biteSetup, "bite.setup")


namespace BITE {

BiteEngine *BiteEngine::mInstance=nullptr;


BiteEngine::BiteEngine()
{
}



BiteEngine::~BiteEngine()
{
    // delete all agents
    for (auto *a : mAgents) {
        GlobalSettings::instance()->controller()->removePaintLayers(a);
        delete a;
    }
    mInstance=nullptr;
}

void BiteEngine::setup()
{
    // link BITE specific logging to general logging settings
    QString enable_log = logLevelDebug() ? "true" : "false";
    QLoggingCategory::setFilterRules(QString("bite.debug=%1\n" \
                                     "bite.setup.debug=%1").arg(enable_log)); // enable *all*

    resetErrors();

    if (!GlobalSettings::instance()->outputManager()->find("bite")) {
        GlobalSettings::instance()->outputManager()->addOutput(new BiteOutput);
    }


    // setup scripting
    mScript.setup(this);


    // now load the javascript and execute
    QString file_name = GlobalSettings::instance()->path(GlobalSettings::instance()->settings().value("modules.bite.file"));
    mRunning = true;

    QString code = Helper::loadTextFile(file_name);
    if (code.isEmpty())
        throw IException("Loading of BITE script file '"  + file_name + "'failed; file missing or empty.");

    qCDebug(biteSetup) << "Loading script file" << file_name;
    QJSValue result = GlobalSettings::instance()->scriptEngine()->evaluate(code, file_name);
    mRunning = false;
    if (result.isError()) {
        int lineno = result.property("lineNumber").toInt();
        QStringList code_lines = code.replace('\r', "").split('\n'); // remove CR, split by LF
        QString code_part;
        for (int i=std::max(0, lineno - 5); i<std::min(lineno+5, static_cast<int>(code_lines.count())); ++i)
            code_part.append(QString("%1: %2 %3\n").arg(i).arg(code_lines[i]).arg(i==lineno?"  <---- [ERROR]":""));
        qCCritical(biteSetup).noquote() << "Javascript Error in file" << result.property("fileName").toString() << ":" << result.property("lineNumber").toInt() << ":" << result.toString() << ":\n" << code_part;
        throw IException("BITE Error in Javascript (Please check the logfile): " + result.toString() + "\nIn:\n" + code_part) ;
    }




    if (mHasScriptError) {
        qCCritical(bite) << "Error in setup of BITE engine:" << mErrorStack.join("\n");
        throw IException("BITE-Error (check also the log): \n" + mErrorStack.join("\n"));
    }
}

void BiteEngine::addAgent(BiteAgent *new_agent)
{
    BiteAgent *a = agentByName(new_agent->name());
    if (a) {
        qCInfo(bite) << "adding an agent with a name already in use. Deleting the *old* agent.";
        mAgents.removeOne(a);
        // remove agent from UI
        GlobalSettings::instance()->controller()->removePaintLayers(a);
        delete a;
    }
    mAgents.push_back(new_agent);
    // add agent to UI
    QStringList varlist = new_agent->wrapper()->getVariablesList();
    for (int i=0;i<varlist.size();++i)
        varlist[i] = QString("Bite:%1 - %2").arg(new_agent->name()).arg(varlist[i]);
    GlobalSettings::instance()->controller()->addPaintLayers(new_agent, varlist);

}

BiteAgent *BiteEngine::agentByName(QString name)
{
    for (auto *b : mAgents)
        if (b->name() == name)
            return b;
    return nullptr;
}

QStringList BiteEngine::agentNames()
{
    QStringList names;
    for (auto *b : mAgents)
        names.push_back(b->name());
    return names;
}

QJSEngine *BiteEngine::scriptEngine()
{
    // use global engine from iLand
    return GlobalSettings::instance()->scriptEngine();

}

Grid<double> *BiteEngine::preparePaintGrid(QObject *handler, QString name)
{
    // check if handler is a valid agent
    BiteAgent *ba = qobject_cast<BiteAgent*>(handler);
    if (!ba)
        return nullptr;
    // name: is still Bite::<agentname> - <varname>
    QStringList l = name.split(" - ");
    if (l.size() != 2)
        return nullptr;
    ba->updateDrawGrid(l[1]);
    return ba->baseDrawGrid();
}

QStringList BiteEngine::evaluateClick(QObject *handler, const QPointF coord, const QString &grid_name)
{
    Q_UNUSED(grid_name)
    BiteAgent *ba = qobject_cast<BiteAgent*>(handler);
    if (!ba)
        return QStringList();
    BiteCell *cell = ba->grid().constValueAt(coord);
    if (!cell)
        return QStringList();
    BiteWrapper wrap(ba->wrapper(), cell);
    QStringList result;
    for (int i=0;i<ba->variables().count();++i)
        result.push_back(QString("%1: %2").arg(ba->variables()[i]).arg( wrap.value(i) ));
    return result;
}

double BiteEngine::variableValueAt(QObject *handler, const QPointF coord, const int layer_id)
{
    BiteAgent *ba = qobject_cast<BiteAgent*>(handler);
    if (!ba)
        return 0.;
    if (!ba->grid().coordValid(coord))
        return 0.;
    BiteCell *cell = ba->grid().constValueAt(coord);
    if (!cell)
        return 0.;
    return BiteWrapper(ba->wrapper(), cell).value(layer_id);

}



void BiteEngine::run()
{
    DebugTimer t("Bite:run");
    resetErrors();
    ExprExceptionAsScriptError no_expression;



    qCDebug(bite) << "Run year" << currentYear();
    for (auto *b : mAgents) {
        try {
            mRunning = true;
        b->run();
        } catch (const IException &e) {
            mRunning = false;
            throw IException(QString("Error in execution of the Bite agent '%1': %2").arg(b->name()).arg(e.message()));
        }
        mRunning = false;

        if (mHasScriptError) {
            qCCritical(bite) << "Error in setup of BITE engine:" << mErrorStack.join("\n");
            throw IException("BITE-Error (check also the log): \n" + mErrorStack.join("\n"));
        }
    }
    // execute bite related outputs
    GlobalSettings::instance()->outputManager()->execute("bite");
}

void BiteEngine::error(QString error_msg)
{
    mErrorStack.push_back(error_msg);
    mHasScriptError = true;
    if (!mRunning) {
        GlobalSettings::instance()->scriptEngine()->throwError(error_msg);
        //throw IException("Bite Error: " + error_msg);
    }
}

void BiteEngine::notifyTreeRemoval(Tree *tree, int reason)
{
    QMultiHash<int, BiteAgent*>::iterator i = mTreeRemovalNotifiers.find(reason);
    if (i == mTreeRemovalNotifiers.end())
        return;
    QJSValueList args;
    while (i != mTreeRemovalNotifiers.end() && i.key() == reason) {
        // cout << i.value() << endl;
        i.value()->runOnTreeRemovedFilter(tree, reason);
        ++i;
    }
}




QJSValue BiteEngine::valueFromJs(const QJSValue &js_value, const QString &key, const QString default_value, const QString &errorMessage)
{
   if (!js_value.hasOwnProperty(key)) {
       if (!errorMessage.isEmpty())
           throw IException(QString("Error: required key '%1' not found. In: %2 (JS: %3)").arg(key).arg(errorMessage).arg(BiteScript::JStoString(js_value)));
       else if (default_value.isEmpty())
           return QJSValue();
       else {
           // return a numeric or string as default value
           bool ok;
           double default_numeric = default_value.toDouble(&ok);
           if (ok)
               return default_numeric;
           else
               return default_value;
       }
   }
   return js_value.property(key);
}

void BiteEngine::resetErrors()
{
    mErrorStack.clear();
    mHasScriptError = false;
}


} // end namespace
