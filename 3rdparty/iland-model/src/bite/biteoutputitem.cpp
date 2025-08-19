#include "biteoutputitem.h"

#include "biteengine.h"

#include "outputmanager.h"

namespace BITE {


BiteOutputItem::BiteOutputItem(QJSValue obj): BiteItem(obj)
{

}

BiteOutputItem::~BiteOutputItem()
{
    if (!mTableName.isEmpty())
        GlobalSettings::instance()->outputManager()->removeOutput(mTableName);
}

void BiteOutputItem::setup(BiteAgent *parent_agent)
{
    BiteItem::setup(parent_agent);
    try {
        checkProperties(mObj);

        mTableName = BiteEngine::valueFromJs(mObj, "tableName", "", "Property 'tableName' is required!").toString();
        QJSValue columns = BiteEngine::valueFromJs(mObj, "columns", "", "Property 'columns' is required!");
        if (!columns.isArray())
            throw IException("Property 'columns' is not an array.");
        QJSValueIterator it(columns);
        while (it.hasNext()) {
            it.next();
            if (it.name()==QStringLiteral("length"))
                continue;

            mColumns.push_back(it.value().toString());
        }

        //GlobalSettings::instance()->outputManager()->removeOutput(mTableName);
        if (GlobalSettings::instance()->outputManager()->find(mTableName)) {
            throw IException(QString("The table name of the bite output '%1' must be unique but is already used.").arg(mTableName));
        }
        mOutput = new BiteCellOutput();
        mOutput->setupBite(mColumns, mTableName);
        GlobalSettings::instance()->outputManager()->addOutput(mOutput);

        QJSValue filter = BiteEngine::valueFromJs(mObj, "outputFilter");
        if (!filter.isUndefined())
            mOutputFilter.setup(filter, DynamicExpression::CellWrap, parent_agent);

        mEvents.setup(mObj, QStringList() << "onOutput" , agent());

        // the output require cell level stats
        agent()->createStatsGrid();


    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of agent %3 in BiteOutput item '%1': %2").arg(name()).arg(e.message(), parent_agent->name());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);

    }
}

QString BiteOutputItem::info()
{
    QString res = QString("Type: BiteOutput\nDesc: %1").arg(description());
    return res;
}

void BiteOutputItem::runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist)
{
    Q_UNUSED(treelist)
    Q_UNUSED(saplist)
    bool filter = mOutputFilter.evaluateBool(cell);
    if (!filter)
        return;
    mOutput->execCell(cell, agent());
}

QStringList BiteOutputItem::allowedProperties()
{
    QStringList l = BiteItem::allowedProperties();
    l << "tableName" << "columns" << "outputFilter";
    return l;

}

} // namespace
