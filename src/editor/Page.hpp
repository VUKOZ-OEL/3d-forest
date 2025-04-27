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

// Include 3D Forest.
#include <PageData.hpp>
class Editor;
class Query;

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Page. */
class EXPORT_EDITOR Page
{
public:
    /** @name LAS Point Data. */
    /**@{*/
    /** Point coordinates.
        The data are stored as [x0, y0, z0, x1, y1, ...].
        These are actual X, Y, or Z coordinates after scaling and offset.
     */
    double *position;

    /** Pulse return magnitude.
        The data are stored as [i0, i1, ...].
        The values are in range from 0 (zero intensity) to 1 (full intensity).
    */
    double *intensity;

    /** Return number.
        Contains values from 0 to 15.
    */
    uint8_t *returnNumber;

    /** Number of returns.
        Contains values from 0 to 15.
    */
    uint8_t *numberOfReturns;

    /** Classification. */
    uint8_t *classification;

    /** User data. */
    uint8_t *userData;

    /** GPS time. */
    double *gpsTime;

    /** Red, Green, and Blue image channels.
        The data are stored as [r0, g0, b0, r1, g1, ...].
        Color values are in range from 0 (zero intensity) to 1 (full intensity).
        When the input data set has no colors, then the colors in this vector
        are set to full intensity.
    */
    double *color;
    /**@}*/

    /** @name 3D Forest Attributes. */
    /**@{*/
    /** Segment identification numbers.
        This value is stored in Point Data Record extra bytes.
    */
    size_t *segment;

    /** Point elevation above ground.
        The data are stored as [e0, e1, ...].
        This value is stored in Point Data Record extra bytes.
     */
    double *elevation;

    /** Descriptor values.
        The data are stored as [d0, d1, ...].
        The values are in range from 0 (zero) to 1 (full).
        This value is stored in Point Data Record extra bytes.
    */
    double *descriptor;

    /** Voxel values.
        This value is stored in Point Data Record extra bytes.
    */
    size_t *voxel;
    /**@}*/

    /** @name Rendering Data. */
    /**@{*/
    /** Rendering Point Coordinates.
        The data are stored as [x0, y0, z0, x1, y1, ...].
     */
    float *renderPosition;

    /** Rendering Point Colors.
        The data are stored as [r0, g0, b0, r1, g1, ...].
     */
    std::vector<float> renderColor;
    /**@}*/

    /** Index array contains indices to selected points. */
    std::vector<uint32_t> selection;
    size_t selectionSize;

    Page(Editor *editor, Query *query, uint32_t datasetId, uint32_t pageId);
    ~Page();

    uint32_t datasetId() const { return datasetId_; }
    uint32_t pageId() const { return pageId_; }

    void readPage();
    void writePage();

    size_t size() const;

    void setModified();
    bool modified() const;

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
    static std::string stateToString(Page::State state);

private:
    // Parent.
    Editor *editor_;
    Query *query_;

    // Identifier.
    uint32_t datasetId_;
    uint32_t pageId_;

    // State.
    Page::State state_;

    // Data.
    std::shared_ptr<PageData> pageData_;

    // Buffer.
    std::vector<IndexFile::Selection> selectedNodes_;

    void resize(size_t n);

    void transform();

    void queryWhere();
    void queryWhereBox();
    void queryWhereCone();
    void queryWhereCylinder();
    void queryWhereSphere();
    void queryWhereElevation();
    void queryWhereDescriptor();
    void queryWhereIntensity();
    void queryWhereClassification();
    void queryWhereSegment();
    void queryWhereSpecies();

    void runModifiers();
    void runColorModifier();

    void setColor(size_t idx,
                  size_t colorValue,
                  size_t colorMax,
                  const std::vector<Vector3<double>> &pal);
};

inline std::ostream &operator<<(std::ostream &out, const Page::State &in)
{
    switch (in)
    {
        case Page::STATE_READ:
            out << "READ";
            break;
        case Page::STATE_TRANSFORM:
            out << "TRANSFORM";
            break;
        case Page::STATE_SELECT:
            out << "SELECT";
            break;
        case Page::STATE_RUN_MODIFIERS:
            out << "RUN_MODIFIERS";
            break;
        case Page::STATE_RENDER:
            out << "RENDER";
            break;
        case Page::STATE_RENDERED:
            out << "RENDERED";
            break;
        default:
            out << "UNKNOWN";
            break;
    }

    return out;
}

#include <WarningsEnable.hpp>

#endif /* PAGE_HPP */
