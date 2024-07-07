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

// Include std.
#include <cmath>

// Include 3D Forest.
#include <ArgumentParser.hpp>

// Include local.
#define LOG_MODULE_NAME "ArgumentParser"
#include <Log.hpp>

ArgumentParser::ArgumentParser(const std::string &description)
    : description_(description)
{
}

void ArgumentParser::add(const std::string &shortOption,
                         const std::string &longOption,
                         const std::string &defaultValue,
                         const std::string &help,
                         bool required)
{
    args_[longOption].shortOption = shortOption;
    args_[longOption].longOption = longOption;
    args_[longOption].text = defaultValue;
    args_[longOption].help = help;
    args_[longOption].required = required;
    args_[longOption].count = 0;
}

void ArgumentParser::parse(int argc, char *argv[])
{
    int i = 1;

    if (argc > 0)
    {
        programName_ = argv[0];
    }

    while (i < argc)
    {
        std::string option(argv[i]);

        // Try to find option by long option name.
        auto search = args_.find(option);

        // Try to find option by short option name.
        if (search == args_.end())
        {
            search = args_.begin();
            while (search != args_.end())
            {
                if (option == search->second.shortOption)
                {
                    break;
                }

                ++search;
            }
        }

        // Option found.
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

bool ArgumentParser::read(const std::string &longOption, int &value) const
{
    if (contains(longOption))
    {
        value = toInt(longOption);
        return true;
    }

    return false;
}

bool ArgumentParser::toBool(const std::string &longOption) const
{
    return toString(longOption) == "true";
}

float ArgumentParser::toFloat(const std::string &longOption) const
{
    return std::stof(toString(longOption));
}

double ArgumentParser::toDouble(const std::string &longOption) const
{
    return std::stod(toString(longOption));
}

int ArgumentParser::toInt(const std::string &longOption) const
{
    return static_cast<int>(std::stoll(toString(longOption)));
}

size_t ArgumentParser::toSize(const std::string &longOption) const
{
    return static_cast<size_t>(std::stoull(toString(longOption)));
}

uint32_t ArgumentParser::toUint32(const std::string &longOption) const
{
    return static_cast<uint32_t>(std::stoull(toString(longOption)));
}

uint64_t ArgumentParser::toUint64(const std::string &longOption) const
{
    return static_cast<uint64_t>(std::stoull(toString(longOption)));
}

void ArgumentParser::help()
{
    std::string indent{"    "};

    std::cout << "name: " << std::endl;
    std::cout << indent << programName_ << " - " << description_ << std::endl;

    std::cout << std::endl;

    std::cout << "options:" << std::endl;
    for (auto const &arg : args_)
    {
        std::cout << indent;

        if (arg.second.shortOption.empty())
        {
            std::cout << "     ";
        }
        else
        {
            std::cout << arg.second.shortOption << ", ";
            if (arg.second.shortOption.size() < 3)
            {
                std::cout << " ";
            }
        }

        std::cout << arg.second.longOption;

        std::cout << " ... " << arg.second.help;

        std::cout << std::endl;
    }
}
