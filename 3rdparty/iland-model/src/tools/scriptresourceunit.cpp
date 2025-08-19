#include "scriptresourceunit.h"
#include "expressionwrapper.h"
#include "expression.h"

ScriptResourceUnit::ScriptResourceUnit(QObject *parent) : QObject(parent)
{
    mRU=nullptr;
}

QString ScriptResourceUnit::info()
{
    // return some information
    if (!mRU) return QString("invalid resource unit!");
    QString s;
    s.asprintf("%8p", mRU);
    return QString("RU %1: id: %2 index: %3").arg(s).arg(id()).arg(index());

}

double ScriptResourceUnit::expr(QString expr_str)
{
    RUWrapper rw(mRU);
    Expression dexp(expr_str, &rw);
    if (!mRU) {
        qWarning() << "ScriptResourceUnit::expr: invalid RU!";
        return -1.;
    }
    try {
       return dexp.execute();
    } catch (const IException &e) {
        qWarning() << "ScriptResourceUnit: error in expression: " << e.message();
        return -1.;
    }

}
