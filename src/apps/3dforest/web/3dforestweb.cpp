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
#include <Error.hpp>
#include <WebApplication.hpp>
#include <WebServer.hpp>
#include <WebSession.hpp>

// Include std.
#include <stdexcept>
#include <string>

// Include local.
#define LOG_MODULE_NAME "3dforestweb"
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
        if (argc > 5)
        {
            throw std::invalid_argument(
                "Usage: 3DForestWeb [web-root] [listen-address] [port] "
                "[allowed-origin]");
        }

        WebServer::Options options;
        options.documentRoot =
            std::filesystem::absolute(argc > 1 ? argv[1] : "web").string();
        if (argc > 2)
            options.address = argv[2];
        if (argc > 3)
        {
            const unsigned long port = std::stoul(argv[3]);
            if (port == 0 || port > 65535)
                throw std::invalid_argument("Invalid web server port");
            options.port = static_cast<std::uint16_t>(port);
        }
        if (argc > 4)
            options.allowedOrigin = argv[4];

        std::cout << "Listening on " << options.address << ':' << options.port
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
