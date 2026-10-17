#ifndef SCRIPTTREE_H
#define SCRIPTTREE_H

#include <QObject>
#include "tree.h"
#include "species.h"
#include "expressionwrapper.h"

class ScriptTree : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid)
    Q_PROPERTY(double x READ x)
    Q_PROPERTY(double y READ y)
    Q_PROPERTY(double dbh READ dbh)
    Q_PROPERTY(double height READ height)
    Q_PROPERTY(QString species READ species)
    Q_PROPERTY(int flags READ flags)
public:
    enum TreeRemovalType { RemovedDeath=1, RemovedHarvest=2, RemovedDisturbance=4, RemovedSalavaged=8, RemovedKilled=16, RemovedCutDown=32}; // the same enum as in tree.h, but encoded binary
    //enum TreeRemovalType { TreeDeath=0, TreeHarvest=1, TreeDisturbance=2, TreeSalavaged=3, TreeKilled=4, TreeCutDown=5}; // the  enum in tree.h
    Q_ENUM(TreeRemovalType)
    // selected tree flags from tree.h
    enum Flags { TreeDead=1,
                 TreeDeadBarkBeetle=16, TreeDeadWind=32, TreeDeadFire=64, TreeDeadKillAndDrop=128, TreeHarvested=256,
                 TreeAffectedBite=8192 // affected or killed by biotic disturbance module (BITE)
               };
    Q_ENUM(Flags)
    static void addToScriptEngine(QJSEngine &engine);

    explicit ScriptTree(QObject *parent = nullptr);
    void setTree(Tree *t) { mTree = t; }
    const Tree *tree() { return mTree; }
    void clear() { mTree = nullptr; }

    bool valid() const { return mTree != nullptr; }
    double x() const {  return mTree ? mTree->position().x() : -1.;    }
    double y() const {  return mTree ? mTree->position().y() : -1;    }
    double dbh() const {  return mTree ? static_cast<double>(mTree->dbh()) : -1.;    }
    double height() const {  return mTree ? static_cast<double>(mTree->height()) : -1.;    }
    QString species() const { return mTree ? mTree->species()->id() : QStringLiteral("invalid"); }
    int flags() const { return mTree ? mTree->flags() : 0; }

signals:

public slots:
    QString info();
    double expr(QString expr_str);

private:
    Tree *mTree;
};

// Expression class
class ScriptTreeExpr : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString expression READ expression WRITE setExpression)
public:
    Q_INVOKABLE ScriptTreeExpr(QString expr);
    static void addToScriptEngine(QJSEngine &engine);
public slots:
    QString expression() { return mExpression.expression(); }
    void setExpression(QString expr) {  mExpression.setExpression(expr); mExpression.setModelObject(&mTW); }
    double value(ScriptTree *script_tree );
private:
    Expression mExpression;
    TreeWrapper mTW;
};

#endif // SCRIPTTREE_H
