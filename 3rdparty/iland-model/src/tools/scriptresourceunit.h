#ifndef SCRIPTRESOURCEUNIT_H
#define SCRIPTRESOURCEUNIT_H

#include <QObject>
#include "resourceunit.h"

class ScriptResourceUnit : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(int index READ index)
    Q_PROPERTY(bool valid READ valid)

public:
    explicit ScriptResourceUnit(QObject *parent = nullptr);
    void setRU(ResourceUnit *ru) { mRU = ru; }
    const ResourceUnit *ru() { return mRU; }
    void clear() { mRU = nullptr; }


    bool valid() { return mRU!=nullptr; }
    int id() { return mRU ? mRU->id() : -1; }
    int index() {return mRU ? mRU->index() : -1; }
signals:

public slots:
    QString info();
    double expr(QString expr_str);


private:
    ResourceUnit *mRU;
};

#endif // SCRIPTRESOURCEUNIT_H
