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

/** @file examplethread.cpp @brief Thread example. */

#include <Log.hpp>
#include <Thread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

/** Custom thread. */
class MyThread : public Thread
{
public:
    MyThread() : counter_(3) {}

    virtual bool compute()
    {
        std::cout << "MyThread::compute::counter=" << counter_ << std::endl;
        counter_--;
        return counter_ < 1; // finished
    }

protected:
    int counter_;
};

/** Some class with backround thread. */
class MyClass : public ThreadCallbackInterface
{
public:
    MyClass()
    {
        thread_.setCallback(this);
        thread_.create();
        thread_.start();
    }

    virtual ~MyClass() { thread_.stop(); }

    virtual void threadProgress(bool finished)
    {
        std::cout << "MyClass::threadProgress::finished="
                  << static_cast<int>(finished) << std::endl;
    }

protected:
    MyThread thread_;
};

int main()
{
    std::cout << "main" << std::endl;

    MyClass c;

    msleep(500);

    return 0;
}
