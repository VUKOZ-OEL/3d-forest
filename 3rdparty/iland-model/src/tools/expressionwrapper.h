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

#ifndef EXPRESSIONWRAPPER_H
#define EXPRESSIONWRAPPER_H
#include <QtCore/QString>
/** ExpressionWrapper is the base class for exposing C++ elements
 *  to the built-in Expression engine. See TreeWrapper for an example.
*/
class ExpressionWrapper
{
public:
    ExpressionWrapper();
    virtual ~ExpressionWrapper() {}
    virtual const QStringList getVariablesList();
    virtual double value(const int variableIndex);
    virtual double valueByName(const QString &variableName);
    virtual int variableIndex(const QString &variableName);
};

/** TreeWrapper wraps an individual tree in iLand.
 *
 **/
class Tree;
class TreeWrapper: public ExpressionWrapper
{
public:
    TreeWrapper() : mTree(nullptr) {}
    TreeWrapper(const Tree* tree) : mTree(tree) {}
    void setTree(const Tree* tree) { mTree = tree; }
    virtual const QStringList getVariablesList();
    virtual double value(const int variableIndex);

private:
    const Tree *mTree;
};

/** RUWrapper encapsualates an iLand resource unit (1 ha pixel)
*/
class ResourceUnit;
class RUWrapper: public ExpressionWrapper
{
public:
    RUWrapper() : mRU(nullptr) {}
    RUWrapper(const ResourceUnit* resourceUnit) : mRU(resourceUnit) {}
    void setResourceUnit(const ResourceUnit* resourceUnit) { mRU = resourceUnit; }
    virtual const QStringList getVariablesList();
    virtual double value(const int variableIndex);

private:
    const ResourceUnit *mRU;
};

/** SaplingWrapper encapsualates a sapling cohort (on a 2x2m pixel)
  */
struct SaplingTree;
class SaplingWrapper: public ExpressionWrapper
{
public:
    SaplingWrapper() : mSapling(nullptr) {}
    SaplingWrapper(const SaplingTree* saplingTree, const ResourceUnit *ru) : mSapling(saplingTree), mRU(ru) {}
    void setSaplingTree(const SaplingTree* saplingTree, const ResourceUnit *ru) { mSapling = saplingTree; mRU=ru; }
    virtual const QStringList getVariablesList();
    virtual double value(const int variableIndex);

private:
    const SaplingTree *mSapling;
    const ResourceUnit *mRU;
};

/** SaplingWrapper encapsualates a sapling cohort (on a 2x2m pixel)
  */
class DeadTree;
class DeadTreeWrapper: public ExpressionWrapper
{
public:
    DeadTreeWrapper()  {}
    DeadTreeWrapper(const DeadTree* deadTree) : mDeadTree(deadTree) {}
    void setDeadTree(const DeadTree* deadTree) { mDeadTree= deadTree; }
    virtual const QStringList getVariablesList();
    virtual double value(const int variableIndex);

private:
    const DeadTree *mDeadTree {nullptr};
};

#endif // EXPRESSIONWRAPPER_H
