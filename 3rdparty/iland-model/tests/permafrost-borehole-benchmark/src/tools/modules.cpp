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
#include "modules.h"
#include "plugin_interface.h"

#include "globalsettings.h"
#include "debugtimer.h"
#include "exception.h"
#include <QtPlugin>

// include the static modules here in the code:
#if QT_VERSION >= 0x050000
Q_IMPORT_PLUGIN(FirePlugin)
Q_IMPORT_PLUGIN(WindPlugin)
Q_IMPORT_PLUGIN(BarkBeetlePlugin)
#else
Q_IMPORT_PLUGIN(iland_fire)
Q_IMPORT_PLUGIN(iland_wind)
Q_IMPORT_PLUGIN(iland_barkbeetle)
#endif

Modules::Modules()
{
    init();
}

// load the static plugins
void Modules::init()
{
    foreach (QObject *plugin, QPluginLoader::staticInstances()) {
        DisturbanceInterface *di = qobject_cast<DisturbanceInterface *>(plugin);
        if (di) {
            qDebug() << di->name();
            // check xml file
            if (GlobalSettings::instance()->settings().valueBool(QString("modules.%1.enabled").arg(di->name()))) {
                // plugin is enabled: store in list of active modules
                mInterfaces.append(di);
                // check for other interfaces
                SetupResourceUnitInterface *si=qobject_cast<SetupResourceUnitInterface *>(plugin);
                if (si)
                    mSetupRUs.append(si);
                WaterInterface *wi = qobject_cast<WaterInterface *>(plugin);
                if (wi)
                    mWater.append(wi);
                TreeDeathInterface *td = qobject_cast<TreeDeathInterface*>(plugin);
                if (td)
                    mTreeDeath.append(td);
            }
        }
    }

    // fix the order of modules: make sure that "barkbeetle" is after "wind"
    DisturbanceInterface *wind = module(QStringLiteral("wind"));
    DisturbanceInterface *bb = module(QStringLiteral("barkbeetle"));
    if (wind && bb) {
        int iw = mInterfaces.indexOf(wind), ib = mInterfaces.indexOf(bb);
        if (ib<iw)
            mInterfaces.swap(iw, ib);
    }


}

DisturbanceInterface * Modules::module(const QString &module_name)
{
    foreach(DisturbanceInterface *di, mInterfaces)
        if (di->name() == module_name)
            return di;
    return 0;
}

void Modules::setupResourceUnit(const ResourceUnit *ru)
{
    foreach(SetupResourceUnitInterface *si, mSetupRUs)
        si->setupResourceUnit(ru);
}

void Modules::setup()
{

    foreach(DisturbanceInterface *di, mInterfaces)
        di->setup();

    // set up the scripting (i.e., Javascript)
    QJSEngine *engine = GlobalSettings::instance()->scriptEngine();
    foreach(DisturbanceInterface *di, mInterfaces)
        di->setupScripting(engine);
}


void Modules::calculateWater(const ResourceUnit *resource_unit, const WaterCycleData *water_data)
{
    foreach(WaterInterface *wi, mWater)
        wi->calculateWater(resource_unit, water_data);
}

void Modules::treeDeath(const Tree *tree, int removal_type)
{
    if (mTreeDeath.isEmpty())
        return;

    for (QList<TreeDeathInterface*>::const_iterator it=mTreeDeath.constBegin(); it!=mTreeDeath.constEnd(); ++it)
        (*it)->treeDeath(tree, removal_type);

}

void Modules::run()
{
    DebugTimer t("modules");

    // *** run in fixed order ***
    foreach(DisturbanceInterface *di, mInterfaces) {
        try {
            di->run();
        } catch (const IException &e) {
            qWarning() << "ERROR: uncaught exception in module '" << di->name() << "':";
            qWarning() << "ERROR:" << e.message();
            qWarning() << " **************************************** ";
        }
    }


    // *** run in random order ****
    //    QList<DisturbanceInterface*> run_list = mInterfaces;

    //    // execute modules in random order
    //    while (!run_list.isEmpty()) {
    //        int idx = irandom(0, run_list.size()-1);
    //        if (logLevelDebug())
    //            qDebug() << "executing disturbance module: " << run_list[idx]->name();

    //        try {
    //            run_list[idx]->run();
    //        } catch (const IException &e) {
    //            qWarning() << "ERROR: uncaught exception in module '" << run_list[idx]->name() << "':";
    //            qWarning() << "ERROR:" << e.message();
    //            qWarning() << " **************************************** ";
    //        }

    //        // remove from list
    //        run_list.removeAt(idx);
    //    }
}

void Modules::yearBegin()
{
    foreach(DisturbanceInterface *di, mInterfaces)
        di->yearBegin();

}



