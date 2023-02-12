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

/** @file SegmentationL1TaskFinish.hpp */

#ifndef SEGMENTATION_L1_TASK_FINISH_HPP
#define SEGMENTATION_L1_TASK_FINISH_HPP

#include <Editor.hpp>
#include <Mesh.hpp>
#include <SegmentationL1TaskInterface.hpp>

/** Segmentation L1 Task Finish. */
class SegmentationL1TaskFinish : public SegmentationL1TaskInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;
        ProgressActionInterface::initialize(1);
    }

    virtual void next()
    {
        Mesh mesh;

        size_t n = context_->samples.size();
        mesh.xyz.resize(n * 3);
        mesh.rgb.resize(n * 3);

        for (size_t i = 0; i < n; i++)
        {
            SegmentationL1Point &point = context_->samples[i];
            mesh.xyz[i * 3 + 0] = static_cast<float>(point.x);
            mesh.xyz[i * 3 + 1] = static_cast<float>(point.y);
            mesh.xyz[i * 3 + 2] = static_cast<float>(point.z);

            mesh.rgb[i * 3 + 0] = 1.0F;
            mesh.rgb[i * 3 + 1] = 0.0F;
            mesh.rgb[i * 3 + 2] = 0.0F;
        }

        Layer layer;
        layer.set(1, "Points", Vector3<float>(1.0F, 0.0F, 0.0F));
        layer.setMesh(mesh);

        Layers layers;
        layers.clear();
        layers.setDefault();
        layers.push_back(layer);

        context_->editor->setLayers(layers);

        increment(maximum());
    }

private:
    SegmentationL1Context *context_;
};

#endif /* SEGMENTATION_L1_TASK_FINISH_HPP */
