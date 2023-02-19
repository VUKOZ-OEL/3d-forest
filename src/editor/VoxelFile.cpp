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

/** @file VoxelFile.cpp */

#include <Endian.hpp>
#include <VoxelFile.hpp>

#if !defined(EXPORT_EDITOR_IMPORT)
const uint64_t VoxelFile::ELEMENT_SIZE = sizeof(VoxelFile::Voxel);
#endif

VoxelFile::VoxelFile()
{
}

VoxelFile::~VoxelFile()
{
    file_.close();
}

void VoxelFile::create(const std::string &path)
{
    file_.create(path);
}

void VoxelFile::open(const std::string &path)
{
    file_.open(path);
}

void VoxelFile::close()
{
    file_.close();
}

void VoxelFile::read(VoxelFile::Voxel &voxel)
{
    uint8_t buffer[32];
    file_.read(buffer, VoxelFile::ELEMENT_SIZE);
    voxel.x = ltohd(&buffer[0]);
    voxel.y = ltohd(&buffer[8]);
    voxel.z = ltohd(&buffer[16]);
    voxel.descriptor = ltohd(&buffer[24]);
}

void VoxelFile::write(VoxelFile::Voxel &voxel)
{
    uint8_t buffer[32];
    htold(&buffer[0], voxel.x);
    htold(&buffer[8], voxel.y);
    htold(&buffer[16], voxel.z);
    htold(&buffer[24], voxel.descriptor);
    file_.write(buffer, VoxelFile::ELEMENT_SIZE);
}

void VoxelFile::skip(uint64_t nvoxel)
{
    if (nvoxel > 0)
    {
        file_.skip(nvoxel * VoxelFile::ELEMENT_SIZE);
    }
}

uint64_t VoxelFile::nVoxels() const
{
    return file_.size() / VoxelFile::ELEMENT_SIZE;
}

uint64_t VoxelFile::iVoxel() const
{
    return file_.offset() / VoxelFile::ELEMENT_SIZE;
}
