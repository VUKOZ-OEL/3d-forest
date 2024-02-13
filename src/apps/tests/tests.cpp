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

/** @file tests.cpp */

// Include 3D Forest.
#include <Test.hpp>

// Include local.
#define LOG_MODULE_NAME "tests"
#include <Log.hpp>

int main()
{
    int rc = 1;

    LOGGER_START_FILE("log_tests.txt");

    try
    {
        Test::instance()->run();

        if (Test::instance()->failed() == 0)
        {
            rc = 0;
        }
        else
        {
            std::cout << "Tests Failed: " << Test::instance()->failed()
                      << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    LOGGER_STOP_FILE;

    return rc;
}
