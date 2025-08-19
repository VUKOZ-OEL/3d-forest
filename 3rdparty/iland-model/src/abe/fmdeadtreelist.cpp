#include "fmdeadtreelist.h"

#include "snag.h"
#include "resourceunit.h"
#include "model.h"
#include "expression.h"
#include "expressionwrapper.h"
#include "mapgrid.h"

#include <QJSEngine>

ABE::FMDeadTreeList::FMDeadTreeList(QObject *parent)
    : QObject{parent}
{}

int ABE::FMDeadTreeList::loadFromRU(ResourceUnit *ru, DeadTreeType loadWhat, bool append)
{
    auto &dt_list = ru->snag()->deadTrees();
    if (!append)
        mDeadTrees.clear();

    for (auto &dt : dt_list) {
        if ( (dt.isStanding() && loadWhat != DeadTreeType::DWD) ||
            (!dt.isStanding() && loadWhat != DeadTreeType::Snags) )
            mDeadTrees.push_back(&dt);
    }
    return length();
}

int ABE::FMDeadTreeList::loadFromRU(int ru_index, DeadTreeType loadWhat, bool append)
{
    ResourceUnit *ru = Globals->model()->ru(ru_index);
    if (!ru)
        throw IException("Invalid resource unit index: " + QString::number(ru_index));
    return loadFromRU(ru, loadWhat, append);
}

int ABE::FMDeadTreeList::loadFromStand(int stand_id, DeadTreeType loadWhat, QString filter)
{
    auto *stand_grid = Globals->model()->standGrid();
    stand_grid->loadDeadTrees(stand_id, mDeadTrees,  filter );
    // now filter either snags or DWD:
    switch (loadWhat) {
    case DeadTreeType::DWD: mDeadTrees.removeIf([](DeadTree *dt) { return dt->isStanding(); } ); break;
    case DeadTreeType::Snags: mDeadTrees.removeIf([](DeadTree *dt) {return !dt->isStanding(); }); break;
    default: break;
    }

    return mDeadTrees.size();
}

int ABE::FMDeadTreeList::loadFromRect(ResourceUnit *ru, const QRectF &rect)
{
    auto &dt_list = ru->snag()->deadTrees();
    mDeadTrees.clear();

    for (auto &dt : dt_list) {
        if ( rect.contains( QPointF(dt.x(), dt.y()) ) )
            mDeadTrees.push_back(&dt);
    }
    return length();

}

int ABE::FMDeadTreeList::remove()
{
    int removed = 0;
    QSet<ResourceUnit*> to_pack;
    for (auto &dt : mDeadTrees) {
        // find resource unit:
        QPointF coord(QPointF(dt->x(), dt->y()));
        ResourceUnit *ru = GlobalSettings::instance()->model()->ru(coord);
        if (ru && ru->snag()) {
            for (auto &d : ru->snag()->deadTrees())
                if ( &d == dt) {
                    d.setToBeRemoved();
                    to_pack.insert(ru);
                    ++removed;
                }

        }

    }
    for (auto ru : to_pack)
        ru->snag()->packDeadTreeList();

    return removed;
}

int ABE::FMDeadTreeList::filter(QString filter)
{
    if (filter.isEmpty())
        return mDeadTrees.size();

    DeadTreeWrapper dw;
    Expression expression(filter, &dw);
    expression.enableIncSum();

    for (auto &dt : mDeadTrees) {
        dw.setDeadTree(dt);
        if (!expression.execute())
            dw = nullptr; // mark for removal
    }
    int n_rem = mDeadTrees.removeAll(nullptr);
    if (logLevelDebug())
        qDebug() << "deadtrees:apply filter" << filter << ", removed" << n_rem;

    return mDeadTrees.size();


}

double ABE::FMDeadTreeList::aggregateFunction(QString expression, QString filter, QString what)
{
    DeadTreeWrapper tw;
    Expression filter_expr;
    if (!filter.isEmpty()) {
        filter_expr.setExpression(filter);
        filter_expr.setModelObject(&tw);
        filter_expr.enableIncSum();
    }
    Expression expr(expression, &tw);

    double value = 0.;
    int n = 0;
    for (auto &dt : mDeadTrees) {
        tw.setDeadTree(dt);
        if (filter_expr.isEmpty() ||
            filter_expr.executeBool()) {
            value += expr.execute();
            ++n;
        }
    }
    if (what == "sum")
        return value;
    if (what == "mean") {
        if (n>0)
            return value / double(n);
        else
            return 0;
    }
    throw IException("invalid aggregation.... grr");

}

void ABE::FMDeadTreeList::addToScriptEngine(QJSEngine *engine)
{
    QJSValue jsMetaObject = engine->newQMetaObject(&ABE::FMDeadTreeList::staticMetaObject);
    engine->globalObject().setProperty("DeadTreeList", jsMetaObject);

}
