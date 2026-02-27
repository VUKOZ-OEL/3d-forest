#include "fmsaplinglist.h"

#include "resourceunit.h"
#include "expressionwrapper.h"

#include <QJSEngine>
namespace ABE {

FMSaplingList::FMSaplingList(QObject *parent) : QObject(parent)
{

}

int FMSaplingList::loadFromRect(ResourceUnit *ru, const QRectF &rect)
{

    mSaplings.clear();
    // loop over all LIF grid cells of the resource unit, and load all occupied saplings
    GridRunner<float> lif_runner(GlobalSettings::instance()->model()->grid(), rect);
    while (lif_runner.next()) {
        SaplingCell *cell = ru->saplingCell(lif_runner.currentIndex());
        for (int j=0;j<NSAPCELLS;++j) {
            if (cell->saplings[j].is_occupied()) {
                mSaplings.push_back(QPair<SaplingTree*, SaplingCell*>( &(cell->saplings[j]), cell));
            }
        }

    }
    return mSaplings.size();

}

int FMSaplingList::loadFromRU(ResourceUnit *ru, bool append)
{
    if (!append)
        mSaplings.clear();

    SaplingCell *sapcell = ru->saplingCellArray();
    for (int i=0;i<cPxPerHectare;++i, ++sapcell) {
        for (int j=0;j<NSAPCELLS;++j) {
            if (sapcell->saplings[j].is_occupied()) {
                mSaplings.push_back(QPair<SaplingTree*, SaplingCell*>( &(sapcell->saplings[j]), sapcell));
            }
        }
    }
    return mSaplings.size();
}

void FMSaplingList::addToScriptEngine(QJSEngine *engine)
{
    QJSValue jsMetaObject = engine->newQMetaObject(&ABE::FMSaplingList::staticMetaObject);
    engine->globalObject().setProperty("SaplingList", jsMetaObject);

}

int FMSaplingList::loadFromStand(int standId, QString filter, bool do_append)
{
    if (!do_append)
        mSaplings.clear();

    SaplingWrapper sw;
    Expression filter_expr(filter.isEmpty() ? "true" : filter, &sw);

    SaplingCellRunner scr(standId, GlobalSettings::instance()->model()->standGrid());
    while (SaplingCell *sc = scr.next()) {
        if (sc){
            for (int i=0;i<NSAPCELLS;++i) {
                if (sc->saplings[i].is_occupied()) {
                    sw.setSaplingTree(&sc->saplings[i], sc->ru);
                    if (filter.isEmpty() || filter_expr.execute()) {
                        mSaplings.push_back(QPair<SaplingTree*, SaplingCell*>( &(sc->saplings[i]), sc));
                    }
                }
            }
        }
    }

    return mSaplings.size();

}

int FMSaplingList::filter(QString filter)
{
    if (filter.isEmpty())
        return mSaplings.size();

    SaplingWrapper sw;
    Expression expression(filter, &sw);
    expression.enableIncSum();
    QPair<SaplingTree*, SaplingCell*> empty_tree(nullptr,nullptr);

    for (int i=0;i<mSaplings.size();++i) {
        sw.setSaplingTree(mSaplings[i].first, mSaplings[i].second->ru);
        if (!expression.execute())
            mSaplings[i] = empty_tree; // mark for removal
    }
    int n_rem = mSaplings.removeAll(empty_tree);
    if (logLevelDebug())
        qDebug() << "apply sapling filter" << filter << ", removed" << n_rem;
    return mSaplings.size();

}

int FMSaplingList::kill(QString filter)
{
    int nsap_removed = 0;
    SaplingWrapper sw;
    Expression expression(filter.isEmpty() ? QStringLiteral("true") : filter, &sw);
    for (int i=0;i<mSaplings.size();++i) {
        sw.setSaplingTree(mSaplings[i].first, mSaplings[i].second->ru);
        if (expression.execute()) {
            mSaplings[i].first->clear();
            mSaplings[i].second->checkState();
            nsap_removed++;
        }
    }
    return nsap_removed;

}

int FMSaplingList::browse(bool do_browse)
{
    for (int i=0;i<mSaplings.size();++i) {
        mSaplings[i].first->set_browsed(do_browse);
    }
    return mSaplings.length();
}

double FMSaplingList::sum(QString expression, QString filter)
{
    double running_sum = 0.;
    SaplingWrapper sw;
    Expression filterexpr(filter, &sw);
    Expression expr(expression, &sw);
    for (int i=0;i<mSaplings.size();++i) {
        sw.setSaplingTree(mSaplings[i].first, mSaplings[i].second->ru);
        if (filterexpr.isEmpty() || filterexpr.execute()) {
            running_sum += expr.execute();
        }
    }
    return running_sum;

}


}; // end namespace
