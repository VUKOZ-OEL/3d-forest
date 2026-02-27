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
#include "fireplugin.h"
#include "outputmanager.h"
#include "fireout.h"
#include "firescript.h"

#include <QObject>
#include <QJSValue>
#include <QJSEngine>

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(iland_fire, FirePlugin)
#endif

QString FirePlugin::name()
{
    return "fire";
}

QString FirePlugin::version()
{
    return "1.0";
}

QString FirePlugin::description()
{
    return "Fire disturbance module for iLand. The fire ignition and fire spread follows the FireBGC v2 model (Keane et al 2011), " \
            "the estimation of severity and fire effects Schumacher et al (2006). See https://iland-model.org/wildfire for details.\n" \
            "Designed and written by by Rupert Seidl/Werner Rammer.";
}


FirePlugin::FirePlugin()
{
    qDebug() << "Fire plugin created";
    DBGMODE( qDebug("(Fire plugin in debug mode)"););

//    foreach (const ResourceUnit *ru, GlobalSettings::instance()->model()->ruList())
    //        qDebug() << ru->boundingBox() << ru->constTrees().count();
}

void FirePlugin::setup()
{
    // setup of the fire related outputs: note: here the fire module is passed directly to the output
    GlobalSettings::instance()->outputManager()->removeOutput("fire");
    FireOut *fire_output = new FireOut();
    fire_output->setFireModule(&mFire);
    GlobalSettings::instance()->outputManager()->addOutput(fire_output);
    // setup of the fire module: load parameters from project file, etc.
    mFire.setup();
}

//Q_SCRIPT_DECLARE_QMETAOBJECT(FireScript, QObject*)

// add the fire script interface
void FirePlugin::setupScripting(QJSEngine *engine)
{
    FireScript *fire_script = new FireScript();
    fire_script->setFireModule(&mFire);
    QJSValue obj = engine->newQObject(fire_script);
    engine->globalObject().setProperty("Fire", obj);
    mFire.setScriptObj(fire_script);

    qDebug() << "setup scripting called...";
}





