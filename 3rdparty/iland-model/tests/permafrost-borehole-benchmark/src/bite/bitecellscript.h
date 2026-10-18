/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/
#ifndef BITECELLSCRIPT_H
#define BITECELLSCRIPT_H

#include <QObject>
#include <QJSValue>
#include <QMap>
#include "expression.h"
#include "bitecell.h"
#include "scripttree.h"

namespace ABE {
class FMTreeList;
class FMSaplingList;
}

namespace BITE {

class BiteCellScript : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive)
    Q_PROPERTY(bool spreading READ spreading WRITE setSpreading)
    Q_PROPERTY(int yearsLiving READ yearsLiving)
    Q_PROPERTY(int cumYearsLiving READ cumYearsLiving)
    Q_PROPERTY(int outbreakYears READ outbreakYears)
    Q_PROPERTY(ABE::FMTreeList* trees READ trees)
    Q_PROPERTY(ABE::FMSaplingList* saplings READ saplings)
    Q_PROPERTY(BiteAgent* agent READ agent)
public:
    explicit BiteCellScript(QObject *parent = nullptr);
    ~BiteCellScript() {}
    void setCell(BiteCell *c) { mCell = c; }
    BiteCell *cell() const {return mCell; }
    void setAgent(BiteAgent *a) { mAgent = a; }
    BiteAgent *agent() const { return mAgent; }

    bool active() const { return mCell->isActive(); }
    void setActive(bool a) { mCell->setActive(a); }

    bool spreading() const { return mCell->isSpreading(); }
    void setSpreading(bool a) { mCell->setSpreading(a); }

    int yearsLiving() const { return mCell->yearsLiving(); }
    int cumYearsLiving() const { return mCell->cumYearsLiving(); }
    int outbreakYears() const;

    ABE::FMTreeList *trees();
    ABE::FMSaplingList *saplings();


signals:

public slots:
    QString info();
    // access to variables of the cell
    bool hasValue(QString variable_name);
    double value(QString variable_name);
    void setValue(QString var_name, double value);

    void die() { mCell->die(); }

    void reloadTrees();
    void reloadSaplings();
private:
    BiteCell *mCell;
    BiteAgent *mAgent;
};


class Events {
public:
    Events() {}
    /// clear the list of events
    void clear();
    /// setup events from the javascript object
    void setup(QJSValue &js_value, QStringList event_names, BiteAgent *agent);
    /// execute javascript event /if registered) in the context of the forest stand 'stand'.
    QString run(const QString event, BiteCell *cell=nullptr, QJSValueList *params=nullptr);
    /// returns true, if the event 'event' is available.
    bool hasEvent(const QString &event) const;
    QJSValue eventFunction(const QString &event) { return mEvents[event].property(event); }
    QString dump(); ///< prints some debug info
private:
    QJSValue mInstance; ///< object holding the events
    QMap<QString, QJSValue> mEvents; ///< list of event names and javascript functions
    BiteCellScript mCell;
    QJSValue mScriptCell;
    BiteAgent *mAgent;
};

/** DynamicExpression encapsulates an "expression" that can be either a iLand expression, a constant or a javascript function.
*/
struct DynamicExpression {
    enum EWrapperType { CellWrap, TreeWrap, SaplingWrap } ;
    enum EFilterType { ftInvalid, ftExpression, ftJavascript, ftConstant};
    DynamicExpression(): wrapper_type(CellWrap), filter_type(ftInvalid), expr(nullptr), mAgent(nullptr), mTree(nullptr) {}
    DynamicExpression(const DynamicExpression &src);
    ~DynamicExpression();
    void setup(const QJSValue &js_value, EWrapperType type, BiteAgent *agent);
    EFilterType type() const {return filter_type; }
    double evaluate(BiteCell *cell) const;
    double evaluate(Tree* tree) const;
    double evaluate(SaplingTree* sap, ResourceUnit *ru) const;

    bool evaluateBool(BiteCell *cell) const { return evaluate(cell) > 0.; }
    bool evaluateBool(Tree *tree) const { return evaluate(tree) > 0.; }
    bool evaluateBool(SaplingTree *sap, ResourceUnit *ru) const { return evaluate(sap, ru) > 0.; }

    bool isValid() const { return filter_type!=ftInvalid;}
    bool isConst() const { return filter_type == ftConstant; }
    QString dump() const;
private:
    EWrapperType wrapper_type;
    EFilterType filter_type;
    Expression *expr;
    QJSValue func;
    BiteAgent *mAgent;
    double mConstValue;

    // value for trees
    QJSValue mTreeValue;
    ScriptTree *mTree;

    // value for cells
    BiteCellScript mCell;
    QJSValue mScriptCell;


};

class Constraints {
public:
    Constraints() {}
    ~Constraints();
    void setup(QJSValue &js_value, DynamicExpression::EWrapperType wrap, BiteAgent *agent); ///< setup from javascript
    double evaluate(BiteCell *cell); ///< run the constraints
    double evaluate(ABE::FMTreeList *treelist); ///< run for trees
    double evaluate(ABE::FMSaplingList *saplinglist); ///< run for saplings
    bool isConst(); ///< return true if no dynamic evaluation happens
    QStringList dump(); ///< prints some debug info
private:
    QList<DynamicExpression*> mConstraints;
    BiteAgent *mAgent;
};


} // end namespace
#endif // BITECELLSCRIPT_H
