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

/** @file ClassificationAction.hpp */

#ifndef CLASSIFICATION_ACTION_HPP
#define CLASSIFICATION_ACTION_HPP

#include <ProgressActionInterface.hpp>
#include <Query.hpp>

class Editor;

/** Classification Action. */
class ClassificationAction : public ProgressActionInterface
{
public:
    ClassificationAction(Editor *editor);
    virtual ~ClassificationAction();

    void initialize(size_t pointsPerCell = 10000,
                    double cellLengthMinPercent = 1.,
                    double groundErrorPercent = 15.,
                    double angleDeg = 60.);
    virtual void next();
    void clear();

protected:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    double groundErrorPercent_;
    double angleDeg_;

    void stepGrid();
};

#endif /* CLASSIFICATION_ACTION_HPP */
