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
#include "biteitem.h"
#include "biteengine.h"

namespace BITE {

BiteItem::BiteItem(QObject *parent) : QObject(parent), mAgent(nullptr)
{

}

BiteItem::BiteItem(QJSValue obj) : QObject(nullptr), mAgent(nullptr)
{
    mObj = obj;

}

void BiteItem::setup(BiteAgent *agent)
{
    mAgent = agent;
    try {
        mDescription = BiteEngine::valueFromJs(mObj, "description").toString();
        setRunCells(true); // default: run by cell


    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of Bite item '%1': %2").arg(mName).arg(e.message());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);

    }
    qCDebug(biteSetup) << "*** Setup of a item complete ***";
}

QString BiteItem::info()
{
    return QString("*** base class BiteItem ****");
}

void BiteItem::notify(BiteCell *cell, BiteCell::ENotification what)
{
    Q_UNUSED(cell)
    Q_UNUSED(what)
}

void BiteItem::afterSetup()
{
    // nothing in base class
}

void BiteItem::beforeRun()
{
    // nothing in base class
}

void BiteItem::run()
{
    qCDebug(bite) << " *** Execution of item: " << name();
}

void BiteItem::runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist)
{
    // do nothing
    Q_UNUSED(cell)
    Q_UNUSED(treelist)
    Q_UNUSED(saplist)
}

int BiteItem::cellSize() const
{
    Q_ASSERT(mAgent!=nullptr);
    return agent()->cellSize();
}

QStringList BiteItem::allowedProperties()
{
    QStringList l;
    l << "description";
    return l;
}

void BiteItem::checkProperties(QJSValue obj)
{
    QStringList allowed = allowedProperties();
    if (obj.isObject()) {
        QJSValueIterator it(obj);
        while (it.hasNext()) {
            it.next();
            if (!it.name().startsWith("on") &&  !it.name().startsWith("user") && !allowed.contains(it.name())) {
                qCDebug(biteSetup) << it.name() << "is not a valid property! Allowed are: " << allowed;
            }
        }
    }
}

bool BiteItem::verbose()
{
    Q_ASSERT(mAgent!=nullptr);
    return mAgent->verbose();
}



}
