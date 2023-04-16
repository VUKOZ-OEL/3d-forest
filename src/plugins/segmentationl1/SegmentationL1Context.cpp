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

/** @file SegmentationL1Context.cpp */

#include <Editor.hpp>
#include <SegmentationL1Context.hpp>

#define LOG_MODULE_NAME "SegmentationL1Context"
#include <Log.hpp>

SegmentationL1Context::SegmentationL1Context(Editor *editor_)
    : editor(editor_),
      query(editor_)
{
    clear();
}

void SegmentationL1Context::clear()
{
    query.clear();
    parameters.clear();
    nPoints = 0;
    samples.clear();
    samplesBackup.clear();
    median.clear();
}

void SegmentationL1Context::execInitialSamplesQuery()
{
    // Create descriptor filter for initial samples.
    double min = static_cast<double>(parameters.sampleDescriptorMinimum);
    double max = static_cast<double>(parameters.sampleDescriptorMaximum);

    Range<double> descriptor;
    descriptor.set(0.0, 1.0, min * 0.01, max * 0.01);

    // Setup the query.
    query.setWhere(editor->viewports().where());
    query.where().setDescriptor(descriptor);
    query.exec();
}