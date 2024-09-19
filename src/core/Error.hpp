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

/** @file Error.hpp */

#ifndef ERROR_HPP
#define ERROR_HPP

// Include std.
#include <stdexcept>
#include <string>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Throw std exception. */
#define THROW(msg) throw std::runtime_error(msg)

/** Throw std exception with the last error for POSIX functions. */
#define THROW_ERRNO(msg) throw std::runtime_error(errorString(msg))

/** Throw std exception with GetLastError for Windows functions. */
#define THROW_LAST_ERROR(msg) throw std::runtime_error(errorStringWin(msg))

/** Get error string for given error number. */
std::string EXPORT_CORE errorString(int errnum);

/** Get the last error string. */
std::string EXPORT_CORE errorString();

/** Get the last error string with extra description for POSIX functions. */
std::string EXPORT_CORE errorString(const std::string &message);

/** Get the last error string with extra description for Windows functions. */
std::string EXPORT_CORE errorStringWin(const std::string &message);

#include <WarningsEnable.hpp>

#endif /* ERROR_HPP */
