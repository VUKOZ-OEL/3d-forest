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

/** @file ProgressActionInterface.hpp */

#ifndef PROGRESS_ACTION_INTERFACE_HPP
#define PROGRESS_ACTION_INTERFACE_HPP

#include <ProgressCounter.hpp>

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Progress Action Interface. */
class EXPORT_EDITOR ProgressActionInterface
{
public:
    ProgressActionInterface();
    virtual ~ProgressActionInterface() = default;

    virtual void next() = 0;

    bool end() const { return progress_.end(); }

    size_t progressMaximumSteps() const { return progress_.maximumSteps(); }
    size_t progressValueSteps() const { return progress_.valueSteps(); }
    uint64_t progressMaximumStep() const { return progress_.maximumStep(); }
    uint64_t progressValueStep() const { return progress_.valueStep(); }
    double progressPercent() const { return progress_.percent(); }

protected:
    ProgressCounter progress_;
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_ACTION_INTERFACE_HPP */
