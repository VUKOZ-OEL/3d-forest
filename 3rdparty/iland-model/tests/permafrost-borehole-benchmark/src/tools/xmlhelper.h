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

#ifndef XMLHELPER_H
#define XMLHELPER_H

#include <QtXml>

class XmlHelper
{
public:
   XmlHelper();
   ~XmlHelper();
   XmlHelper(const QString &fileName) {loadFromFile(fileName);}
   XmlHelper(QDomElement topNode);
   void loadFromFile(const QString &fileName);
   void resetWarnings();
   void printSuppressedWarnings();
   // relative top nodes
   QDomElement top() const { return mTopNode;}
   void setCurrentNode(const QString &path) { mCurrentTop = node(path); } ///< sets @p path as the current (relative) node.
   void setCurrentNode(const QDomElement &node) { mCurrentTop = node; } ///< sets node as the current (relative) top node.
   /// returns true if the current (relative!) node is valid (i.e. not null).
   bool isValid() const { return !mCurrentTop.isNull(); }
   bool hasNode(const QString &path) const; ///< returns true if @p path exists.
    // read access
   QDomElement node(const QString &path) const; ///< retrieve node defined by path (see class description)
   QString value(const QString &path, const QString &defaultValue="") const; ///< retrieve value (as string) from node @p path.
   bool valueBool(const QString &path, const bool defaultValue=false) const; ///< retrieve value (as bool) from node @p path.
   double valueDouble(const QString &path, const double defaultValue=0.) const; ///< retrieve value (as double) from node @p path.
   int valueInt(const QString &path, const int defaultValue=0) const; ///< retrieve value (as int) from node @p path.
   // write access
   bool setNodeValue(QDomElement &node, const QString &value); ///< set value of 'node'. return true on success.
   bool setNodeValue(const QString &path, const QString &value); ///< set value of node indicated by 'path'. return true on success.
   // special parameters
   double paramValue(const QString &paramName, const double defaultValue=0.) const; ///< get value of special "parameter" space
   QString paramValueString(const QString &paramName, const QString &defaultValue="") const; ///< get value of special "parameter" space
   bool paramValueBool(const QString &paramName, const bool &defaultValue=true) const; ///< get value of special "parameter" space
    // helpers
   QStringList dump(const QString &path, int levels=-1);
private:
   void dump_rec(QDomElement c, QStringList &stack, QStringList &out);
   void missedKey(const QString &keyname) const;
   /// retrieve full name (. - notation)
   QString fullName(const QString &keyname) const;
   QDomDocument mDoc;
   QDomElement mCurrentTop;
   QDomElement mTopNode;
   QHash<const QString, QString> mParamCache;
   QHash<QString, int> mMissingKeys;
};

#endif // XMLHELPER_H
