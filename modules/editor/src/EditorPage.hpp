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

/** @file EditorPage.hpp */

#ifndef EDITOR_PAGE_HPP
#define EDITOR_PAGE_HPP

#include <EditorPoint.hpp>
#include <FileIndex.hpp>

class EditorDatabase;
class EditorQuery;

/** Editor Page. */
class EditorPage
{
public:
    /** Points. */
    std::vector<EditorPoint> points;

    /** Index array contains indices to filtered points. */
    std::vector<uint32_t> selection;

    /** Bounding box. */
    Box<double> box;

    /** Octree. */
    FileIndex octree;

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

    EditorPage(EditorDatabase *editor,
               EditorQuery *query,
               uint32_t datasetId,
               uint32_t pageId);
    ~EditorPage();

    uint32_t datasetId() const { return datasetId_; }
    uint32_t pageId() const { return pageId_; }

    // All states
    void read();
    void write();

    // Individual states
    void setStateRead();
    void setStateSelect();
    void setStateRender();
    bool nextState();
    bool nextStateRender();

protected:
    // Parent
    EditorDatabase *editor_;
    EditorQuery *query_;

    // Identifier
    uint32_t datasetId_;
    uint32_t pageId_;

    // State
    bool loaded;
    bool transformed;
    bool selected;
    bool rendered;
    bool modified;

    // File data
    std::vector<uint8_t> buffer_;
    std::vector<double> positionBase_;

    void clear();
    void resize(size_t n);

    void toPoint(uint8_t *ptr, size_t i, uint8_t fmt);

    void transform();
    void select();

    void selectBox();
    void selectClassification();
    void selectLayer();
    void selectColor();

    void setColor(size_t idx,
                  size_t value,
                  size_t max,
                  const std::vector<Vector3<float>> &pal);
};

#endif /* EDITOR_PAGE_HPP */
