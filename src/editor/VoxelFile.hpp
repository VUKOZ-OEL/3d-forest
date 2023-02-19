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

/** @file VoxelFile.hpp */

#ifndef VOXEL_FILE_HPP
#define VOXEL_FILE_HPP

#include <sstream>

#include <ExportEditor.hpp>
#include <File.hpp>

/** Voxel File. */
class EXPORT_EDITOR VoxelFile
{
public:
    static const uint64_t ELEMENT_SIZE;

    /** Voxel. */
    struct EXPORT_EDITOR Voxel
    {
        double x;
        double y;
        double z;
        double descriptor;
    };

    VoxelFile();
    ~VoxelFile();

    void create(const std::string &path);
    void open(const std::string &path);
    void close();

    void read(VoxelFile::Voxel &voxel);
    void write(VoxelFile::Voxel &voxel);

    void skip(uint64_t nvoxel);

    uint64_t nVoxels() const;
    uint64_t iVoxel() const;

private:
    File file_;
};

inline std::ostream &operator<<(std::ostream &os, const VoxelFile::Voxel &obj)
{
    return os << "x <" << obj.x << "> y <" << obj.y << "> z <" << obj.z
              << "> descriptor <" << obj.descriptor << ">";
}

#endif /* VOXEL_FILE_HPP */
