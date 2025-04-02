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
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ArgumentParser"
#include <Log.hpp>

ArgumentParser::ArgumentParser(const std::string &description)
    : description_(description)
{
    add("-h", "--help", "", "Show this help message and exit.");
}

void ArgumentParser::add(const std::string &shortOption,
                         const std::string &longOption,
                         const std::string &defaultValue,
                         const std::string &help,
                         bool required)
{
    options_[longOption].shortOption = shortOption;
    options_[longOption].longOption = longOption;
    options_[longOption].text = defaultValue;
    options_[longOption].help = help;
    options_[longOption].required = required;
    options_[longOption].count = 0;

    insertOrder.push_back(longOption);
}

bool ArgumentParser::parse(int argc, char *argv[])
{
    int i = 1;

    if (argc > 0)
    {
        programName_ = argv[0];
    }

    // Find matching options.
    while (i < argc)
    {
        std::string option(argv[i]);

        // Try to find option by long option name.
        auto search = options_.find(option);

        // Try to find option by short option name.
        if (search == options_.end())
        {
            search = options_.begin();
            while (search != options_.end())
            {
                if (option == search->second.shortOption)
                {
                    break;
                }

                ++search;
            }
        }

        // Option found.
        if (search != options_.end())
        {
            if (i + 1 < argc && options_.count(argv[i + 1]) == 0)
            {
                search->second.text = argv[i + 1];
                i++;
            }

            search->second.count++;
        }
        else
        {
            invalidOption(option);
            return false;
        }

        i++;
    }

    // Show help.
    if (contains("--help"))
    {
        help();
        return false;
    }

    // Check required options.
    for (auto const &arg : options_)
    {
        if (arg.second.required && arg.second.count < 1)
        {
            missingOption(arg.second);
            return false;
        }
    }

    return true;
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
    return ::toBool(toString(longOption));
}

float ArgumentParser::toFloat(const std::string &longOption) const
{
    return ::toFloat(toString(longOption));
}

double ArgumentParser::toDouble(const std::string &longOption) const
{
    return ::toDouble(toString(longOption));
}

int ArgumentParser::toInt(const std::string &longOption) const
{
    return ::toInt(toString(longOption));
}

size_t ArgumentParser::toSize(const std::string &longOption) const
{
    return ::toSize(toString(longOption));
}

uint32_t ArgumentParser::toUint32(const std::string &longOption) const
{
    return ::toUint32(toString(longOption));
}

uint64_t ArgumentParser::toUint64(const std::string &longOption) const
{
    return ::toUint64(toString(longOption));
}

void ArgumentParser::help() const
{
    std::string indent{"    "};

    std::cout << "name: " << std::endl;
    std::cout << indent << programName_ << " - " << description_ << std::endl;

    std::cout << std::endl;

    std::cout << "options:" << std::endl;
    for (auto const &it : insertOrder)
    {
        auto const &option = options_.at(it);

        std::cout << indent;

        if (option.shortOption.empty())
        {
            std::cout << "     ";
        }
        else
        {
            std::cout << option.shortOption << ", ";
            if (option.shortOption.size() < 3)
            {
                std::cout << " ";
            }
        }

        std::cout << option.longOption;

        std::cout << " ... " << option.help;

        if (!option.text.empty())
        {
            std::cout << ", default " << option.text;
        }

        if (option.required)
        {
            std::cout << ", required " << option.text;
        }

        std::cout << std::endl;
    }
}

void ArgumentParser::error(const std::string &message) const
{
    std::cout << programName_ << ": " << message << std::endl;
    std::cout << "Try '" << programName_ << " --help' for more information."
              << std::endl;
}

void ArgumentParser::invalidOption(const std::string &option) const
{
    error("invalid option '" + option + "'");
}

void ArgumentParser::missingOption(const Option &option) const
{
    error("missing required option '" + option.longOption + "'");
}
