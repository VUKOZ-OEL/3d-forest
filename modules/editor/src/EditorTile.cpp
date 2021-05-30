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

/**
    @file EditorTile.cpp
*/

#include <EditorTile.hpp>

EditorTile::EditorTile()
    : dataSetId(0),
      tileId(0),
      loaded(false),
      modified(false)
{
}

EditorTile::~EditorTile()
{
}

EditorTile::View::View() : renderStep(1), renderStepCount(1)
{
}

EditorTile::View::~View()
{
}

void EditorTile::View::resetFrame()
{
    renderStep = 1;
}

void EditorTile::View::nextFrame()
{
    renderStep++;
}

bool EditorTile::View::isStarted() const
{
    return renderStep == 1;
}

bool EditorTile::View::isFinished() const
{
    return renderStep > renderStepCount;
}
