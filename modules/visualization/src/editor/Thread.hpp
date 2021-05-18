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

/**
    @file Thread.hpp
*/

#ifndef THREAD_HPP
#define THREAD_HPP

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

/** Thread. */
class Thread : public QThread
{
    Q_OBJECT

public:
    Thread(QObject *parent = nullptr);
    virtual ~Thread();

    void init();
    void start();
    void cancel();
    void stop();

    virtual bool compute() = 0;

protected:
    /** Thread state. */
    enum State
    {
        STATE_RUN,
        STATE_CANCEL,
        STATE_EXIT
    };

    // Thread state
    QMutex mutex_;
    QWaitCondition condition_;
    State state_;
    bool finished_;

    // Synchronization to caller
    QMutex mutexCaller_;
    QWaitCondition conditionCaller_;
    bool received_;

    void setState(State state);
    void run() override;
};

#endif /* THREAD_HPP */
