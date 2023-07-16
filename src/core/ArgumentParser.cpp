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

/** @file ArgumentParser.cpp */

#include <cmath>

#include <ArgumentParser.hpp>

#define LOG_MODULE_NAME "ArgumentParser"
#include <Log.hpp>

ArgumentParser::ArgumentParser()
{
}

void ArgumentParser::add(const std::string &name,
                         const std::string &defaultValue)
{
    args_[name].text = defaultValue;
    args_[name].count = 0;
}

void ArgumentParser::parse(int argc, char *argv[])
{
    int i = 1;

    while (i < argc)
    {
        std::string name(argv[i]);

        auto search = args_.find(name);
        if (search != args_.end())
        {
            if (i + 1 < argc && args_.count(argv[i + 1]) == 0)
            {
                search->second.text = argv[i + 1];
                i++;
            }

            search->second.count++;
        }

        i++;
    }
}

bool ArgumentParser::read(const std::string &name, int &value) const
{
    if (contains(name))
    {
        value = toInt(name);
        return true;
    }

    return false;
}

float ArgumentParser::toFloat(const std::string &name) const
{
    return std::stof(toString(name));
}

double ArgumentParser::toDouble(const std::string &name) const
{
    return std::stod(toString(name));
}

int ArgumentParser::toInt(const std::string &name) const
{
    return static_cast<int>(std::stoll(toString(name)));
}

size_t ArgumentParser::toSize(const std::string &name) const
{
    return static_cast<size_t>(std::stoull(toString(name)));
}

uint32_t ArgumentParser::toUint32(const std::string &name) const
{
    return static_cast<uint32_t>(std::stoull(toString(name)));
}

uint64_t ArgumentParser::toUint64(const std::string &name) const
{
    return static_cast<uint64_t>(std::stoull(toString(name)));
}
