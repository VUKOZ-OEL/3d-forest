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

/** @file AlgorithmThread.hpp */

#ifndef ALGORITHM_THREAD_HPP
#define ALGORITHM_THREAD_HPP

#include <Query.hpp>
#include <Thread.hpp>
class Editor;
class AlgorithmWidgetInterface;

/** Algorithm Thread. */
class AlgorithmThread : public Thread
{
public:
    AlgorithmThread();
    virtual ~AlgorithmThread();

    void clear();
    void restart(AlgorithmWidgetInterface *algorithm, bool autoStart);
    virtual bool next();
    void progress(size_t &nTasks, size_t &iTask, double &percent) const;
    void updateData();

private:
    AlgorithmWidgetInterface *algorithm_;
};

#endif /* ALGORITHM_THREAD_HPP */
