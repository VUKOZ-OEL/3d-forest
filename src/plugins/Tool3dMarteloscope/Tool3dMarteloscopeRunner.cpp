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

/** @file Tool3dMarteloscopeRunner.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <File.hpp>
#include <Tool3dMarteloscopeRunner.hpp>

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    #include <QWebEngineView>
#endif

// Include std.
#include <chrono>
#include <mutex>
#include <thread>

#if defined(_WIN32)
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

// Include local.
#define LOG_MODULE_NAME "Tool3dMarteloscopeRunner"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

namespace
{
std::mutex webSessionPortMutex;
}

Tool3dMarteloscopeRunner::Tool3dMarteloscopeRunner(Application *app) : app_(app)
{
}

Tool3dMarteloscopeRunner::~Tool3dMarteloscopeRunner()
{
    stop();
}

bool Tool3dMarteloscopeRunner::isPortFree(int port) const
{
    if (port < 1 || port > 65535)
        return false;

#if defined(_WIN32)
    static std::once_flag winsockInitialized;
    static int winsockResult = WSASYSNOTREADY;
    std::call_once(winsockInitialized,
                   []
                   {
                       WSADATA data{};
                       winsockResult = WSAStartup(MAKEWORD(2, 2), &data);
                   });
    if (winsockResult != 0)
        return false;
    SOCKET socketHandle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketHandle == INVALID_SOCKET)
        return false;
#else
    int socketHandle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketHandle < 0)
        return false;
#endif

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<uint16_t>(port));
    const int result = ::connect(socketHandle,
                                 reinterpret_cast<sockaddr *>(&address),
                                 sizeof(address));
#if defined(_WIN32)
    closesocket(socketHandle);
#else
    ::close(socketHandle);
#endif
    return result != 0;
    // return !connected; // true if connection failed → port free
}

int Tool3dMarteloscopeRunner::findFreePort(int startPort) const
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

void Tool3dMarteloscopeRunner::start(const std::string &projectPath,
                                     int startPort)
{
    std::string binPath = File::currentPath();

    std::string pythonHome;
    std::string pythonPath;
    std::string pythonExe = File::join(binPath, "python/python.exe");
    std::string pyhonScript =
        File::join(binPath,
                   "plugins/3DForestTool3dMarteloscopePlugin/python/app.py");

    if (!File::exists(pythonExe))
    {
        // Use system Python if bundled version is not found.
        // This is intended for use in a development environment.
        pythonExe = "python";
    }
    else
    {
        // Use bundled Python
        pythonHome = File::join(binPath, "python");
        pythonPath = File::join(pythonHome, "Lib");
    }

    start(binPath,
          pythonHome,
          pythonPath,
          pythonExe,
          pyhonScript,
          projectPath,
          startPort);
}

void Tool3dMarteloscopeRunner::start(const std::string &binPath,
                                     const std::string &pythonHome,
                                     const std::string &pythonPath,
                                     const std::string &pythonExe,
                                     const std::string &pyhonScript,
                                     const std::string &projectPath,
                                     int startPort)
{
    const std::string baseUrlPath =
        app_ ? app_->localWebAppBasePath() : std::string();
    std::unique_lock<std::mutex> portLock(webSessionPortMutex, std::defer_lock);
    if (!baseUrlPath.empty())
    {
        // Keep simultaneous web sessions from selecting the same unused port.
        portLock.lock();
    }

    if (process_)
    {
        if (process_->isRunning())
        {
            LOG_DEBUG(<< "Restarting Streamlit...");
        }
        stop();
    }

    // Try to get a free port
    int port = findFreePort(startPort);
    if (port == -1)
    {
        LOG_WARNING("No free ports found!");
        return;
    }
    port_ = port;

    LOG_DEBUG(<< "run:");
    LOG_DEBUG(<< "  binPath " << binPath);
    LOG_DEBUG(<< "  pythonHome " << pythonHome);
    LOG_DEBUG(<< "  pythonPath " << pythonPath);
    LOG_DEBUG(<< "  pythonExe " << pythonExe);
    LOG_DEBUG(<< "  pyhonScript " << pyhonScript);
    LOG_DEBUG(<< "  projectPath " << projectPath);

    // Start Streamlit
    process_ = std::make_unique<Process>();

    process_->setFinishedCallback(
        [](int exitCode) {
            LOG_DEBUG("Streamlit process finished with exit code " << exitCode);
        });

    ProcessEnvironment env = ProcessEnvironment::systemEnvironment();
    if (pythonExe != "python")
    {
        env.insert("PYTHONHOME", pythonHome);
        env.insert("PYTHONPATH", pythonPath);
        env.insert("PYTHONNOUSERSITE", "1");
    }

    std::vector<std::string> args = {"-m",
                                     "streamlit",
                                     "run",
                                     pyhonScript,
                                     "--server.address",
                                     "127.0.0.1",
                                     "--server.port",
                                     std::to_string(port_),
                                     "--server.headless",
                                     "true",
                                     "--",
                                     projectPath,
                                     binPath};
    if (!baseUrlPath.empty())
    {
        args.insert(args.begin() + 4, {"--server.baseUrlPath", baseUrlPath});
    }

    process_->start(pythonExe, args, env);

    if (!process_->waitForStarted(3000))
    {
        LOG_WARNING(<< "Failed to start Streamlit using '" << pythonExe
                    << "' with script '" << pyhonScript
                    << "': " << process_->startError());
        process_.reset();
        return;
    }

#if !defined(TOOL_3D_MARTELOSCOPE_RUNNER_TEST)
    if (!baseUrlPath.empty())
    {
        bool serverReady = false;
        for (int attempt = 0; attempt < 150; ++attempt)
        {
            if (!process_->isRunning())
                break;
            if (!isPortFree(port_))
            {
                serverReady = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (!serverReady)
        {
            LOG_WARNING(<< "Streamlit did not listen on loopback port " << port_
                        << " in time");
            process_->kill();
            (void)process_->waitForFinished(1000);
            process_.reset();
            port_ = -1;
            return;
        }
    }
#endif
    LOG_DEBUG("Streamlit started on port " << port_);

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    LOG_DEBUG("Open QWebEngineView");

    // Show the web view
    view_ = new QWebEngineView();
    view_->setAttribute(Ui::WA_DeleteOnClose);
    view_->setUrl(QUrl(std::string("http://localhost:%1").arg(port_)));
    view_->resize(1024, 768);
    view_->show();
#else
    #if !defined(TOOL_3D_MARTELOSCOPE_RUNNER_TEST)
    if (!app_ || !app_->openLocalWebApp(port_))
    {
        LOG_WARNING("Could not expose the Marteloscope app in this UI");
    }
    #endif
#endif
}

void Tool3dMarteloscopeRunner::stop()
{
    if (stopping_)
    {
        return;
    }

    stopping_ = true;

    if (app_)
    {
        app_->closeLocalWebApp();
    }

    if (process_)
    {
        LOG_DEBUG("Stopping Streamlit...");
        process_->setFinishedCallback(Process::FinishedCallback());
        if (process_->isRunning())
        {
            process_->terminate();
            if (!process_->waitForFinished(2000))
            {
                process_->kill();
                (void)process_->waitForFinished(1000);
            }
        }
        process_.reset();
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

bool Tool3dMarteloscopeRunner::isRunning() const
{
    return process_ && process_->isRunning();
}
