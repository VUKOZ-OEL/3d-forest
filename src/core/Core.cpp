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

/** @file Core.cpp */

// Include std.

// Include 3D Forest.
#include <Core.hpp>

// Include local.
#define LOG_MODULE_NAME "Core"
#include <Log.hpp>

Core &core()
{
    static Core instance;
    return instance;
}

Core::Core()
{
    languageCode_ = "en";
}

void Core::clear()
{
    translations_.clear();
}

void Core::setLanguageByCode(const std::string &code)
{
    languageCode_ = code;
}

void Core::insertTranslation(const std::string &source, const std::string &translation, const std::string &languageCode)
{
    translations_[source].language[languageCode] = translation;
}

const std::string &Core::translate(const std::string &text) const
{
    return translate(text, languageCode_);
}

const std::string &Core::translate(const std::string &text, const std::string &languageCode) const
{
    auto itSource = translations_.find(text);
    if (itSource != translations_.end())
    {
        const Translation &translation = itSource->second;
        auto itLanguage = translation.language.find(languageCode);
        if (itLanguage != translation.language.end())
        {
            return itLanguage->second;
        }
    }

    return text;
}
