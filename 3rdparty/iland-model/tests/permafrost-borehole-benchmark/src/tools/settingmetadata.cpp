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

void SettingMetaData::checkXMLFile(const QString fileName)
{
    XmlHelper xml; ///< xml-based hierarchical settings
    try {
    xml.loadFromFile(fileName);
    } catch (const IException &e) {
        qDebug() << "The file" << fileName << "does not exist or is not a valid XML file: " << e.message();
        return;
    }

    QStringList exceptions = QStringList() << "model.species.nitrogenResponseClasses.class" << "model.settings.seedDispersal.seedBelt.species" << "user";
    // load from resource file
    QSettings set(":/project_file_metadata.txt", QSettings::IniFormat);
    QStringList existingKeys = set.allKeys();


    // Check for keys in XML - File
    qDebug() << "Missing keys (Keys defined by iLand, missing in XML file)";
    qDebug() << "=========================================================";
    int n_not_found = 0;
    for (int i=0;i<existingKeys.size();++i) {
        bool has_key = xml.hasNode(existingKeys[i]);
        if (!has_key) {
            qDebug() << existingKeys[i] << ":" << set.value(existingKeys[i]).toString();
            ++n_not_found;
        }
    }
    qDebug() << "===============================================";
    qDebug() << "Number of keys not found in XML: " << n_not_found;
    qDebug() << "===============================================";
    qDebug() << "";

    qDebug() << "Invalid keys (keys in XML file, but not valid in iLand)";
    qDebug() << "=======================================================";

    // check for keys in XML but not in the list of allowed keys
    n_not_found = 0;
    QStringList xmlkeys = xml.dump("");
    for (QStringList::Iterator it=xmlkeys.begin(); it!=xmlkeys.end(); ++it) {
        QString key = it->mid(8, it->indexOf(QChar(':'))-8); // skip 'project.' (8 char)
        if (!existingKeys.contains(key) && key!=" ") {
            // check for exceptions
            bool is_exc = false;
            for (QStringList::ConstIterator s=exceptions.constBegin(); s!=exceptions.constEnd(); ++s)
                if (key.startsWith(*s))
                    is_exc=true;
            if (!is_exc) {
                qDebug() << key;
                ++n_not_found;
            }
        }

    }
    qDebug() << "===============================================";
    qDebug() << "Number of invalid keys in XML: " << n_not_found;
    qDebug() << "===============================================";



}
