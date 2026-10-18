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

/** @file ProcessEnvironment.hpp */

#ifndef PROCESS_ENVIRONMENT_HPP
#define PROCESS_ENVIRONMENT_HPP

// Include std.
#include <map>
#include <string>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Environment variables for a process. */
class EXPORT_CORE ProcessEnvironment
{
public:
    ProcessEnvironment() = default;

    static ProcessEnvironment systemEnvironment();

    void insert(const std::string &name, const std::string &value);
    bool contains(const std::string &name) const;
    std::string value(const std::string &name,
                      const std::string &defaultValue = std::string()) const;
    const std::map<std::string, std::string> &values() const { return values_; }

private:
    std::map<std::string, std::string> values_;
};

#include <WarningsEnable.hpp>

#endif /* PROCESS_ENVIRONMENT_HPP */
