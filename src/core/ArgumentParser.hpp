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

/** @file ArgumentParser.hpp */

#ifndef ARGUMENT_PARSER_HPP
#define ARGUMENT_PARSER_HPP

// Include std.
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Argument Parser. */
class EXPORT_CORE ArgumentParser
{
public:
    ArgumentParser(const std::string &description = "");
    ~ArgumentParser() = default;

    void add(const std::string &shortOption,
             const std::string &longOption,
             const std::string &defaultValue = "",
             const std::string &help = "",
             bool required = false);
    void parse(int argc, char *argv[]);

    bool contains(const std::string &longOption) const;
    bool read(const std::string &longOption, int &value) const;

    const std::string &toString(const std::string &longOption) const;
    bool toBool(const std::string &longOption) const;
    float toFloat(const std::string &longOption) const;
    double toDouble(const std::string &longOption) const;
    int toInt(const std::string &longOption) const;
    size_t toSize(const std::string &longOption) const;
    uint32_t toUint32(const std::string &longOption) const;
    uint64_t toUint64(const std::string &longOption) const;

    void help();

private:
    std::string programName_;
    std::string description_;

    /** Argument Parser Value. */
    class Value
    {
    public:
        std::string shortOption;
        std::string longOption;
        std::string text;
        std::string help;
        bool required;
        int count{0};
        Value() : count(0) {}
    };

    std::map<std::string, Value> args_;
};

inline bool ArgumentParser::contains(const std::string &longOption) const
{
    return args_.at(longOption).count > 0;
}

inline const std::string &ArgumentParser::toString(
    const std::string &longOption) const
{
    return args_.at(longOption).text;
}

#include <WarningsEnable.hpp>

#endif /* ARGUMENT_PARSER_HPP */
