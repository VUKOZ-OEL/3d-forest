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

/** @file sandbox.cpp */

// Include std.
#include <cstdio>

// Include 3rd party.
#include <drogon/HttpAppFramework.h>
#include <jsoncpp/include/json/reader.h>
#include <zlib/zlib.h>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "sandbox"
#include <Log.hpp>

static void sandbox()
{
    std::printf("Zlib version '%s'\n", zlibVersion());

    JsonCpp::Reader jsonReader;

    std::printf("Drogon version '%s'\n", drogon::getVersion().c_str());
}

int main()
{
    try
    {
        sandbox();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "error: unknown" << std::endl;
        return 1;
    }

    return 0;
}
