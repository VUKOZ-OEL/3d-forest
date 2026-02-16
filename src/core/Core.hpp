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

/** @file Core.hpp */

#ifndef CORE_HPP
#define CORE_HPP

// Include std.
#include <map>
#include <string>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Core. */
class EXPORT_CORE Core
{
public:
    Core();
    ~Core() = default;

    void clear();
    void setLanguageByCode(const std::string &code);
    void insertTranslation(const std::string &source,
                           const std::string &translation,
                           const std::string &languageCode);
    const std::string &translate(const std::string &text) const;
    const std::string &translate(const std::string &text,
                                 const std::string &languageCode) const;

private:
    /** Translation. */
    class Translation
    {
    public:
        std::map<std::string,std::string> language;
    };

    std::string languageCode_;
    std::map<std::string,Translation> translations_; // source:translation
};

EXPORT_CORE Core &core();

#include <WarningsEnable.hpp>

#endif /* CORE_HPP */
