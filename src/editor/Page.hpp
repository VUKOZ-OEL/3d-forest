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

/** @file Page.hpp */

#ifndef PAGE_HPP
#define PAGE_HPP

#include <IndexFile.hpp>

class Editor;
class Query;

/** Page. */
class Page
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
    std::vector<float> intensity;

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
    std::vector<float> color;

    /** Red, Green, and Blue output colors.
        The data are stored as [r0, g0, b0, r1, g1, ...].
        Color values are in range from 0 (zero intensity) to 1 (full intensity).
        This value is stored in Point Data Record extra bytes.
    */
    std::vector<float> userColor;

    /** Layer identification numbers.
        This value is stored in Point Data Record extra bytes.
    */
    std::vector<uint32_t> layer;
    /**@}*/

    /** Index array contains indices to selected points. */
    std::vector<uint32_t> selection;
    size_t selectionSize;

    /** Bounding box. */
    Box<double> box;

    /** Octree. */
    IndexFile octree;

    /** @name Rendering */
    /**@{*/
    /** Rendering Point Coordinates.
        The data are stored as [x0, y0, z0, x1, y1, ...].
     */
    std::vector<float> renderPosition;

    /** Rendering Point Colors.
        The data are stored as [r0, g0, b0, r1, g1, ...].
     */
    std::vector<float> renderColor;
    /**@}*/

    Page(Editor *editor, Query *query, uint32_t datasetId, uint32_t pageId);
    ~Page();

    uint32_t datasetId() const { return datasetId_; }
    uint32_t pageId() const { return pageId_; }

    // All states
    void read();
    void write();

    // Individual states
    /** Page State. */
    enum State
    {
        STATE_READ,
        STATE_TRANSFORM,
        STATE_SELECT,
        STATE_RUN_MODIFIERS,
        STATE_RENDER,
        STATE_RENDERED
    };

    Page::State state() const { return state_; }
    void setState(Page::State state);
    bool nextState();

    void setModified();
    bool isModified() const { return modified_; }

protected:
    // Parent
    Editor *editor_;
    Query *query_;

    // Identifier
    uint32_t datasetId_;
    uint32_t pageId_;

    // State
    Page::State state_;
    bool modified_;

    // File data
    std::vector<uint8_t> buffer_;
    std::vector<double> positionBase_;

    // Buffer
    std::vector<IndexFile::Selection> selectedNodes_;

    void clear();
    void resize(size_t n);

    void toPoint(uint8_t *ptr, size_t i, uint8_t fmt);

    void transform();
    void select();

    void selectBox();
    void selectCone();
    void selectSphere();
    void selectClassification();
    void selectLayer();

    void runModifiers();
    void runColorModifier();

    void setColor(size_t idx,
                  size_t value,
                  size_t max,
                  const std::vector<Vector3<float>> &pal);
};

#endif /* PAGE_HPP */
