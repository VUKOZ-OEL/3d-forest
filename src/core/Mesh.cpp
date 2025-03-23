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

/** @file Mesh.cpp */

// Include std.
#include <cmath>

// Include 3D Forest.
#include <Mesh.hpp>
#include <Vector3.hpp>

// Include local.
#define LOG_MODULE_NAME "Mesh"
#include <Log.hpp>

Mesh::Mesh() : mode(Mesh::Mode::MODE_POINTS)
{
}

Mesh::~Mesh()
{
}

void Mesh::clear()
{
    mode = Mesh::Mode::MODE_POINTS;

    position.clear();
    color.clear();
    normal.clear();

    indices.clear();
}

void Mesh::calculateNormals()
{
    if (indices.empty())
    {
        if (mode == Mesh::Mode::MODE_TRIANGLES)
        {
            calculateNormalsTriangles();
        }
    }
}

void Mesh::calculateNormalsTriangles()
{
    Vector3<float> v1;
    Vector3<float> v2;
    Vector3<float> vnormal;

    normal.resize(position.size());

    size_t nTriangles = position.size() / 9;

    for (size_t i = 0; i < nTriangles; i++)
    {
        v1[0] = position[i * 9 + 3] - position[i * 9 + 0];
        v1[1] = position[i * 9 + 4] - position[i * 9 + 1];
        v1[2] = position[i * 9 + 5] - position[i * 9 + 2];

        v2[0] = position[i * 9 + 6] - position[i * 9 + 0];
        v2[1] = position[i * 9 + 7] - position[i * 9 + 1];
        v2[2] = position[i * 9 + 8] - position[i * 9 + 2];

        vnormal = Vector3<float>::crossProduct(v1, v2);
        vnormal.normalize();

        normal[i * 9 + 0] = vnormal[0];
        normal[i * 9 + 1] = vnormal[1];
        normal[i * 9 + 2] = vnormal[2];

        normal[i * 9 + 3] = vnormal[0];
        normal[i * 9 + 4] = vnormal[1];
        normal[i * 9 + 5] = vnormal[2];

        normal[i * 9 + 6] = vnormal[0];
        normal[i * 9 + 7] = vnormal[1];
        normal[i * 9 + 8] = vnormal[2];
    }
}

double Mesh::calculateSurfaceArea2d()
{
    if (indices.empty())
    {
        if (mode == Mesh::Mode::MODE_TRIANGLES)
        {
            return calculateSurfaceArea2dTriangles();
        }
    }

    return 0;
}

double Mesh::calculateSurfaceArea2dTriangles()
{
    double a = 0;
    double b = 0;
    double c = 0;

    size_t nTriangles = position.size() / 9;

    for (size_t i = 0; i < nTriangles; i++)
    {
        a += position[i * 9 + 0] * (position[i * 9 + 4] - position[i * 9 + 7]);
        b += position[i * 9 + 3] * (position[i * 9 + 7] - position[i * 9 + 1]);
        c += position[i * 9 + 6] * (position[i * 9 + 1] - position[i * 9 + 4]);
    }

    return fabs(a + b + c) * 0.5;
}

std::string toString(const Mesh::Mode &in)
{
    switch (in)
    {
        case Mesh::Mode::MODE_LINES:
            return "Lines";
        case Mesh::Mode::MODE_TRIANGLES:
            return "Triangles";
        case Mesh::Mode::MODE_POINTS:
        default:
            return "Points";
    }
}

void fromString(Mesh::Mode &out, const std::string &in)
{
    if (in == "Lines")
    {
        out = Mesh::Mode::MODE_LINES;
    }
    else if (in == "Triangles")
    {
        out = Mesh::Mode::MODE_TRIANGLES;
    }
    else
    {
        out = Mesh::Mode::MODE_POINTS;
    }
}

void toJson(Json &out, const Mesh::Mode &in)
{
    out = toString(in);
}

void fromJson(Mesh::Mode &out, const Json &in)
{
    fromString(out, in.string());
}

void toJson(Json &out, const Mesh &in)
{
    toJson(out["name"], in.name);
    toJson(out["mode"], in.mode);
    toJson(out["position"], in.position);
    toJson(out["color"], in.color);
    toJson(out["normal"], in.normal);
    toJson(out["indices"], in.indices);
}

void fromJson(Mesh &out, const Json &in)
{
    fromJson(out.name, in["name"]);
    fromJson(out.mode, in["mode"]);
    fromJson(out.position, in["position"]);
    fromJson(out.color, in["color"]);
    fromJson(out.normal, in["normal"]);
    fromJson(out.indices, in["indices"]);
}
