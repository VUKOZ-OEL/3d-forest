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

/** @file Voxels.hpp */

#ifndef VOXELS_HPP
#define VOXELS_HPP

#include <ExportEditor.hpp>
#include <Voxel.hpp>

class Query;

/** Voxels. */
class EXPORT_EDITOR Voxels
{
public:
    Voxels();

    void clear();

    void create(const Box<double> &spaceRegion, double voxelSize);

    const Box<double> &spaceRegion() const { return spaceRegion_; }
    const Vector3<double> &voxelSize() const { return voxelSize_; }

    bool next(Voxel *voxel,
              Box<double> *cell = nullptr,
              Query *query = nullptr);

    uint64_t visitedVoxelsCount() const { return visitedVoxelsCount_; }

    // Occupancy
    size_t size() const { return voxels_.size(); }

    void addVoxel(const Voxel &voxel);

    Voxel &at(size_t index) { return voxels_[index]; }
    const Voxel &at(size_t index) const { return voxels_[index]; }
    Voxel &at(size_t x, size_t y, size_t z) { return voxels_[find(x, y, z)]; }
    const Voxel &at(size_t x, size_t y, size_t z) const
    {
        return voxels_[find(x, y, z)];
    }

    // Sort
    void sort(double elevationMinimum, double elevationMaximum);
    size_t sortedSize() const { return sortedVoxels_.size(); }
    Voxel &sortedAt(size_t index) { return *sortedVoxels_[index]; }
    const Voxel &sortedAt(size_t index) const { return *sortedVoxels_[index]; }

    // Grid
    size_t sizeX() const { return nx_; }
    size_t sizeY() const { return ny_; }
    size_t sizeZ() const { return nz_; }
    size_t indexSize() const { return index_.size(); }

    static const size_t npos;

    bool contains(size_t x, size_t y, size_t z) const
    {
        return find(x, y, z) != npos;
    }

    size_t find(size_t x, size_t y, size_t z) const
    {
        return index_[indexOf(x, y, z)];
    }

    size_t indexOf(size_t x, size_t y, size_t z) const
    {
        return x + (y * nx_) + (z * nx_ * ny_);
    }

    size_t indexOf(const Voxel &voxel) const
    {
        return indexOf(voxel.x_, voxel.y_, voxel.z_);
    }

    void box(const Voxel &voxel, Box<double> *box);

    // Values
    double descriptorMin() const { return descriptorMin_; }
    double descriptorMax() const { return descriptorMax_; }

    void normalize(Voxel *voxel);

    // Debug
    void dump() const;

private:
    // Region
    Box<double> spaceRegion_;
    double voxelSizeInput_;
    Vector3<double> voxelSize_;

    // Index
    size_t nx_;
    size_t ny_;
    size_t nz_;
    std::vector<size_t> index_;

    // Voxels
    std::vector<Voxel> voxels_;

    // Sort
    std::vector<Voxel *> sortedVoxels_;

    // Values
    double descriptorMin_;
    double descriptorMax_;

    // Create
    std::vector<Box<size_t>> stack_;
    uint64_t visitedVoxelsCount_;

    void push(size_t x1, size_t y1, size_t z1, size_t x2, size_t y2, size_t z2);

    bool next(Query *query,
              Box<double> *cell = nullptr,
              size_t *index = nullptr,
              uint32_t *x = nullptr,
              uint32_t *y = nullptr,
              uint32_t *z = nullptr);
};

inline std::ostream &operator<<(std::ostream &os, const Voxels &obj)
{
    // clang-format off
    return os << std::fixed
              << "(" << obj.size()
              << ", (" << obj.sizeX() << " x "
                       << obj.sizeY() << " x "
                       << obj.sizeZ()
              << "), " << obj.voxelSize()
              << ", " << obj.spaceRegion()
              << ")"
              << std::defaultfloat;
    // clang-format on
}

#endif /* VOXELS_HPP */
