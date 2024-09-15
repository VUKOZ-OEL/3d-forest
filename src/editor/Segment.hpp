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

/** @file Segment.hpp */

#ifndef SEGMENT_HPP
#define SEGMENT_HPP

// Include 3D Forest.
#include <Box.hpp>
#include <Json.hpp>
#include <Mesh.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Segment. */
class EXPORT_EDITOR Segment
{
public:
    size_t id{0};
    std::string label;
    Vector3<double> color;
    bool selected{false};

    std::vector<Mesh> meshList;

    bool hasCalculatedAttributes{false};

    Vector3<double> position;
    Box<double> boundary;

    double height{0.0};

    Vector3<double> dbhPosition;
    double dbh{0.0};

    Segment();
};

void fromJson(Segment &out, const Json &in);
void toJson(Json &out, const Segment &in);

std::ostream &operator<<(std::ostream &out, const Segment &in);

#include <WarningsEnable.hpp>

#endif /* SEGMENT_HPP */
