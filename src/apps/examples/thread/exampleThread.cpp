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

/** @file exampleThread.cpp @brief Thread example. */

#include <Log.hpp>
#include <Thread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

/** Custom thread. */
class MyThread : public Thread
{
public:
    MyThread() : counter_(0) {}

    virtual bool compute()
    {
        std::cout << "MyThread::compute::counter=" << counter_ << std::endl;

        Time::msleep(1000);
        counter_++;

        bool finished = counter_ > 2;
        if (callback_)
        {
            callback_->threadProgress(finished);
        }

        return finished;
    }

private:
    int counter_;
};

/** Some class with backround thread. */
class MyClass : public ThreadCallbackInterface
{
public:
    MyClass() {}
    virtual ~MyClass() {}

    virtual void threadProgress(bool finished)
    {
        std::cout << "MyClass::threadProgress::finished="
                  << static_cast<int>(finished) << std::endl;
    }

    void run()
    {
        thread_.setCallback(this);
        thread_.create();
        thread_.start();
        thread_.wait();
    }

private:
    MyThread thread_;
};

static void exampleThread()
{
    MyClass c;
    c.run();
}

int main()
{
    try
    {
        exampleThread();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
