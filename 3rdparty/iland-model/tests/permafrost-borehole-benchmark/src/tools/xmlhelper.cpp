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

#include "xmlhelper.h"
#include "helper.h"
#include "exception.h"


/** @class XmlHelper
  XmlHelper wraps a XML file and provides some convenient functions to
  retrieve values. Internally XmlHelper uses a QDomDocument (the full structure is
  kept in memory so the size is restricted).
  Use node() to get a QDomElement or use value() to directly retrieve the node value.
  Nodes could be addressed relative to a node defined by setCurrentNode() using a ".".
  The notation is as follows:
  - a '.' character defines a hierarchy
  - [] the Nth element of the same hierarchical layer can be retrieved by [n-1]
  Use also the convenience functions valueBool() and valueDouble().
  While all the value/node etc. functions parse the DOM tree at every call, the data accessed by paramValue() - type
  functions is parsed only once during startup and stored in a QVariant array. Accessible are all nodes that are children of the
   "<parameter>"-node.

  @code
  QDomElement e,f
  e = xml.node("first.second.third"); // e points to "third"
  xml.setCurrentNode("first");
  f = xml.node(".second.third"); // e and f are equal
  e = xml.node("level1[2].level2[3].level3"); // 3rd element of level 1, ...
  int x = xml.value(".second", "0").toInt(); // node value of "second" or "0" if not found.
  if (xml.valueBool("enabled")) // use of valueBool with default value (false)
     ...
  XmlHelper xml_sec(xml.node("first.second")); // create a xml-helper with top node=first.second
  xml_sec.valueDouble("third"); // use this
  @endcode

  */

XmlHelper::XmlHelper()
{
}
XmlHelper::~XmlHelper()
{
    //qDebug() << "xml helper destroyed";
}
/** Create a XmlHelper instance with @p topNode as top node.
  The xml tree is not copied.
*/
XmlHelper::XmlHelper(QDomElement topNode)
{
    mTopNode = topNode;
    mCurrentTop = topNode;
}

void XmlHelper::loadFromFile(const QString &fileName)
{
    mDoc.clear();
    QString xmlFile = Helper::loadTextFile(fileName);

    if (!xmlFile.isEmpty()) {

    QString errMsg;
    int errLine, errCol;
        if (!mDoc.setContent(xmlFile, &errMsg, &errLine, &errCol)) {
            throw IException(QString("Error in xml-file!\nError applying xml line %1, col %2.\nMessage: %3").arg(errLine).arg(errCol).arg(errMsg));
        }
    } else {
         throw IException("xmlfile does not exist or is empty!");
    }
    mCurrentTop = mDoc.documentElement(); // top element
    mTopNode = mCurrentTop;

    // fill parameter cache
    QDomElement e = node("model.parameter");
    e = e.firstChildElement();
    mParamCache.clear();
    while (!e.isNull()) {
        mParamCache[e.nodeName()] = e.text();
        e = e.nextSiblingElement();
    }
}

void XmlHelper::resetWarnings()
{
    mMissingKeys.clear();
}

void XmlHelper::printSuppressedWarnings()
{
    QHash<QString, int>::const_iterator i = mMissingKeys.constBegin();
    int n=0;
    while (i != mMissingKeys.constEnd()) {
        if (i.value()>3)
            n++;
        ++i;
    }
    if (n==0)
        return;

    qDebug() << "Settings not found in project file (more often than 3 times):";
    qDebug() << "=============================================================";
    i = mMissingKeys.constBegin();
    while (i != mMissingKeys.constEnd()) {
        if (i.value()>3)
            qDebug() << i.key() << ":" << i.value() << "times";
        ++i;
    }

}

/** numeric values of elements in the section <parameter> are stored in a QHash structure for faster access.
    with paramValue() these data can be accessed.
  */
double XmlHelper::paramValue(const QString &paramName, const double defaultValue) const
{
    if (mParamCache.contains(paramName))
        return mParamCache.value(paramName).toDouble();
    return defaultValue;
}
QString XmlHelper::paramValueString(const QString &paramName, const QString &defaultValue) const
{
    if (mParamCache.contains(paramName))
        return mParamCache.value(paramName);
    return defaultValue;
}

bool XmlHelper::paramValueBool(const QString &paramName, const bool &defaultValue) const
{
    if (mParamCache.contains(paramName)) {
        QString v = mParamCache.value(paramName).trimmed();
        bool ret = (v=="1" || v=="true");
        return ret;
    }
    return defaultValue;
}

bool XmlHelper::hasNode(const QString &path) const
{
    return !node(path).isNull();
}

QString XmlHelper::value(const QString &path, const QString &defaultValue) const
{
    QDomElement e = node(path);
    if (e.isNull()) {
        missedKey(path);
        return defaultValue;
    } else {
        if (e.text().isEmpty())
            return defaultValue;
        else
            return e.text();
    }
}
bool XmlHelper::valueBool(const QString &path, const bool defaultValue) const
{
    QDomElement e = node(path);
    if (e.isNull()) {
        missedKey(path);
        return defaultValue;
    }
    QString v = e.text();
    if (v=="true" || v=="True" || v=="1")
        return true;
    else
        return false;
}
double XmlHelper::valueDouble(const QString &path, const double defaultValue) const
{
    QDomElement e = node(path);
    if (e.isNull()) {
        missedKey(path);
        return defaultValue;
    } else {
        if (e.text().isEmpty())
            return defaultValue;
        else
            return e.text().toDouble();
    }
}

int XmlHelper::valueInt(const QString &path, const int defaultValue) const
{
    double dbl_val = valueDouble(path, defaultValue);
    return static_cast<int>(dbl_val);
}

/// retreives node with given @p path and a element where isNull() is true if nothing is found.
QDomElement XmlHelper::node(const QString &path) const
{
    QStringList elem = path.split('.', QString::SkipEmptyParts);
    QDomElement c;
    if (path.count()>0 && path.at(0) == '.')
        c = mCurrentTop;
    else
        c = mTopNode;
    foreach (QString level, elem) {
        if (level.indexOf('[')<0) {
            c = c.firstChildElement(level);
            if (c.isNull())
                break;
        } else {
            int pos = level.indexOf('[');
            level.chop(1); // drop closing bracket
            int ind = level.rightRef( level.length() - pos -1).toInt();
            QString name = level.left(pos);
            c = c.firstChildElement(name);
            while (ind>0 && !c.isNull()) {
                c = c.nextSiblingElement();
                ind--;
            }
            if (c.isNull())
                break;
        }
    }
    //qDebug() << "node-request:" << path;
    return c;
}

// writers
bool XmlHelper::setNodeValue(QDomElement &node, const QString &value)
{
    if (!node.isNull() && node.hasChildNodes()) {
        node.firstChild().toText().setData(value);
        return true;
    }
    return false;
}
bool XmlHelper::setNodeValue(const QString &path, const QString &value)
{
    QDomElement e = node(path);
    if (e.isNull()) {
        qDebug() << "XML: attempting to set value of" << path << ": node not present.";
        return false;
    }
    return setNodeValue(e,value);
}

QMutex xml_mutex;
void XmlHelper::missedKey(const QString &keyname) const
{
    // make the hash non const (for updating it), therefore add a lock
    QMutexLocker lock(&xml_mutex);
    QHash<QString, int> &keys = const_cast<QHash<QString, int>& >(mMissingKeys);

    QString key = fullName(keyname);
    keys[key]++;

    int n=keys[key];
    if (n<3)
        qDebug() << "Warning: xml: node" << key << "is not present.";
    if (n==3)
        qDebug() << "Warning: xml: node" << key << "is not present (3rd occurrence, suppressed in the future).";
}

QString XmlHelper::fullName(const QString &keyname) const
{
    QStringList elem = keyname.split('.', QString::SkipEmptyParts);

    if (keyname.count()==0 && keyname.at(0) != '.')
        return keyname;

    // we have a relative path
    QString result = keyname;
    result.remove(0,1); // remove leading "."

    QDomNode c= mCurrentTop;
    // loop over parents until we reach the top-node ("project")
    while (!c.isNull() && c.nodeName()!="project") {
        result = c.nodeName() + "." + result;
        c = c.parentNode();
    }
    return result;
}

// private recursive loop
void XmlHelper::dump_rec(QDomElement c, QStringList &stack, QStringList &out)
{
    if (c.isNull())
        return;
    QDomElement ch = c.firstChildElement();
    bool hasChildren = !ch.isNull();
    bool nChildren = !ch.isNull() && !ch.nextSiblingElement().isNull();
    int child_index=-1;
    while (!ch.isNull()) {
        if (nChildren) {
            child_index++;
            // stack.push_back(QString("%1[%2]").arg(ch.nodeName()).arg(child_index)); // including child index
            stack.push_back(QString("%1").arg(ch.nodeName()));
        } else
          stack.push_back(ch.nodeName());
        dump_rec(ch, stack, out);
        stack.pop_back();
        ch = ch.nextSiblingElement();
    }
    QString self;
    if (!hasChildren)
        self = c.text();
    self = QString("%1: %3").arg(stack.join("."), self);
    out.push_back(self);
}


QStringList XmlHelper::dump(const QString &path, int levels)
{
    Q_UNUSED(levels);
    QDomElement c = node(path);

    QStringList stack;
    stack.push_back(c.nodeName());
    QStringList result;
    dump_rec(c, stack, result);
    return result;
}
