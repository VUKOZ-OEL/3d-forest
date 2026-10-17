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

/** @file WebServer.hpp */

#pragma once

// Include 3D Forest.
#include <WebSession.hpp>

// Include std.
#include <cstdint>
#include <memory>
#include <string>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebServer. */
class EXPORT_UI_WEB WebServer
{
public:
    struct Options
    {
        std::string address{"127.0.0.1"};
        std::uint16_t port{8080};
        std::string documentRoot{"web"};
        // Must exactly match the browser's Origin. Local-development default.
        std::string allowedOrigin{"http://127.0.0.1:8080"};
        unsigned int ioThreads{2};
        std::size_t maxSessions{32};
        std::chrono::seconds reconnectTimeout{120};
    };

    WebServer();
    explicit WebServer(Options options);
    ~WebServer();
    WebServer(const WebServer &) = delete;
    WebServer &operator=(const WebServer &) = delete;

    void setApplicationFactory(WebSession::Factory factory); // Before exec().
    int exec();  // Blocking; one server/exec per process (Drogon singleton).
    void quit(); // Thread-safe once exec() has started.

private:
    struct State;
    class Controller;
    std::shared_ptr<State> state_;
};

#include <WarningsEnable.hpp>
