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
#include "barkbeetleplugin.h"
#include "barkbeetleout.h"
#include "barkbeetlescript.h"
#include "outputmanager.h"
#include "tree.h"
#include "species.h"

#include <QObject>
#include <QJSValue>
#include <QJSEngine>

//#if QT_VERSION < 0x050000
//Q_EXPORT_PLUGIN2(iland_barkbeetle, BarkBeetlePlugin)
//#endif

QString BarkBeetlePlugin::name()
{
    return "barkbeetle";
}

QString BarkBeetlePlugin::version()
{
    return "1.0";
}

QString BarkBeetlePlugin::description()
{
    return "bark beetle module for iLand.";
}


BarkBeetlePlugin::BarkBeetlePlugin()
{
    qDebug() << "Bark beetle plugin created";
    DBGMODE( qDebug("(bark beetle plugin in debug mode)"););

//    foreach (const ResourceUnit *ru, GlobalSettings::instance()->model()->ruList())
    //        qDebug() << ru->boundingBox() << ru->constTrees().count();
}

void BarkBeetlePlugin::setup()
{
    // setup of the fire related outputs: note: here the fire module is passed directly to the output
    BarkBeetleOut *bb_output = new BarkBeetleOut();
    bb_output->setBarkBeetleModule(&mBeetle);
    GlobalSettings::instance()->outputManager()->removeOutput(bb_output->tableName());
    GlobalSettings::instance()->outputManager()->addOutput(bb_output);
    // setup of the fire module: load parameters from project file, etc.
    mBeetle.setup();
}

//Q_SCRIPT_DECLARE_QMETAOBJECT(FireScript, QObject*)

// add the fire script interface
void BarkBeetlePlugin::setupScripting(QJSEngine *engine)
{
    BarkBeetleScript *script = new BarkBeetleScript();
    script->setBBModule(&mBeetle);
    QJSValue obj = engine->newQObject(script);
    engine->globalObject().setProperty("BarkBeetle", obj);


    qDebug() << "setup scripting of BarkBeetlePlugin called...";
}







