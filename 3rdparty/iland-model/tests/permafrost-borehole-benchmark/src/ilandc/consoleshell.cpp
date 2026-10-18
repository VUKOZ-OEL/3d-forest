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

#include "consoleshell.h"

#include <QtCore>
#include <QtDebug>
//#include <QKeyEvent>

#include "global.h"
#include "model.h"
#include "modelcontroller.h"
#include "version.h"

QTextStream *ConsoleShell::mLogStream = 0;
bool ConsoleShell::mFlushLog = false;

// a try to really get keyboard strokes in console mode...
// did not work.
class KeyboardTaker : public QThread
 {
 public:
     void run();
     bool stop;
 };

 void KeyboardTaker::run()
 {
     stop = false;
     QTextStream qin(stdin, QFile::ReadOnly);
     while (!stop) {
         QString input = qin.read(1);
         if (!input.isNull()) {
             // .. process input
             qWarning() << "input:" << input;
         }
     }
 }

ConsoleShell::ConsoleShell()
{
}

/*
*/

void ConsoleShell::run()
{

    QString xml_name = QCoreApplication::arguments().at(1);
    // get the number of years to run...
    bool ok;
    int years = QCoreApplication::arguments().at(2).toInt(&ok);
    if (years<0 || !ok) {
        qDebug() << QCoreApplication::arguments().at(2) << "is an invalid number of years to run!";
        QCoreApplication::quit();
        return;
    }

    if (!QFile::exists(xml_name)) {
        qDebug() << "invalid XML project file: " << xml_name;
        QCoreApplication::quit();
        return;
    }
    try {

        ModelController iland_model;
        GlobalSettings::instance()->setModelController( &iland_model );
        QObject::connect(&iland_model, SIGNAL(year(int)),SLOT(runYear(int)));
        iland_model.setFileName(xml_name);
        if (iland_model.hasError()) {
            qWarning() << "!!!! ERROR !!!!";
            qWarning() << iland_model.lastError();
            qWarning() << "!!!! ERROR !!!!";
            QCoreApplication::quit();
            return;
        }

        mParams.clear();
        if (QCoreApplication::arguments().count()>3) {
            qWarning() << "set command line values:";
            for (int i=3;i<QCoreApplication::arguments().count();++i) {
                QString line = QCoreApplication::arguments().at(i);
                line = line.remove(QChar('"')); // drop quotes
                mParams.append(line);
                //qDebug() << qPrintable(line);
                QString key = line.left(line.indexOf('='));
                QString value = line.mid(line.indexOf('=')+1);
                const_cast<XmlHelper&>(GlobalSettings::instance()->settings()).setNodeValue(key, value);
                qWarning() << QString("set '%1' to value '%2'. result: '%3'").arg(key).arg(value).arg(GlobalSettings::instance()->settings().value(key));
            }
        }
        if (!setupLogging()) {
            qWarning() << "**************************************************";
            qWarning() << "********* Error in setup of logging **************";
            qWarning() << "**************************************************";
            QCoreApplication::quit();
            return;
        }

        qDebug() << "**************************************************";
        qDebug() << "***********     iLand console session     ********";
        qDebug() << "**************************************************";

        qWarning() << "*** creating model...";
        qWarning() << "**************************************************";

        iland_model.create();
        if (iland_model.hasError()) {
            qWarning() << "!!!! ERROR !!!!";
            qWarning() << iland_model.lastError();
            qWarning() << "!!!! ERROR !!!!";
            QCoreApplication::quit();
            return;
        }
        runJavascript("onCreate");
        qWarning() << "**************************************************";
        qWarning() << "*** running model for" << years << "years";
        qWarning() << "**************************************************";

        iland_model.run(years + 1);
        if (iland_model.hasError()) {
            qWarning() << "!!!! ERROR !!!!";
            qWarning() << iland_model.lastError();
            qWarning() << "!!!! ERROR !!!!";
            QCoreApplication::quit();
            return;
        }
        runJavascript("onFinish");

        qWarning() << "**************************************************";
        qWarning() << "*** model run finished.";
        qWarning() << "**************************************************";

    } catch (const IException &e) {
        qWarning() << "*** An exception occured ***";
        qWarning() << e.message();
    }
    catch (const std::exception &e) {
        qWarning() << "*** An (std)exception occured ***";
        qWarning() << e.what();
    }
    QCoreApplication::quit();


}

void ConsoleShell::runYear(int year)
{
    printf("\r%s: simulating year %d %s          ", QDateTime::currentDateTime().toString("hh:mm:ss").toLocal8Bit().data(), year-1, GlobalSettings::instance()->controller()->timeString().toLocal8Bit().data());
}

static QMutex qdebug_mutex;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
 {
    Q_UNUSED(context);
    QMutexLocker m(&qdebug_mutex);

    switch (type) {
    case QtDebugMsg:
        *ConsoleShell::logStream() << msg << endl;
        if (ConsoleShell::flush())
            ConsoleShell::logStream()->flush();
        break;
    case QtWarningMsg:
        *ConsoleShell::logStream() << msg << endl;
        if (ConsoleShell::flush())
            ConsoleShell::logStream()->flush();
        printf("%s: %s\n", QDateTime::currentDateTime().toString("hh:mm:ss").toLocal8Bit().data(), msg.toLocal8Bit().data());
        break;
// available from qt5.5
    case QtInfoMsg:
        *ConsoleShell::logStream() << msg << endl;
        if (ConsoleShell::flush())
            ConsoleShell::logStream()->flush();
        printf("%s: %s\n", QDateTime::currentDateTime().toString("hh:mm:ss").toLocal8Bit().data(), msg.toLocal8Bit().data());

        break;
    case QtCriticalMsg:
        *ConsoleShell::logStream() << msg << endl;
        if (ConsoleShell::flush())
            ConsoleShell::logStream()->flush();
        printf("Critical: %s\n", msg.toLocal8Bit().data());
        break;
    case QtFatalMsg:
        *ConsoleShell::logStream() << msg << endl;
        if (ConsoleShell::flush())
            ConsoleShell::logStream()->flush();
        printf("Fatal: %s\n", msg.toLocal8Bit().data());
    }
 }


bool ConsoleShell::setupLogging()
{
    if (mLogStream) {
        if (mLogStream->device())
            delete mLogStream->device();
        delete mLogStream;
        mLogStream = NULL;
    }

    QString fname = GlobalSettings::instance()->settings().value("system.logging.logFile", "logfile.txt");
    mFlushLog = GlobalSettings::instance()->settings().valueBool("system.logging.flush");
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    fname.replace("$date$", timestamp);
    fname = GlobalSettings::instance()->path(fname, "log");
    QFile *file = new QFile(fname);

    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "cannot open logfile" << fname;
        return false;
    } else {
        qDebug() << "Log output is redirected to logfile" << fname;
        mLogStream = new QTextStream(file);
        qInstallMessageHandler(myMessageOutput);
        return true;
    }



}

void ConsoleShell::runJavascript(const QString key)
{
    for (int i=0;i<mParams.count(); ++i) {
        QString line=mParams[i];
        QString pkey = line.left(line.indexOf('='));
        if (pkey == key) {
            QString command = line.mid(line.indexOf('=')+1);
            // execute the function
            qWarning() << "executing trigger" << key;
            qWarning() << GlobalSettings::instance()->executeJavascript(command);
        }
    }


}

