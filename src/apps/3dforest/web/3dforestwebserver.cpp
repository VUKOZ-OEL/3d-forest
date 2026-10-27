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
#include <WebAuthStore.hpp>
#include <WebServer.hpp>
#include <WebSession.hpp>

// Include std.
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

// Include local.
#define LOG_MODULE_NAME "3dforestwebserver"
#include <Log.hpp>
#include <WarningsDisable.hpp>

#ifndef GIT_COMMIT_HASH
    #define GIT_COMMIT_HASH "unknown"
#endif

namespace
{
std::string readPassword(const char *prompt)
{
    std::cout << prompt << std::flush;
#ifdef _WIN32
    std::wstring widePassword;
    for (;;)
    {
        const wint_t key = _getwch();
        if (key == '\r' || key == '\n')
            break;
        if (key == 3 || key == 0x1b)
            throw std::runtime_error("Password entry cancelled");
        if (key == 0 || key == 0xe0)
        {
            _getwch();
            continue;
        }
        if (key == '\b')
        {
            if (!widePassword.empty())
            {
                widePassword.pop_back();
                if (!widePassword.empty() && widePassword.back() >= 0xd800 &&
                    widePassword.back() <= 0xdbff)
                    widePassword.pop_back();
            }
        }
        else if (key >= 32 && key != 0x7f && widePassword.size() < 1024)
            widePassword.push_back(static_cast<wchar_t>(key));
    }
    const int bytes = WideCharToMultiByte(CP_UTF8,
                                          WC_ERR_INVALID_CHARS,
                                          widePassword.data(),
                                          static_cast<int>(widePassword.size()),
                                          nullptr,
                                          0,
                                          nullptr,
                                          nullptr);
    if (bytes <= 0)
        throw std::runtime_error("Password contains invalid characters");
    std::string password(static_cast<std::size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8,
                        WC_ERR_INVALID_CHARS,
                        widePassword.data(),
                        static_cast<int>(widePassword.size()),
                        password.data(),
                        bytes,
                        nullptr,
                        nullptr);
    std::fill(widePassword.begin(), widePassword.end(), L'\0');
#else
    termios original{};
    if (::tcgetattr(STDIN_FILENO, &original) != 0)
        throw std::runtime_error("Password entry requires a terminal");
    termios hidden = original;
    hidden.c_lflag &= static_cast<tcflag_t>(~ECHO);
    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &hidden) != 0)
        throw std::runtime_error("Could not disable terminal password echo");
    std::string password;
    try
    {
        std::getline(std::cin, password);
    }
    catch (...)
    {
        ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
        throw;
    }
    ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
#endif
    std::cout << '\n';
    return password;
}
} // namespace

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
        arg.add("-c",
                "--tls-certificate",
                "",
                "TLS certificate PEM file (required with --tls-private-key).");
        arg.add("-k",
                "--tls-private-key",
                "",
                "TLS private key PEM file (required with --tls-certificate).");
        arg.add("",
                "--account-directory",
                "accounts",
                "Directory for web accounts and the server pepper.");
        arg.add("-a",
                "--add-user",
                "",
                "Create an account (prompts for its password) and exit.");

        arg.addExample("3DForestWebServer.exe --web-root client "
                       "--listen-address 127.0.0.1 --port 8080 "
                       "--allowed-origin http://127.0.0.1:8080");
        arg.addExample("3DForestWebServer.exe -w client "
                       "-l 0.0.0.0 -p 8080 "
                       "-o https://forest.example.org");
        arg.addExample(
            "3DForestWebServer.exe --listen-address 0.0.0.0 "
            "--port 8443 --allowed-origin https://forest.example.org "
            "--tls-certificate fullchain.pem --tls-private-key key.pem");
        arg.addExample("3DForestWebServer.exe --add-user analyst "
                       "--account-directory accounts");

        if (arg.parse(argc, argv))
        {
            if (arg.contains("--add-user"))
            {
                auto password =
                    readPassword("Password (minimum 8 characters): ");
                auto confirmation = readPassword("Confirm password: ");
                if (password != confirmation)
                {
                    std::fill(password.begin(), password.end(), '\0');
                    std::fill(confirmation.begin(), confirmation.end(), '\0');
                    throw std::invalid_argument("Passwords do not match");
                }
                try
                {
                    WebAuthStore::addUser(arg.toString("--account-directory"),
                                          arg.toString("--add-user"),
                                          password);
                }
                catch (...)
                {
                    std::fill(password.begin(), password.end(), '\0');
                    std::fill(confirmation.begin(), confirmation.end(), '\0');
                    throw;
                }
                std::fill(password.begin(), password.end(), '\0');
                std::fill(confirmation.begin(), confirmation.end(), '\0');
                std::cout << "Web account created.\n";
                rc = 0;
            }
            else
            {
                WebServer::Options options;
                options.documentRoot =
                    std::filesystem::absolute(arg.toString("--web-root"))
                        .string();
                options.address = arg.toString("--listen-address");
                const uint32_t port = arg.toUint32("--port");
                if (port == 0 || port > 65535)
                {
                    throw std::invalid_argument("Invalid web server port");
                }
                options.port = static_cast<std::uint16_t>(port);
                options.allowedOrigin = arg.toString("--allowed-origin");
                options.tlsCertificate = arg.toString("--tls-certificate");
                options.tlsPrivateKey = arg.toString("--tls-private-key");
                options.accountDirectory = arg.toString("--account-directory");

                std::cout << "Listening on " << options.address << ':'
                          << options.port << "; allowed browser origin: "
                          << options.allowedOrigin << " (Ctrl+C to stop)\n";

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
