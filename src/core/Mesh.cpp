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
#include <fstream>

// Include 3D Forest.
#include <Mesh.hpp>
#include <Vector3.hpp>

// Include local.
#define LOG_MODULE_NAME "Mesh"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::clear()
{
    mode = Mesh::Mode::MODE_UNKNOWN;

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
    // color.resize(position.size());

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

#if 0
        color[i * 9 + 0] = vnormal[0] * 0.5F + 0.5F;
        color[i * 9 + 1] = vnormal[1] * 0.5F + 0.5F;
        color[i * 9 + 2] = vnormal[2] * 0.5F + 0.5F;
        color[i * 9 + 3] = vnormal[0] * 0.5F + 0.5F;
        color[i * 9 + 4] = vnormal[1] * 0.5F + 0.5F;
        color[i * 9 + 5] = vnormal[2] * 0.5F + 0.5F;
        color[i * 9 + 6] = vnormal[0] * 0.5F + 0.5F;
        color[i * 9 + 7] = vnormal[1] * 0.5F + 0.5F;
        color[i * 9 + 8] = vnormal[2] * 0.5F + 0.5F;
#endif
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

void Mesh::exportPLY(const std::string &path, double scale) const
{
    LOG_DEBUG(<< "Export path <" << path << "> position size <"
              << position.size() << ">.");

    if (position.size() < 3)
    {
        return;
    }

    float s = static_cast<float>(scale);
    size_t nVertices = position.size() / 3;
    unsigned int nElements;
    char text[512];

    File f;
    f.open(path, "w+t");

    f.write("ply\n");
    f.write("format ascii 1.0\n");
    f.write("element vertex " + toString(nVertices) + "\n");
    f.write("property float x\n");
    f.write("property float y\n");
    f.write("property float z\n");

    if (mode == Mesh::Mode::MODE_TRIANGLES)
    {
        nElements = static_cast<unsigned int>(nVertices / 3);
        f.write("element face " + toString(nElements) + "\n");
        f.write("property list uchar uint vertex_indices\n");
    }

    f.write("end_header\n");

    for (size_t i = 0; i < nVertices; i++)
    {
        (void)snprintf(text,
                       sizeof(text),
                       "%s %s %s\n",
                       toString(position[i * 3 + 0] * s).c_str(),
                       toString(position[i * 3 + 1] * s).c_str(),
                       toString(position[i * 3 + 2] * s).c_str());
        f.write(text);
    }

    if (mode == Mesh::Mode::MODE_TRIANGLES)
    {
        for (unsigned int i = 0; i < nElements; i++)
        {
            (void)snprintf(text,
                           sizeof(text),
                           "3 %u %u %u\n",
                           i * 3,
                           i * 3 + 1,
                           i * 3 + 2);
            f.write(text);
        }
    }

    f.close();
}
void Mesh::importPLY(const std::string &path, double scale)
{
    LOG_DEBUG(<< "Import path <" << path << ">.");

    std::ifstream file(path);

    if (!file)
    {
        THROW("Could not open file <" + path + ">.");
    }

    int state = 0;
    float s = static_cast<float>(scale);
    size_t nVertices = 0;
    size_t nVerticesLines = 0;
    std::vector<float> positionRead;

    std::string line;
    while (std::getline(file, line))
    {
        LOG_DEBUG(<< "Line <" << line << ">.");
        if (state == 0)
        {
            if (line == "end_header")
            {
                state = 1;
            }
            else if (line.rfind("element vertex", 0) == 0)
            {
                std::string numberStr = line.substr(15);
                nVertices = toSize(numberStr);
                LOG_DEBUG(<< "Vertex count <" << nVertices << ">.");
                positionRead.resize(nVertices * 3);
                set(positionRead, 0.0F);
            }
        }
        else if (state == 1)
        {
            LOG_DEBUG(<< "Vertex number <" << nVerticesLines << ">.");

            std::vector<std::string> tokens = split(line, ' ');
            if (tokens.size() > 2)
            {
                LOG_DEBUG(<< "Vertex coordinates x <" << tokens[0] << "> y <"
                          << tokens[1] << "> z <" << tokens[2] << ">.");
                positionRead[nVerticesLines * 3 + 0] = toFloat(tokens[0]) * s;
                positionRead[nVerticesLines * 3 + 1] = toFloat(tokens[1]) * s;
                positionRead[nVerticesLines * 3 + 2] = toFloat(tokens[2]) * s;
            }

            nVerticesLines++;
            if (nVerticesLines >= nVertices)
            {
                break;
            }
        }
    }

    position = positionRead;
    mode = Mesh::Mode::MODE_TRIANGLES;
}

std::string toString(const Mesh::Mode &in)
{
    switch (in)
    {
        case Mesh::Mode::MODE_POINTS:
            return "Points";
        case Mesh::Mode::MODE_LINES:
            return "Lines";
        case Mesh::Mode::MODE_TRIANGLES:
            return "Triangles";
        case Mesh::Mode::MODE_UNKNOWN:
        default:
            return "Unknown";
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
    else if (in == "Points")
    {
        out = Mesh::Mode::MODE_POINTS;
    }
    else
    {
        out = Mesh::Mode::MODE_UNKNOWN;
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

void fromJson(Mesh::Mode &out,
              const Json &in,
              const std::string &key,
              Mesh::Mode defaultValue = Mesh::Mode::MODE_UNKNOWN,
              bool optional = true)
{
    if (in.contains(key) && in[key].typeString())
    {
        fromString(out, in[key].string());
    }
    else if (!optional)
    {
        THROW("JSON required key " + key + " was not found");
    }
    else
    {
        out = defaultValue;
    }
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
    out.clear();

    fromJson(out.name, in, "name", "", false);
    fromJson(out.mode, in, "mode");

    fromJson(out.position, in, "position");
    fromJson(out.color, in, "color");
    fromJson(out.normal, in, "normal");
    fromJson(out.indices, in, "indices");
}
