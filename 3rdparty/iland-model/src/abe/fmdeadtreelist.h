#ifndef FMDEADTREELIST_H
#define FMDEADTREELIST_H

#include <QObject>
#include "scriptglobal.h"


#include "deadtree.h"

class ResourceUnit; // forward
namespace ABE {

class FMDeadTreeList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ length)

public:
    enum class DeadTreeType {
       Snags,
       DWD,
       Both
    };
    Q_ENUM(DeadTreeType);
    Q_INVOKABLE explicit FMDeadTreeList(QObject *parent = nullptr);
    QVector<DeadTree* > &deadTrees() { return mDeadTrees; }
    int length() { return mDeadTrees.length(); }

    static void addToScriptEngine(QJSEngine *engine);

    /// load all dead trees from a resource unit. Specify
    /// whether to load snags, downed dead wood, or both with 'loadWhat', and
    /// set append=true to append to the list of dead trees
    int loadFromRU(ResourceUnit *ru, DeadTreeType loadWhat, bool append=false);

signals:

public slots:
    // load functions
    /// load all dead trees from a resource unit. Specify
    /// whether to load snags, downed dead wood, or both with 'loadWhat', and
    /// set append=true to append to the list of dead trees
    int loadFromRU(int ru_index, DeadTreeType loadWhat, bool append=false);
    /// load all dead trees from a stand denoted by 'stand_id'
    int loadFromStand(int stand_id, DeadTreeType loadWhat, QString filter=QString());

    /// load dead trees from a portion of a RU
    /// load both snags and DWD as default
    int loadFromRect(ResourceUnit *ru, const QRectF &rect);


    // actions
    /// remove all snags / SWD list items from iLand
    int remove();

    // access
    /// filter list according th the expression provided as 'filter'
    int filter(QString filter);

    /// calculate the mean value for all trees in the internal list for 'expression' (filtered by the filter criterion)
    double mean(QString expression, QString filter=QString()) { return aggregateFunction(expression, filter, "mean");}
    /// calculate the sum for all trees in the internal list for the 'expression' (filtered by the filter criterion)
    double sum(QString expression, QString filter=QString()) { return aggregateFunction(expression, filter, "sum");}


private:
    double aggregateFunction(QString expression, QString filter, QString what);
    QVector<DeadTree* > mDeadTrees;
};

} // end namespace ABE

#endif // FMDEADTREELIST_H
