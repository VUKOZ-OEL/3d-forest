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
#include <QJSValue>
#include <QJSEngine>

#include "global.h"
#include "resourceunit.h"

#include "windplugin.h"
#include "windmodule.h"
#include "windscript.h"
#include "windout.h"
#include "outputmanager.h"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(iland_wind, WindPlugin)
#endif

WindPlugin::WindPlugin()
{
    qDebug() << "Wind plugin created";
    DBGMODE( qDebug("(Wind plugin in debug mode)"););
    mWind = 0;
    mWindOut = 0;
}

WindPlugin::~WindPlugin()
{
    if (mWind)
        delete mWind;
    mWind = 0;
    qDebug() << "wind plugin destroyed.";
}

QString WindPlugin::name()
{
    return "wind";
}

QString WindPlugin::version()
{
    return "0.1";
}

QString WindPlugin::description()
{
    return "Wind disturbance module for iLand. " \
            "Designed and written by Rupert Seidl/Werner Rammer.";
}

void WindPlugin::setup()
{
    if (!mWind)
        mWind = new WindModule;

    mWind->setup();

    mWindOut = new WindOut();
    mWindOut->setWindModule(mWind);
    GlobalSettings::instance()->outputManager()->removeOutput(mWindOut->tableName());
    GlobalSettings::instance()->outputManager()->addOutput(mWindOut);

}

void WindPlugin::setupResourceUnit(const ResourceUnit *ru)
{
    mWind->setupResourceUnit(ru);
}

void WindPlugin::setupScripting(QJSEngine *engine)
{
    WindScript *wind_script = new WindScript();
    wind_script->setModule(mWind);
    QJSValue obj = engine->newQObject(wind_script);
    engine->globalObject().setProperty("Wind", obj);

    qDebug() << "setup scripting for windmodule called...";
}

void WindPlugin::yearBegin()
{
}

void WindPlugin::run()
{
    mWind->run();
}




