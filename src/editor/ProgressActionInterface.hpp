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

#include <cstdint>

#include <ExportEditor.hpp>

/** Progress Action Interface. */
class EXPORT_EDITOR ProgressActionInterface
{
public:
    static const uint64_t npos = UINT64_MAX;

    ProgressActionInterface();
    virtual ~ProgressActionInterface() = default;

    virtual void step() = 0;

    void initialize(uint64_t nElements = npos,
                    uint64_t interleave = 1,
                    double secondsPerStep = 0.25);
    void startTimer();
    bool timedOut();
    bool end() const { return processed() >= maximum(); }

    double percent() const;
    void setProcessed(uint64_t n) { nElementsProcessed_ = n; }
    uint64_t processed() const { return nElementsProcessed_; }
    uint64_t process() const { return nElements_ - nElementsProcessed_; }
    uint64_t maximum() const { return nElements_; }
    bool initializing() const { return nElements_ == npos; }
    void increment(uint64_t nElementsProcessed);

private:
    uint64_t nElements_;
    uint64_t nElementsProcessed_;
    uint64_t interleave_;
    double secondsPerStep_;

    double timeBegin_;
    double timeNow_;
    uint64_t interleaveCounter_;
};

#endif /* PROGRESS_ACTION_INTERFACE_HPP */
