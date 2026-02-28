/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file External3dMarteloscopeRunner.cpp */

// Include 3D Forest.
#include <External3dMarteloscopeRunner.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QProcess>
#include <QTcpSocket>

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    #include <QWebEngineView>
#endif

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopeRunner"
#include <Log.hpp>

External3dMarteloscopeRunner::External3dMarteloscopeRunner(
    MainWindow *mainWindow)
    : QObject(mainWindow),
      mainWindow_(mainWindow)
{
}

External3dMarteloscopeRunner::~External3dMarteloscopeRunner()
{
    stop();
}

bool External3dMarteloscopeRunner::isPortFree(int port) const
{
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", port);
    bool connected = socket.waitForConnected(200);
    socket.abort();
    return !connected; // true if connection failed → port free
}

int External3dMarteloscopeRunner::findFreePort(int startPort) const
{
    int port = startPort;
    while (port < startPort + 100)
    {
        // search up to +100 ports
        if (isPortFree(port))
        {
            return port;
        }
        ++port;
    }

    return -1; // none free
}

void External3dMarteloscopeRunner::start(const QString &pythonHome,
                                         const QString &pythonPath,
                                         const QString &pythonExe,
                                         const QString &pyhonScript,
                                         const QString &projectPath,
                                         int startPort)
{
    if (process_)
    {
        qDebug() << "Restarting Streamlit...";
        stop();
    }

    // Try to get a free port
    int port = findFreePort(startPort);
    if (port == -1)
    {
        qWarning() << "No free ports found!";
        return;
    }
    port_ = port;

    // If port was in use, we assume an existing server is running → just
    // connect
    if (!isPortFree(startPort) && port == startPort)
    {
        qDebug() << "Streamlit already running on port" << startPort
                 << ", reusing it.";
    }
    else
    {
        qDebug() << "run:";
        qDebug() << "  pythonHome" << pythonHome;
        qDebug() << "  pythonPath" << pythonPath;
        qDebug() << "  pythonExe" << pythonExe;
        qDebug() << "  pyhonScript" << pyhonScript;
        qDebug() << "  projectPath" << projectPath;

        // Start Streamlit
        process_ = new QProcess(this);

        if (pythonExe != "python")
        {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert("PYTHONHOME", pythonHome);
            env.insert("PYTHONPATH", pythonPath);
            env.insert("PYTHONNOUSERSITE", "1");
            process_->setProcessEnvironment(env);
        }

        QStringList args;
        args << "-m" << "streamlit" << "run" << pyhonScript << "--server.port"
             << QString::number(port_) << "--server.headless" << "true"
             << "--" << projectPath;

        process_->start(pythonExe, args);

        if (!process_->waitForStarted(3000))
        {
            qWarning() << "Failed to start Streamlit!";
            process_->deleteLater();
            process_.clear();
            return;
        }
        qDebug() << "Streamlit started on port" << port_;

        QProcess *p = process_; // snapshot pointer for the lambda

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
        connect(p,
                &QProcess::finished,
                this,
                [this, p]()
                {
                    qDebug() << "Streamlit process finished.";
                    if (view_)
                    {
                        view_->close();
                        view_.clear();
                    }

                    // process_->deleteLater();
                    if (process_ == p)
                    {
                        process_.clear();
                    }
                });
#else
        connect(p,
                &QProcess::finished,
                this,
                [this, p]()
                {
                    qDebug() << "Streamlit process finished.";
                    // process_->deleteLater();
                    if (process_ == p)
                    {
                        process_.clear();
                    }
                });
#endif
    }

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    qDebug() << "Open QWebEngineView";

    // Show the web view
    view_ = new QWebEngineView();
    view_->setAttribute(Qt::WA_DeleteOnClose);
    view_->setUrl(QUrl(QString("http://localhost:%1").arg(port_)));
    view_->resize(800, 600);
    view_->show();

    connect(view_, &QObject::destroyed, this, [this]() { view_.clear(); });
#endif
}

void External3dMarteloscopeRunner::stop()
{
    if (stopping_)
    {
        return;
    }

    stopping_ = true;

    if (process_)
    {
        qDebug() << "Stopping Streamlit...";
        process_->disconnect(this);

        if (process_->state() != QProcess::NotRunning)
        {
            process_->terminate();
            if (!process_->waitForFinished(2000))
            {
                process_->kill();
                process_->waitForFinished(1000);
            }
        }
        process_->deleteLater();
        process_.clear();
    }

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    if (view_)
    {
        view_->close();
        view_.clear();
    }
#endif

    port_ = -1;
    stopping_ = false;
}

bool External3dMarteloscopeRunner::isRunning() const
{
    return process_ && process_->state() != QProcess::NotRunning;
}
