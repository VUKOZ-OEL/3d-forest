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

/** @class SettingMetaData
 This is some help text for the SettingMetaData class.
*/

#include <QtCore>
#include <QSettings>

#include "settingmetadata.h"

#include "xmlhelper.h"
#include "exception.h"

QStringList SettingMetaData::checkXMLKeys(const QString fileName, QStringList metaKeys)
{
    XmlHelper xml; ///< xml-based hierarchical settings
    try {
        xml.loadFromFile(fileName);
    } catch (const IException &e) {
        qDebug() << "The file" << fileName << "does not exist or is not a valid XML file: " << e.message();
        return {};
    }

    QStringList exceptions = QStringList() << "gui.layout";
    // load from resource file
    QSettings set(":/project_file_metadata.txt", QSettings::IniFormat);
    QStringList existingKeys = metaKeys;
    QStringList missingKeys = QStringList();

    // Check for keys in XML - File
//    qDebug() << "Missing keys (Keys defined by iLand, missing in XML file)";
//    qDebug() << "=========================================================";

    //for (int i=0;i<existingKeys.size();++i) {
    foreach (QString key, existingKeys) {
        bool has_key = xml.hasNode(key);
        if (!has_key && !exceptions.contains(key)) {
            //qDebug() << key; // << ":" << set.value(key).toString();
            // to avoid double counting of not existing keys (if there are mirrored elements)
            exceptions << key;
            missingKeys << key;
        }
    }
//    qDebug() << "===============================================";
//    qDebug() << "Number of keys not found in XML: " << missingKeys.size();
//    qDebug() << "===============================================";
//    qDebug() << "";

    return missingKeys;
}

void SettingMetaData::updateXMLFile(const QString fileName, QStringList missingKeys) {

    XmlHelper xml; ///< xml-based hierarchical settings
    try {
        xml.loadFromFile(fileName);
    } catch (const IException &e) {
        qDebug() << "The file" << fileName << "does not exist or is not a valid XML file: " << e.message();
        return;
    }

    int n_counter = 0;

    foreach (const QString& key, missingKeys) {
        if (xml.createNode(key)) {
            qDebug() << "Added " << key << " to XML file.";
            ++n_counter;
        } else {
            qDebug() << "Couldn't add " << key << " to XML file.";
        }
    }
    xml.saveToFile(fileName);
    qDebug() << "Successfully added " << n_counter << " keys to XML file.";
}

void SettingMetaData::checkXMLFile(const QString fileName, QStringList metaKeys)
{
    XmlHelper xml; ///< xml-based hierarchical settings
    try {
    xml.loadFromFile(fileName);
    } catch (const IException &e) {
        qDebug() << "The file" << fileName << "does not exist or is not a valid XML file: " << e.message();
        return;
    }
    QStringList existingKeys = metaKeys;
    QStringList exceptions = QStringList() << "gui.layout" << "model.species.nitrogenResponseClasses.class" << "model.settings.seedDispersal.seedBelt.species" << "user" << "model.species";

    QStringList missingKeys = checkXMLKeys(fileName, metaKeys);
    if (missingKeys.size() > 0) {
        qDebug() << "Missing keys (Keys defined by iLand, missing in XML file)";
        qDebug() << "=========================================================";
        foreach (const QString &key, missingKeys) {
            qDebug() << key;
        }
        qDebug() << "===============================================";
        qDebug() << "Number of keys not found in XML: " << missingKeys.size();
        qDebug() << "===============================================";
        qDebug() << "\n\n";
    } else {
        qDebug() << "There are no missing keys, XML file is up-to-date.\n\n";
    }

    qDebug() << "Invalid keys (keys in XML file, but not valid in iLand)";
    qDebug() << "=======================================================";

    // check for keys in XML but not in the list of allowed keys
    int n_not_found = 0;
    QStringList xmlkeys = xml.dump("");
    foreach (QString rawKey, xmlkeys) {
        rawKey.truncate(rawKey.indexOf(":"));
        QString key = (rawKey.size() > 7) ? rawKey.sliced(8) : " ";
        if (!existingKeys.contains(key) && key!=" " && key.split(".").size() > 2) {
            // check for exceptions
            //qDebug() << "key " << key << " has children: " << xml.nodeHasChildren(key);
            bool is_exc = false;
            for (QStringList::ConstIterator s=exceptions.constBegin(); s!=exceptions.constEnd(); ++s)
                if (key.startsWith(*s))
                    is_exc=true;
            if (!is_exc) {
                if (!xml.nodeHasChildren(key)) {
                    qDebug() << key;
                    ++n_not_found;
                }
            }
        }

    }
    qDebug() << "===============================================";
    qDebug() << "Number of invalid keys in XML: " << n_not_found;
    qDebug() << "===============================================";



}

void SettingMetaData::loadFromFile(const QString &metaFilePath,
                                   QStringList &keys,
                                   QStringList &values)
{
//    QDir parentDir = QDir::current();
//    QDir::
    QFile file(metaFilePath);

    if (file.open(QIODevice::ReadOnly)) {
        keys.clear();
        values.clear();

        QTextStream inStream(&file);
        QString line, key, value;
        QStringList keyValuePair;
        int indexDel;

        while (!inStream.atEnd()) {
            line = inStream.readLine();
            if (line.size() > 0 && !line.startsWith(";")) {
                // use different approach to split. If there are "=" somewhere in the
                // labels or tool tips line.split("=") messes the key-value pair up.
                //keyValuePair = line.split("=");
                indexDel = line.indexOf("=");
                key = line.sliced(0, indexDel).trimmed();
                value = line.mid(indexDel+1).trimmed();
                keys.append(key);
                values.append(value);
            }
        }

        file.close();
    }
    else {
        qDebug() << "File couldn't be opened. Abort.";
    }

}

