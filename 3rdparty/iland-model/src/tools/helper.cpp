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

#include "helper.h"
#include "debugtimer.h"
#include <QtCore>
#ifdef ILAND_GUI
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif
#endif
//#include "cycle.h"
#include <limits>

#ifdef ILAND_GUI
#include <QColor>
#include "version.h"
#else
#include "version.h"
#endif

Helper::Helper()
{
}


QString Helper::loadTextFile(const QString& fileName)
{
    DebugTimer t("Helper::loadTextFile");
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }
    QTextStream s(&file);
    //s.setCodec("UTF-8");
    QString result=s.readAll();
    return result;
}

QStringList Helper::loadTextFileLines(const QString &fileName)
{
    DebugTimer t("Helper::loadTextFileLines");
    QStringList lines;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return lines;

    QTextStream in(&file);
    //lines.reserve(100000); // Or a more realistic estimate of expected lines
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return lines;
}

void Helper::saveToTextFile(const QString& fileName, const QString& text)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    QTextStream s(&file);
    s << text;
}
QByteArray Helper::loadFile(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();

//    QTextStream s(&file);

//    QByteArray result;
//    s >> result;

//    return result;
}

void Helper::saveToFile(const QString &fileName, const QByteArray &data)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    QTextStream s(&file);
    s << data;
}

/// ask the user for a input value
QString Helper::userValue(const QString &message, const QString defaultValue, QWidget *parent)
{
#ifdef ILAND_GUI
    bool ok;
    QString result = QInputDialog::getText(parent, "input data", message, QLineEdit::Normal, defaultValue, &ok);
    if (ok)
        return result;
    else
        return defaultValue;
#else
    Q_UNUSED(defaultValue); Q_UNUSED(message); Q_UNUSED(parent);
    return QString("not availabile in non-gui-mode");
#endif
}

void Helper::msg(const QString &message, QWidget *parent)
{
#ifdef ILAND_GUI
    QMessageBox::information(parent, "iLand", message);
#else
    Q_UNUSED(message); Q_UNUSED(parent);
#endif
}

bool Helper::question(const QString &message, QWidget *parent)
{
#ifdef ILAND_GUI
   return QMessageBox::question(parent, "iLand", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
#else
    Q_UNUSED(message); Q_UNUSED(parent);
    return false;
#endif
}

QString Helper::fileDialog(const QString &title, const QString &start_directory, const QString &filter, const QString& type, QWidget *parent)
{
#ifdef ILAND_GUI
    QString the_filter = filter;
    if (the_filter.isEmpty())
        the_filter = "All files (*.*)";
    else
        the_filter += ";;All files (*.*)"; // as 2nd filter
    QFileDialog dialog(parent);
    dialog.setFileMode(QFileDialog::Directory);

    QString fileName;

    if ( type == "directory") {
        fileName = dialog.getExistingDirectory(parent,
                                            title, start_directory);
    }
    else if (type == "file") {
        fileName = dialog.getOpenFileName(parent,
                                            title, start_directory, the_filter);
    }

#else
    Q_UNUSED(title); Q_UNUSED(start_directory); Q_UNUSED(filter); Q_UNUSED(parent);
    QString fileName="undefined";
#endif
    return fileName;
}

void Helper::openHelp(const QString& topic)
{
#ifdef ILAND_GUI
    QUrl url;
    qDebug() << "current path" << QDir::currentPath();
    url.setUrl(QString("file:///%1/help/%2.html").arg(QDir::currentPath(),topic) , QUrl::TolerantMode);
    qDebug() << url;
    if (url.isValid())
        qDebug() << "url is valid";
    QDesktopServices::openUrl(url);
#else
    Q_UNUSED(topic);
#endif
}

QString Helper::stripHtml(const QString &source)
{
    QString str = source.simplified();
    return str.replace(QRegularExpression("<[^>]+>"),"");
}


// debugging
bool Helper::m_quiet = true;
bool Helper::m_NoDebug = false;




/** UpdateState.

*/
void UpdateState::invalidate(bool self)
{
    if (self)
        mVal++;
    foreach (UpdateState *s, mChilds)
        s->invalidate(true);
}

void UpdateState::saveState(UpdateState *state)
{
    mSavedStates[state]=state->mVal;
}
bool UpdateState::hasChanged(UpdateState *state)
{
    if (!mSavedStates.contains(state))
        return true;
    qDebug() << "UpdateState::hasChanged: saved: " << mSavedStates[state] << "current: " << state->mVal;
    return mSavedStates[state] != state->mVal;
}
// set internal state to the current state
void UpdateState::update()
{
    mCurrentVal = mVal;
}
// check if state needs update
bool UpdateState::needsUpdate()
{
    return mVal > mCurrentVal;
}




