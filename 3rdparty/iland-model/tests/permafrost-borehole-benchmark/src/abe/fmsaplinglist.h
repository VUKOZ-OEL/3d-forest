#ifndef FMSAPLINGLIST_H
#define FMSAPLINGLIST_H

#include <QObject>

#include "saplings.h"

namespace ABE {


class FMSaplingList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ length)
public:
    explicit FMSaplingList(QObject *parent = nullptr);
    QVector<QPair<SaplingTree*, SaplingCell*> > &saplings() { return mSaplings; }
    int length() { return mSaplings.length(); }


    /// load saplings from a portion of a RU
    int loadFromRect(ResourceUnit *ru, const QRectF &rect);
    /// load all saplings from RU
    int loadFromRU(ResourceUnit *ru, bool append=false);

    // to load from a stand: see e.g. FMTreeList::aggregate_function_sapling(QString expression, QString filter, QString type) and SaplingCellRunner....

signals:

public slots:
    /// return the sum of 'expression' over all saplings in the list
    /// if 'filter' is provided, only trees are counted for which 'filter' evaluates to true.
    double sum(QString expression, QString filter=QLatin1Literal(""));

    /// apply a filter on the current list of saplings. Only saplings for which 'filter' returns true remain in the list.
    /// returns the number of sapplings that remain in the list.
    int filter(QString filter);

    /// kill all saplings in the list for which "filter" evaluates to true (or all if filter is omitted)
    /// return the number of killed cohorts
    int kill(QString filter = QLatin1Literal(""));

    /// affect all saplings by browsing -> this reduces the height growth in the current year to 0
    int browse(bool do_browse=true);

private:
    QVector<QPair<SaplingTree*, SaplingCell*> > mSaplings; ///< store a ptr to the sapling tree and the cell the tree is in
};

}; // end namespace

#endif // FMSAPLINGLIST_H
