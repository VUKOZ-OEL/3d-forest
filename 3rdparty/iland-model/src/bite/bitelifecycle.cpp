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
#include "bitelifecycle.h"
#include "biteengine.h"

namespace BITE {



BiteLifeCycle::BiteLifeCycle(QJSValue obj): BiteItem(obj)
{

}

void BiteLifeCycle::setup(BiteAgent *parent_agent)
{
    BiteItem::setup(parent_agent);
    try {

        checkProperties(mObj);

        QJSValue volt = BiteEngine::valueFromJs(mObj, "voltinism", "", "'voltinism' is a required property");
        mVoltinism.setup(volt, DynamicExpression::CellWrap, parent_agent);

        QJSValue spread_filter = BiteEngine::valueFromJs(mObj, "spreadFilter", "", "'spreadFilter' is a required property");
        mSpreadFilter.setup(spread_filter, DynamicExpression::CellWrap, parent_agent);

        mSpreadDelay = BiteEngine::valueFromJs(mObj, "spreadDelay", "0", "'spreadDelay' is a required property").toInt();
        mDieAfterDispersal = BiteEngine::valueFromJs(mObj, "dieAfterDispersal", "", "'dieAfterDispersal' is a required property").toBool();

        QJSValue spread_freq = BiteEngine::valueFromJs(mObj, "spreadInterval", "1", "'spreadInterval' is a required property");
        mSpreadInterval.setup(spread_freq, DynamicExpression::CellWrap, parent_agent);

        QJSValue outbreak_duration = BiteEngine::valueFromJs(mObj, "outbreakDuration", "0");
        mOutbreakDuration.setup(outbreak_duration, DynamicExpression::CellWrap, parent_agent);

        QJSValue outbreak_start = BiteEngine::valueFromJs(mObj, "outbreakStart", "-1");
        mOutbreakStart.setup(outbreak_start, DynamicExpression::CellWrap, parent_agent);

        mOutbreakYears = 0; mNextOutbreakStart = 0; mThisOutbreakDuration = 0;
        BiteCell *dummy = *agent()->grid().begin();
        mNextOutbreakStart = static_cast<int>(mOutbreakStart.evaluate(dummy));
        if (mNextOutbreakStart>0)
            qCDebug(biteSetup) << "LifeCycle with active outbreak waves. First outbreak in" << mNextOutbreakStart << "years.";


        QJSValue mort = BiteEngine::valueFromJs(mObj, "mortality", "0");
        mMortality.setup(mort, DynamicExpression::CellWrap, parent_agent);

        mThis = BiteEngine::instance()->scriptEngine()->newQObject(this);
        BiteAgent::setCPPOwnership(this);

        mEvents.setup(mObj, QStringList() <<  "onSetup", agent());
        QJSValueList eparam = QJSValueList() << thisJSObj();
        mEvents.run("onSetup", nullptr, &eparam);




    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of BiteLifeCylcle item '%1': %2").arg(name()).arg(e.message());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);

    }

}

QString BiteLifeCycle::info()
{
    QString res = QString("Type: BiteLifeCycle\nDesc: %1").arg(description());
    return res;

}

void BiteLifeCycle::notify(BiteCell *cell, BiteCell::ENotification what)
{
    Q_UNUSED(cell)
    switch (what) {

    default: break; // ignore other notifications
    }
}

void BiteLifeCycle::run()
{
    // outbreak dynamics
    calcOutbreakWaves();

    // mortality
    if (mMortality.isConst())
        return;


    BiteCell **cell = agent()->grid().begin();
    for (; cell !=agent()->grid().end(); ++cell) {
        if (*cell && (*cell)->isActive()) {
            (*cell)->setTreesLoaded(false);
            (*cell)->setSaplingsLoaded(false);
            if (mMortality.evaluateBool(*cell)) {
                (*cell)->die();
            }

        }
    }




}

void BiteLifeCycle::yearEnd()
{

    if (!dieAfterDispersal())
        return;

    BiteCell **cell = agent()->grid().begin();
    for (; cell !=agent()->grid().end(); ++cell) {
        if (*cell && (*cell)->isActive()) {
            if ( (*cell)->isSpreading() &&
                 (*cell)->yearLastSpread() == BiteEngine::instance()->currentYear()) {

                // kill cells that should die after dispersal
                (*cell)->die();
            }
        }
    }
}

int BiteLifeCycle::numberAnnualCycles(BiteCell *cell)
{
    double vol_res = mVoltinism.evaluate(cell);
    if (agent()->verbose()) qCDebug(bite) << "LifeCycle: voltinism:" << vol_res;
    return static_cast<int>(vol_res);
}

bool BiteLifeCycle::shouldSpread(BiteCell *cell)
{
    if (!cell->isActive())
        return false;

    if (mSpreadDelay > cell->yearsLiving()) {
        if (agent()->verbose()) qCDebug(bite) << "Not spreading (initial delay)";
        return false;
    }

    if (mSpreadFilter.isValid()) {
        double res = mSpreadFilter.evaluate(cell);
        // the result of the filter is interpreted probabilistically
        if (drandom() < res) {
            if (agent()->verbose())
                qCDebug(bite) << cell->info() <<  ": Spreading, p:" << res;
            return true;
        }
    }

    double val = mSpreadInterval.evaluate(cell);
    if (BiteEngine::instance()->currentYear() - cell->yearLastSpread() >= val) {
        if (agent()->verbose())
            qCDebug(bite) << cell->info() << ": Spreading (Interval)";
        return true;
    }

    return false;

}

QStringList BiteLifeCycle::allowedProperties()
{
    QStringList l = BiteItem::allowedProperties();
    l << "dieAfterDispersal" << "spreadFilter" << "spreadDelay" << "spreadInterval" << "voltinism"
      << "outbreakDuration" << "outbreakStart" << "mortality";
    return l;
}

void BiteLifeCycle::calcOutbreakWaves()
{
    if (mNextOutbreakStart < 0)
        return; // disabled

    BiteCell *dummy = *agent()->grid().begin();
    if (mOutbreakYears>0) {

        // we are currently in an outbreak, determine if we need to stop
        if (mOutbreakYears >= mThisOutbreakDuration) {
            mOutbreakYears = 0; // stop the outbreak
            // determine now when the next outbreak should start
            mNextOutbreakStart = static_cast<int>(mOutbreakStart.evaluate(dummy));
            qCDebug(bite) << "end of outbreak wave: next wave in" << mNextOutbreakStart << "years";
        } else {
            ++mOutbreakYears;
            qCDebug(bite) << "in outbreak wave: outbreakYears=" << mOutbreakYears;
        }

    } else {
        if (--mNextOutbreakStart == 0) {
            // start an outbreak
            mThisOutbreakDuration = static_cast<int>( mOutbreakDuration.evaluate(dummy) );
            if (mThisOutbreakDuration < 1)
                throw IException(QString("BiteLifeCycle: invalid value for 'outbreakDuration': %1").arg(mThisOutbreakDuration));
            mOutbreakYears = 1;
            qCDebug(bite) << "Started the outbreak wave: outbreakYears=" << mOutbreakYears;
        }

    }

}


} // end namespace
