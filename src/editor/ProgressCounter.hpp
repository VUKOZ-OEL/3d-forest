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

/** @file ProgressCounter.hpp */

#ifndef PROGRESS_COUNTER_HPP
#define PROGRESS_COUNTER_HPP

#include <cstdint>
#include <vector>

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Progress Counter. */
class EXPORT_EDITOR ProgressCounter
{
public:
    static const uint64_t npos = UINT64_MAX;

    ProgressCounter();
    ~ProgressCounter() = default;

    void setMaximumStep(uint64_t n = npos,
                        uint64_t interleave = 1,
                        double timeout = 0.25);
    uint64_t maximumStep() const { return maximumStep_; }
    void setValueStep(uint64_t n) { valueStep_ = n; }
    void addValueStep(uint64_t n) { valueStep_ += n; };
    uint64_t valueStep() const { return valueStep_; }

    void setMaximumSteps(size_t n);
    void setMaximumSteps(const std::vector<double> &percent);
    size_t maximumSteps() const { return maximumSteps_; }
    void setValueSteps(size_t n) { valueSteps_ = n; };
    size_t valueSteps() const { return valueSteps_; }

    void startTimer();
    bool timedOut();

    bool end() const
    {
        return (valueStep_ >= maximumStep_) && (valueSteps_ >= maximumSteps_);
    }

    double percent() const;
    int percentToInt() const { return static_cast<int>(percent()); }

    bool initializing() const { return maximumStep_ == npos; }

private:
    size_t maximumSteps_;
    size_t valueSteps_;
    std::vector<double> percentSteps_;

    uint64_t maximumStep_;
    uint64_t valueStep_;

    double timeout_;
    double timeBegin_;
    double timeNow_;

    uint64_t interleave_;
    uint64_t interleaveCounter_;

    double percentStep() const;
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_COUNTER_HPP */
