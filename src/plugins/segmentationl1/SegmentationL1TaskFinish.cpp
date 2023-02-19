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

/** @file SegmentationL1TaskFinish.cpp */

#include <Editor.hpp>
#include <Mesh.hpp>
#include <SegmentationL1TaskFinish.hpp>

#define LOG_MODULE_NAME "SegmentationL1TaskFinish"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1TaskFinish::initialize(SegmentationL1Context *context)
{
    context_ = context;
    ProgressActionInterface::initialize(1);
}

void SegmentationL1TaskFinish::next()
{
    Layer layer;
    layer.set(1, "Points", Vector3<float>(1.0F, 0.0F, 0.0F));
    addMeshPoints(layer);
    addMeshNormals(layer);
    addMeshVectors(layer);

    Layers layers;
    layers.clear();
    layers.setDefault();
    layers.push_back(layer);

    LOG_DEBUG("Set <" << layers.size() << "> layers.");
    context_->editor->setLayers(layers);

    increment(maximum());
}

void SegmentationL1TaskFinish::addMeshPoints(Layer &layer)
{
    Mesh mesh;

    mesh.mode = Mesh::MODE_POINTS;

    size_t n = context_->samples.size();
    LOG_DEBUG("Create mesh from <" << n << "> samples.");

    mesh.xyz.resize(n * 3);
    mesh.rgb.resize(n * 3);

    for (size_t i = 0; i < n; i++)
    {
        SegmentationL1Point &point = context_->samples[i];

        mesh.xyz[i * 3 + 0] = static_cast<float>(point.x);
        mesh.xyz[i * 3 + 1] = static_cast<float>(point.y);
        mesh.xyz[i * 3 + 2] = static_cast<float>(point.z);

        mesh.rgb[i * 3 + 0] = 1.0F;
        mesh.rgb[i * 3 + 1] = 1.0F;
        mesh.rgb[i * 3 + 2] = 0.0F;
    }

    layer.addMesh(mesh);
}

void SegmentationL1TaskFinish::addMeshNormals(Layer &layer)
{
    Mesh mesh;

    mesh.mode = Mesh::MODE_LINES;

    size_t n = context_->samples.size();
    LOG_DEBUG("Create mesh from <" << n << "> samples.");

    mesh.xyz.resize(n * 3 * 2);
    mesh.rgb.resize(n * 3 * 2);

    for (size_t i = 0; i < n; i++)
    {
        SegmentationL1Point &point = context_->samples[i];

        Vector3<float> A(point.x, point.y, point.z);
        Vector3<float> N(point.nx, point.ny, point.nz);
        Vector3<float> B = A + (N * 100.0F);

        mesh.xyz[i * 6 + 0] = A[0];
        mesh.xyz[i * 6 + 1] = A[1];
        mesh.xyz[i * 6 + 2] = A[2];

        mesh.xyz[i * 6 + 3] = B[0];
        mesh.xyz[i * 6 + 4] = B[1];
        mesh.xyz[i * 6 + 5] = B[2];

        mesh.rgb[i * 6 + 0] = 0.0F;
        mesh.rgb[i * 6 + 1] = 1.0F;
        mesh.rgb[i * 6 + 2] = 0.0F;

        mesh.rgb[i * 6 + 3] = 0.0F;
        mesh.rgb[i * 6 + 4] = 1.0F;
        mesh.rgb[i * 6 + 5] = 0.0F;
    }

    layer.addMesh(mesh);
}

void SegmentationL1TaskFinish::addMeshVectors(Layer &layer)
{
    Mesh mesh;

    mesh.mode = Mesh::MODE_LINES;

    size_t n = context_->samples.size();
    LOG_DEBUG("Create mesh from <" << n << "> samples.");

    mesh.xyz.resize(n * 3 * 2);
    mesh.rgb.resize(n * 3 * 2);

    for (size_t i = 0; i < n; i++)
    {
        SegmentationL1Point &point = context_->samples[i];

        Vector3<float> A(point.x, point.y, point.z);
        Vector3<float> V(point.vx, point.vy, point.vz);
        Vector3<float> B = A + (V * 200.0F);

        mesh.xyz[i * 6 + 0] = A[0];
        mesh.xyz[i * 6 + 1] = A[1];
        mesh.xyz[i * 6 + 2] = A[2];

        mesh.xyz[i * 6 + 3] = B[0];
        mesh.xyz[i * 6 + 4] = B[1];
        mesh.xyz[i * 6 + 5] = B[2];

        mesh.rgb[i * 6 + 0] = 1.0F;
        mesh.rgb[i * 6 + 1] = 0.0F;
        mesh.rgb[i * 6 + 2] = 0.0F;

        mesh.rgb[i * 6 + 3] = 1.0F;
        mesh.rgb[i * 6 + 4] = 0.0F;
        mesh.rgb[i * 6 + 5] = 0.0F;
    }

    layer.addMesh(mesh);
}
