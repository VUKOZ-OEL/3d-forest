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

/** @file ProcessEnvironment.cpp */

// Include 3D Forest.
#include <ProcessEnvironment.hpp>

// Include std.
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cwchar>

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#else
extern char **environ;
#endif

namespace
{
bool equalsName(const std::string &a, const std::string &b)
{
#if defined(_WIN32)
    if (a.size() != b.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i])))
        {
            return false;
        }
    }
    return true;
#else
    return a == b;
#endif
}

#if defined(_WIN32)
std::string toUtf8(const wchar_t *value, int length)
{
    if (length <= 0)
    {
        return std::string();
    }
    int size = WideCharToMultiByte(CP_UTF8,
                                   WC_ERR_INVALID_CHARS,
                                   value,
                                   length,
                                   nullptr,
                                   0,
                                   nullptr,
                                   nullptr);
    if (size <= 0)
    {
        return std::string();
    }
    std::string result(static_cast<size_t>(size), '\0');
    if (WideCharToMultiByte(CP_UTF8,
                            WC_ERR_INVALID_CHARS,
                            value,
                            length,
                            &result[0],
                            size,
                            nullptr,
                            nullptr) <= 0)
    {
        return std::string();
    }
    return result;
}
#endif
} // namespace

ProcessEnvironment ProcessEnvironment::systemEnvironment()
{
    ProcessEnvironment result;
#if defined(_WIN32)
    LPWCH block = GetEnvironmentStringsW();
    if (!block)
    {
        return result;
    }
    for (const wchar_t *entry = block; *entry; entry += wcslen(entry) + 1)
    {
        const wchar_t *equals = wcschr(entry, L'=');
        // Windows has pseudo-variables such as =C:=..., which have a second
        // '=' delimiter after their leading '='.
        if (!equals || equals == entry || equals[1] == L'\0')
        {
            continue;
        }
        std::string name = toUtf8(entry, static_cast<int>(equals - entry));
        std::string value =
            toUtf8(equals + 1, static_cast<int>(wcslen(equals + 1)));
        if (!name.empty())
        {
            result.insert(name, value);
        }
    }
    FreeEnvironmentStringsW(block);
#else
    for (char **entry = environ; entry && *entry; ++entry)
    {
        const char *equals = strchr(*entry, '=');
        if (!equals || equals == *entry)
        {
            continue;
        }
        result.insert(std::string(*entry, static_cast<size_t>(equals - *entry)),
                      equals + 1);
    }
#endif
    return result;
}

void ProcessEnvironment::insert(const std::string &name,
                                const std::string &value)
{
    if (name.empty() || name.find('=') != std::string::npos)
    {
        return;
    }
    for (auto i = values_.begin(); i != values_.end(); ++i)
    {
        if (equalsName(i->first, name))
        {
            values_.erase(i);
            break;
        }
    }
    values_[name] = value;
}

bool ProcessEnvironment::contains(const std::string &name) const
{
    for (const auto &entry : values_)
    {
        if (equalsName(entry.first, name))
        {
            return true;
        }
    }
    return false;
}

std::string ProcessEnvironment::value(const std::string &name,
                                      const std::string &defaultValue) const
{
    for (const auto &entry : values_)
    {
        if (equalsName(entry.first, name))
        {
            return entry.second;
        }
    }
    return defaultValue;
}
