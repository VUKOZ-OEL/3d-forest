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

/** @file SegmentationL1.cpp */

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Log.hpp>
#include <SegmentationL1.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "SegmentationL1"

SegmentationL1::SegmentationL1(Editor *editor)
    : editor_(editor),
      query_(editor_)
{
    LOG_DEBUG(<< "Called.");
}

SegmentationL1::~SegmentationL1()
{
    LOG_DEBUG(<< "Called.");
}

void SegmentationL1::clear()
{
    query_.clear();
}

void SegmentationL1::restart(const SegmentationL1Parameters &parameters)
{
    LOG_DEBUG(<< "Called.");
}

bool SegmentationL1::compute()
{
    LOG_DEBUG(<< "Compute.");
    return true;
}

int SegmentationL1::progressPercent() const
{
    return 100;
}
