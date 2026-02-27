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

/** @file Species.hpp */

#ifndef SPECIES_HPP
#define SPECIES_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Species. */
class EXPORT_EDITOR Species
{
public:
    size_t id{0};
    std::string abbreviation;
    std::string genus;
    std::string species;
    std::string latin;
    std::string czech;
    Vector3<double> color;

    Species();
    Species(size_t id,
            const std::string &abbreviation,
            const std::string &genus,
            const std::string &species,
            const std::string &latin,
            const std::string &czech,
            const Vector3<double> &color);
};

void fromJson(Species &out, const Json &in);
void toJson(Json &out, const Species &in);

std::ostream &operator<<(std::ostream &out, const Species &in);

#include <WarningsEnable.hpp>

#endif /* SPECIES_HPP */
