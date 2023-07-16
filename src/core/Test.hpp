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

/** @file Test.hpp */

#ifndef TEST_HPP
#define TEST_HPP

#include <iostream>
#include <vector>

/** Test. */
class Test
{
public:
    /** Test Case. */
    class TestCase
    {
    public:
        virtual void run() = 0;
    };

    Test(Test const &) = delete;
    Test &operator=(Test const &) = delete;
    ~Test() = default;

    static Test *instance()
    {
        static Test testInstance{};
        return &testInstance;
    }

    void addTestCase(Test::TestCase *testCase)
    {
        testCases_.push_back(testCase);
    }

    void run()
    {
        for (auto &testCase : testCases_)
        {
            testCase->run();
        }
    }

    void addResult(const char *condition, bool pass, const char *file, int line)
    {
        if (!pass)
        {
            std::cout << "Test Failed: " << condition << " in file " << file
                      << " line " << line << std::endl;
            testsFailed_++;
        }
        else
        {
            testsPassed_++;
        }
    }

    void print()
    {
        std::cout << "Tests Passed: " << passed() << std::endl;
        std::cout << "Tests Failed: " << failed() << std::endl;
    }

    int passed() const { return testsPassed_; }
    int failed() const { return testsFailed_; }

private:
    explicit Test() : testsPassed_(0), testsFailed_(0){};
    int testsPassed_;
    int testsFailed_;
    std::vector<Test::TestCase *> testCases_;
};

#define TEST(cond)                                                             \
    do                                                                         \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            if (!(cond))                                                       \
            {                                                                  \
                Test::instance()->addResult(#cond, false, __FILE__, __LINE__); \
            }                                                                  \
            else                                                               \
            {                                                                  \
                Test::instance()->addResult(#cond, true, __FILE__, __LINE__);  \
            }                                                                  \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            Test::instance()->addResult(#cond, false, __FILE__, __LINE__);     \
        }                                                                      \
    } while (0)

#define TEST_CASE_NAME(name) testCase##name
#define TEST_CASE_CLASS_NAME(name) TestCase##name
#define TEST_CASE(name) TEST_CASE_CREATE(name)
#define TEST_CASE_CREATE(name)                                                 \
    class TEST_CASE_CLASS_NAME(name) : public Test::TestCase                   \
    {                                                                          \
    public:                                                                    \
        TEST_CASE_CLASS_NAME(name)() { Test::instance()->addTestCase(this); }  \
        virtual void run();                                                    \
    };                                                                         \
    static TEST_CASE_CLASS_NAME(name) TEST_CASE_NAME(name);                    \
    void TEST_CASE_CLASS_NAME(name)::run()

#endif /* TEST_HPP */
