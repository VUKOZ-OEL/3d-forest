#ifndef BITEOUTPUTITEM_H
#define BITEOUTPUTITEM_H

#include "biteitem.h"
#include "bitecellscript.h"
#include "biteoutput.h"

namespace BITE {


class BiteOutputItem: public BiteItem
{
public:
    Q_OBJECT

    Q_INVOKABLE BiteOutputItem(QJSValue obj);
    ~BiteOutputItem();
    void setup(BiteAgent *parent_agent);
    QString info();
public slots:
    void runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist);


protected:
    QStringList allowedProperties();
private:
    DynamicExpression mOutputFilter;
    Events mEvents;
    BiteCellOutput *mOutput;
    QString mTableName;
    QStringList mColumns;

};

} // namespace

#endif // BITEOUTPUTITEM_H
