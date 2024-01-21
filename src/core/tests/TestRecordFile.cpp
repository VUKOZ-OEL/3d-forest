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

/** @file TestRecordFile.cpp */

#include <RecordFile.hpp>
#include <Test.hpp>

#define TEST_RECORD_FILE_PATH "test.bin"

TEST_CASE(TestRecordFileCreate1U32)
{
    // write
    RecordFile w;
    uint32_t wU32 = UINT32_MAX;
    w.create(TEST_RECORD_FILE_PATH, "foo", RecordFile::TYPE_U32);
    w.write(wU32);
    TEST(w.size() == 1);
    TEST(w.index() == 1);
    w.close();

    // read back
    RecordFile r;
    uint32_t rU32 = 0;
    r.open(TEST_RECORD_FILE_PATH);
    TEST(r.size() == 1);
    TEST(r.index() == 0);
    r.read(rU32);
    TEST(r.index() == 1);
    TEST(rU32 == UINT32_MAX);
}
