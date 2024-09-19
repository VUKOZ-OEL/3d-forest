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

/** @file PageData.hpp */

#ifndef PAGE_DATA_HPP
#define PAGE_DATA_HPP

// Include 3D Forest.
#include <IndexFile.hpp>
class Editor;

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Page Data. */
class EXPORT_EDITOR PageData
{
public:
    /** @name Point Data */
    /**@{*/
    /** Point coordinates.
        The data are stored as [x0, y0, z0, x1, y1, ...].
        These are actual X, Y, or Z coordinates after scaling and offset.
     */
    std::vector<double> position;

    /** Pulse return magnitude.
        The data are stored as [i0, i1, ...].
        The values are in range from 0 (zero intensity) to 1 (full intensity).
    */
    std::vector<double> intensity;

    /** Return number.
        Contains values from 0 to 15.
    */
    std::vector<uint8_t> returnNumber;

    /** Number of returns.
        Contains values from 0 to 15.
    */
    std::vector<uint8_t> numberOfReturns;

    /** Classification. */
    std::vector<uint8_t> classification;

    /** User data. */
    std::vector<uint8_t> userData;

    /** GPS time. */
    std::vector<double> gpsTime;

    /** Red, Green, and Blue image channels.
        The data are stored as [r0, g0, b0, r1, g1, ...].
        Color values are in range from 0 (zero intensity) to 1 (full intensity).
        When the input data set has no colors, then the colors in this vector
        are set to full intensity.
    */
    std::vector<double> color;
    /**@}*/

    /** @name Point Data Extra Bytes */
    /**@{*/
    /** Segment identification numbers.
        This value is stored in Point Data Record extra bytes.
    */
    std::vector<size_t> segment;

    /** Point elevation above ground.
        The data are stored as [e0, e1, ...].
        This value is stored in Point Data Record extra bytes.
     */
    std::vector<double> elevation;

    /** Descriptor values.
        The data are stored as [d0, d1, ...].
        The values are in range from 0 (zero) to 1 (full).
        This value is stored in Point Data Record extra bytes.
    */
    std::vector<double> descriptor;

    /** Voxel values.
        This value is stored in Point Data Record extra bytes.
    */
    std::vector<size_t> voxel;
    /**@}*/

    /** @name Rendering */
    /**@{*/
    /** Rendering Point Coordinates.
        The data are stored as [x0, y0, z0, x1, y1, ...].
     */
    std::vector<float> renderPosition;
    /**@}*/

    /** Bounding box. */
    Box<double> box;

    /** Octree. */
    IndexFile octree;

    PageData(uint32_t datasetId, uint32_t pageId);
    ~PageData();

    uint32_t datasetId() const { return datasetId_; }
    uint32_t pageId() const { return pageId_; }

    void readPage(Editor *editor);
    void writePage(Editor *editor);

    void transform(Editor *editor);

    size_t size() const { return intensity.size(); }

    void setModified() { modified_ = true; }
    bool modified() const { return modified_; }

private:
    /** Dataset identifier. */
    uint32_t datasetId_;

    /** Page identifier in a dataset. */
    uint32_t pageId_;

    /** When true, this page should be written back to hard drive. */
    bool modified_;

    /** File buffer to preserve untouched LAS data for updates. */
    std::vector<uint8_t> pointDataBuffer_;

    /** Original xyz position of each point for translations. */
    std::vector<double> positionBase_;

    void resize(size_t n);
    void updatePoint(uint8_t *ptr, size_t i, uint8_t fmt);
};

#include <WarningsEnable.hpp>

#endif /* PAGE_DATA_HPP */
