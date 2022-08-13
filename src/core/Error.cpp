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

/** @file Error.cpp */

#include <cerrno>
#include <cstdio>

#if defined(_MSC_VER)
    #include <iomanip>
    #include <sstream>
    #include <windows.h>
#endif

#include <Error.hpp>

std::string getErrorString(int errnum)
{
    char buffer[512];

    if (errnum < sys_nerr)
    {
        (void)std::snprintf(buffer, sizeof(buffer), "%s", sys_errlist[errnum]);
    }
    else
    {
        (void)std::snprintf(buffer, sizeof(buffer), "Unknown error %d", errnum);
    }

    return std::string(buffer);
}

std::string getErrorString()
{
    return getErrorString(errno);
}

std::string getErrorString(const std::string &message)
{
    return message + ": " + getErrorString();
}

std::string getErrorStringWin(const std::string &message)
{
#if defined(_MSC_VER)
    LPTSTR errorText = NULL;
    DWORD error = GetLastError();

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&errorText,
                  0,
                  NULL);

    std::stringstream errorStream;
    errorStream << ": error code 0x" << std::hex << error << ": " << errorText;

    LocalFree(errorText);

    return message + errorStream.str();
#else
    return message;
#endif
}
