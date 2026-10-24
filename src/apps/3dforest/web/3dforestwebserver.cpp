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

/** @file 3dforest.cpp */

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <Error.hpp>
#include <WebApplication.hpp>
#include <WebServer.hpp>
#include <WebSession.hpp>

// Include std.
#include <stdexcept>
#include <string>

// Include local.
#define LOG_MODULE_NAME "3dforestwebserver"
#include <Log.hpp>
#include <WarningsDisable.hpp>

#ifndef GIT_COMMIT_HASH
    #define GIT_COMMIT_HASH "unknown"
#endif

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log.txt");
    LOG_INFO(<< "3DForest started. Git Revision <" << GIT_COMMIT_HASH << ">.");

    try
    {
        ArgumentParser arg("starts the 3D Forest web server");
        arg.add("-w", "--web-root", "client", "Path to the web document root.");
        arg.add("-l",
                "--listen-address",
                "127.0.0.1",
                "Address the web server listens on.");
        arg.add("-p", "--port", "8080", "Web server port.");
        arg.add("-o",
                "--allowed-origin",
                "http://127.0.0.1:8080",
                "Allowed browser origin.");
        arg.addExample("3DForestWebServer.exe --web-root client "
                       "--listen-address 127.0.0.1 --port 8080 "
                       "--allowed-origin http://127.0.0.1:8080");
        arg.addExample("3DForestWebServer.exe -w client "
                       "-l 0.0.0.0 -p 8080 "
                       "-o https://forest.example.org");

        if (arg.parse(argc, argv))
        {
            WebServer::Options options;
            options.documentRoot =
                std::filesystem::absolute(arg.toString("--web-root")).string();
            options.address = arg.toString("--listen-address");
            const uint32_t port = arg.toUint32("--port");
            if (port == 0 || port > 65535)
            {
                throw std::invalid_argument("Invalid web server port");
            }
            options.port = static_cast<std::uint16_t>(port);
            options.allowedOrigin = arg.toString("--allowed-origin");

            std::cout << "Listening on " << options.address << ':'
                      << options.port
                      << "; allowed browser origin: " << options.allowedOrigin
                      << " (Ctrl+C to stop)\n";

            WebServer server(options);

            server.setApplicationFactory(
                [](WebSession &session)
                {
                    auto app = std::make_unique<WebApplication>(session);

                    app->setOrganizationName("VUKOZ v.v.i.");
                    app->setApplicationName("3D Forest");
                    app->setApplicationVersion("1.0");

                    // Create this session's plugins and common UI.
                    app->init();

                    return app;
                });

            rc = server.exec();
        }
        else
        {
            rc = 0;
        }
    }
    catch (std::exception &e)
    {
        LOG_ERROR("error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR("error: unknown");
    }

    LOG_INFO(<< "3DForest closed with exit code <" << rc << ">.");
    LOGGER_STOP_FILE;

    return rc;
}

#include <WarningsEnable.hpp>
