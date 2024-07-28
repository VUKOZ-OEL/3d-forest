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

/** @file DbhAction.hpp */

#ifndef DBH_ACTION_HPP
#define DBH_ACTION_HPP

// Include 3D Forest.
#include <DbhParameters.hpp>
#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;
class Segment;

/** DBH (Diameter at Breast Height) Action. */
class DbhAction : public ProgressActionInterface
{
public:
    DbhAction(Editor *editor);
    virtual ~DbhAction();

    void start(const DbhParameters &parameters);
    virtual void next();
    void clear();

private:
    Editor *editor_;
    Query query_;

    DbhParameters parameters_;

    /** DBH Group. */
    class Group
    {
    public:
        size_t segmentId{0};
        std::vector<double> points; // x0, y0, z0, x1, y1, z1, ...
    };

    std::map<size_t, Group> groups_;

    void stepPointsToGroups();
};

#endif /* DBH_ACTION_HPP */
