#include "scripttree.h"
#include "expression.h"
#include "expressionwrapper.h"
#include <QJSValue>
#include <QJSEngine>


void ScriptTree::addToScriptEngine(QJSEngine &engine)
{
    QJSValue jsMetaObject = engine.newQMetaObject(&ScriptTree::staticMetaObject);
    engine.globalObject().setProperty("Tree", jsMetaObject);

}

ScriptTree::ScriptTree(QObject *parent) : QObject(parent)
{
    mTree = nullptr;
}

QString ScriptTree::info()
{
    // return some information
    if (!mTree) return QString("invalid tree");
    QString s;
    s.sprintf("%08p", mTree);
    return QString("%1 (%7): %2 (%3cm, %4m, at %5/%6)").arg(s).arg(species()).arg(dbh()).arg(height()).arg(x()).arg(y()).arg(mTree->id());
}

double ScriptTree::expr(QString expr_str)
{
    TreeWrapper tw(mTree);
    Expression dexp(expr_str, &tw);
    if (!mTree) {
        qWarning() << "ScriptTree::expr: invalid tree!";
        return -1.;
    }
    try {
       return dexp.execute();
    } catch (const IException &e) {
        qWarning() << "ScriptTree: error in expression: " << e.message();
        return -1.;
    }

}

ScriptTreeExpr::ScriptTreeExpr(QString expr)
{
    setExpression(expr);
}

void ScriptTreeExpr::addToScriptEngine(QJSEngine &engine)
{
    QJSValue jsMetaObject = engine.newQMetaObject(&ScriptTreeExpr::staticMetaObject);
    engine.globalObject().setProperty("TreeExpr", jsMetaObject);
}

double ScriptTreeExpr::value(ScriptTree *script_tree)
{
    if (!script_tree->tree()) {
        qWarning() << "ScriptTreeExpr::value: invalid tree!";
        return -1.;
    }

    mTW.setTree(script_tree->tree());
    try {
       return mExpression.execute();
    } catch (const IException &e) {
        qWarning() << "ScriptTree: error in expression: " << e.message();
        return -1.;
    }
}
