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

/** @file FileFormatMeshPly.cpp */

// Include std.
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Include 3D Forest.
#include <File.hpp>
#include <FileFormatMeshPly.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "FileFormatMeshPly"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

static bool startsWith(const std::string &s, const std::string &p)
{
    return s.rfind(p, 0) == 0;
}

void FileFormatMeshPly::read(Mesh &mesh, const std::string &path, double scale)
{
    LOG_DEBUG(<< "Start reading PLY mesh from <" << path << ">.");

    mesh.clear();

    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        THROW("Could not open file.");
    }

    // parse header
    std::string line;
    std::getline(f, line);

    if (line != "ply")
    {
        THROW("PLY file header not found.");
    }

    bool binary = false;
    size_t vertexCount = 0;
    size_t faceCount = 0;
    size_t ni = 0;

    while (std::getline(f, line))
    {
        if (startsWith(line, "format"))
        {
            if (line.find("binary_little_endian") != std::string::npos)
            {
                binary = true;
            }
            else if (line.find("ascii") != std::string::npos)
            {
                binary = false;
            }
            else
            {
                THROW("Binary/ascii format definition not found.");
            }
        }
        else if (startsWith(line, "element vertex"))
        {
            std::istringstream iss(line);
            std::string a, b;
            iss >> a >> b >> vertexCount;
        }
        else if (startsWith(line, "element face"))
        {
            std::istringstream iss(line);
            std::string a, b;
            iss >> a >> b >> faceCount;
        }
        else if (line == "end_header")
        {
            break;
        }
    }

    LOG_DEBUG(<< "Header vertexCount <" << vertexCount << ">.");
    LOG_DEBUG(<< "Header faceCount <" << faceCount << ">.");

    mesh.position.resize(vertexCount * 3);
    mesh.indices.resize(faceCount * 3);
    mesh.mode = Mesh::Mode::MODE_TRIANGLES;

    // read data
    if (binary)
    {
        // vertices
        for (size_t i = 0; i < vertexCount; ++i)
        {
            f.read(reinterpret_cast<char *>(&mesh.position[i * 3]),
                   3 * sizeof(float));
        }

        // faces
        std::vector<int> idx;
        for (size_t i = 0; i < faceCount; ++i)
        {
            uint8_t c = 0;
            f.read(reinterpret_cast<char *>(&c), 1);
            if (c < 1)
            {
                continue;
            }

            size_t count = static_cast<size_t>(c);
            idx.resize(count);

            f.read(reinterpret_cast<char *>(idx.data()), count * sizeof(int));

            if (count > 2)
            {
                mesh.indices[ni++] = static_cast<unsigned int>(idx[0]);
                mesh.indices[ni++] = static_cast<unsigned int>(idx[1]);
                mesh.indices[ni++] = static_cast<unsigned int>(idx[2]);
            }
        }
    }
    else
    {
        // vertices
        for (size_t i = 0; i < vertexCount; ++i)
        {
            f >> mesh.position[i * 3 + 0] >> mesh.position[i * 3 + 1] >>
                mesh.position[i * 3 + 2];
        }

        // faces
        std::vector<int> idx(3);
        for (size_t i = 0; i < faceCount; ++i)
        {
            int c = 0;
            f >> c;
            if (c < 1)
            {
                continue;
            }

            size_t count = static_cast<size_t>(c);

            for (size_t j = 0; j < count; ++j)
            {
                unsigned int e;
                f >> e;
                if (count > 2 && j < 3)
                {
                    mesh.indices[ni++] = e;
                }
            }
        }
    }

    mesh.indices.resize(ni);

    for (size_t i = 0; i < mesh.position.size(); ++i)
    {
        mesh.position[i] *= static_cast<float>(scale);
    }

    LOG_DEBUG(<< "Finished reading PLY mesh.");
}

void FileFormatMeshPly::write(const Mesh &mesh,
                              const std::string &path,
                              double scale)
{
    LOG_DEBUG(<< "Export path <" << path << "> position size <"
              << mesh.position.size() << ">.");

    if (mesh.position.size() < 3)
    {
        return;
    }

    float s = static_cast<float>(scale);
    size_t nVertices = mesh.position.size() / 3;
    unsigned int nElements = 0;
    char text[512];

    File f;
    f.open(path, "w+t");

    f.write("ply\n");
    f.write("format ascii 1.0\n");
    f.write("element vertex " + toString(nVertices) + "\n");
    f.write("property float x\n");
    f.write("property float y\n");
    f.write("property float z\n");

    if (mesh.mode == Mesh::Mode::MODE_TRIANGLES)
    {
        if (mesh.indices.empty())
        {
            nElements = static_cast<unsigned int>(nVertices / 3);
        }
        else
        {
            nElements = static_cast<unsigned int>(mesh.indices.size() / 3);
        }

        f.write("element face " + toString(nElements) + "\n");
        f.write("property list uchar uint vertex_indices\n");
    }

    f.write("end_header\n");

    for (size_t i = 0; i < nVertices; i++)
    {
        (void)snprintf(text,
                       sizeof(text),
                       "%s %s %s\n",
                       toString(mesh.position[i * 3 + 0] * s).c_str(),
                       toString(mesh.position[i * 3 + 1] * s).c_str(),
                       toString(mesh.position[i * 3 + 2] * s).c_str());
        f.write(text);
    }

    if (mesh.mode == Mesh::Mode::MODE_TRIANGLES)
    {
        if (mesh.indices.empty())
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
        else
        {
            for (unsigned int i = 0; i < nElements; i++)
            {
                (void)snprintf(text,
                               sizeof(text),
                               "3 %u %u %u\n",
                               mesh.indices[i * 3],
                               mesh.indices[i * 3 + 1],
                               mesh.indices[i * 3 + 2]);
                f.write(text);
            }
        }
    }

    f.close();
}
