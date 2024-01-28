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

/** @file TestVectorFile.cpp */

// Include 3D Forest.
#include <Endian.hpp>
#include <Test.hpp>
#include <VectorFile.hpp>

#define TEST_VECTOR_FILE_PATH "test.bin"

class IOUint32
{
public:
    static void read(const uint8_t *buffer, uint32_t &e)
    {
        e = ltoh32(&buffer[0]);
    }
    static void write(const uint32_t &e, uint8_t *buffer)
    {
        htol32(&buffer[0], e);
    }
};

TEST_CASE(TestVectorFileCreate1)
{
    VectorFile<uint32_t, IOUint32> w;
    w.create(TEST_VECTOR_FILE_PATH);
    w.push_back(1U);
    TEST(w.size() == 1);
    w.close();

    VectorFile<uint32_t, IOUint32> r;
    r.open(TEST_VECTOR_FILE_PATH);
    TEST(r.size() == 1);
    TEST(r.at(0) == 1U);
}

TEST_CASE(TestVectorFileCache)
{
    const size_t pageSize = 2;
    const size_t cacheSize = 3;
    const uint32_t n = 7;

    // Write.
    VectorFile<uint32_t, IOUint32> w;
    w.create(TEST_VECTOR_FILE_PATH, pageSize, cacheSize);

    for (uint32_t e = 0; e < n; e++)
    {
        w.push_back(e);
        TEST(w.size() == e + 1U);
    }

    w.close();

    // Read.
    VectorFile<uint32_t, IOUint32> r;
    r.open(TEST_VECTOR_FILE_PATH, pageSize, cacheSize);
    TEST(r.size() == n);
    for (uint32_t e = 0; e < n; e++)
    {
        TEST(r.at(e) == e);
    }

    // Read.
    VectorFile<uint32_t, IOUint32> r2;
    r2.open(TEST_VECTOR_FILE_PATH);
    TEST(r2.size() == n);
    for (uint32_t e = 0; e < n; e++)
    {
        TEST(r2.at(e) == e);
    }
}

class MyTestStruct
{
public:
    class IO
    {
    public:
        static void read(const uint8_t *buffer, MyTestStruct &e)
        {
            e.a = ltoh32(&buffer[0]);
            e.b = ltoh32(&buffer[4]);
        }
        static void write(const MyTestStruct &e, uint8_t *buffer)
        {
            htol32(&buffer[0], e.a);
            htol32(&buffer[4], e.b);
        }
    };

    uint32_t a;
    uint32_t b;
};

TEST_CASE(TestVectorFileCreateStruct)
{
    VectorFile<MyTestStruct, MyTestStruct::IO> w;
    w.create(TEST_VECTOR_FILE_PATH);
    w.push_back({1U, 2U});
    TEST(w.size() == 1);
    w.close();

    VectorFile<MyTestStruct, MyTestStruct::IO> r;
    r.open(TEST_VECTOR_FILE_PATH);
    TEST(r.size() == 1);
    TEST(r.at(0).a == 1U);
    TEST(r.at(0).b == 2U);
}
