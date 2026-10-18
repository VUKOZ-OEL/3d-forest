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
#include "bitescript.h"

#include "biteengine.h"
#include "biteagent.h"

#include "biteitem.h"
#include "bitedispersal.h"
#include "bitecolonization.h"
#include "bitebiomass.h"
#include "bitelifecycle.h"
#include "biteimpact.h"
#include "biteoutputitem.h"

#include "fmtreelist.h"
#include "fmsaplinglist.h"
namespace BITE {


BiteScript::BiteScript(QObject *parent): QObject(parent)
{

}

void BiteScript::setup(BiteEngine *biteengine)
{
    mEngine = biteengine;
    // setup links to JS Object
    QJSEngine *engine = BiteEngine::instance()->scriptEngine();

    if (engine->globalObject().hasOwnProperty("BiteAgent"))
        return; // already done

    qRegisterMetaType<ABE::FMTreeList*>("ABE::FMTreeList*"); // register type, required to have that type as property
    qRegisterMetaType<ABE::FMSaplingList*>("ABE::FMSaplingList*"); // register type, required to have that type as property
    qRegisterMetaType<BiteItem*>("BiteItem*"); // register type, required to have that type as property
    qRegisterMetaType<BiteCellScript*>("BiteCellScript*"); // register type, required to have that type as property
    qRegisterMetaType<BiteAgent*>("BiteAgent*");
    // create this object
    QJSValue jsObj = engine->newQObject(this);
    engine->globalObject().setProperty("Bite", jsObj);

    // createable objects: BiteAgent

    QJSValue jsMetaObject = engine->newQMetaObject(&BiteAgent::staticMetaObject);
    engine->globalObject().setProperty("BiteAgent", jsMetaObject);

    // BiteItem (base class)
    jsMetaObject = engine->newQMetaObject(&BiteItem::staticMetaObject);
    engine->globalObject().setProperty("BiteItem", jsMetaObject);

    // derived classes (tbd)
    jsMetaObject = engine->newQMetaObject(&BiteDispersal::staticMetaObject);
    engine->globalObject().setProperty("BiteDispersal", jsMetaObject);

    jsMetaObject = engine->newQMetaObject(&BiteDistribution::staticMetaObject);
    engine->globalObject().setProperty("BiteDistribution", jsMetaObject);

    jsMetaObject = engine->newQMetaObject(&BiteColonization::staticMetaObject);
    engine->globalObject().setProperty("BiteColonization", jsMetaObject);

    jsMetaObject = engine->newQMetaObject(&BiteBiomass::staticMetaObject);
    engine->globalObject().setProperty("BiteBiomass", jsMetaObject);

    jsMetaObject = engine->newQMetaObject(&BiteLifeCycle::staticMetaObject);
    engine->globalObject().setProperty("BiteLifeCycle", jsMetaObject);

    jsMetaObject = engine->newQMetaObject(&BiteImpact::staticMetaObject);
    engine->globalObject().setProperty("BiteImpact", jsMetaObject);

    jsMetaObject = engine->newQMetaObject(&BiteOutputItem::staticMetaObject);
    engine->globalObject().setProperty("BiteOutput", jsMetaObject);

}

QStringList BiteScript::agents()
{
    return mEngine->agentNames();
}


QString BiteScript::JStoString(QJSValue value)
{
    if (value.isArray() || value.isObject()) {
        QJSValue fun = BiteEngine::instance()->scriptEngine()->evaluate("(function(a) { return JSON.stringify(a); })");
        QJSValue result = fun.call(QJSValueList() << value);
        return result.toString();
    } else
        return value.toString();

}

BiteAgent *BiteScript::agent(QString agent_name)
{
    BiteAgent *ag = mEngine->agentByName(agent_name);
    if (!ag)
        throw IException("There is no Bite Agent with name: " + agent_name);
    return ag;
}

void BiteScript::log(QString msg)
{
    qCDebug(bite).noquote() << msg;
}

void BiteScript::log(QJSValue obj)
{
    QString msg = JStoString(obj);
    qCDebug(bite).noquote() <<  msg;

}

void BiteScript::run(int year)
{
    BiteEngine::instance()->setYear(year);
    BiteEngine::instance()->run();
}

} // end namespace
