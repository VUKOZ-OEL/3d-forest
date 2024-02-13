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

/** @file exampleFile.cpp @brief File example. */

// Include 3D Forest.
#include <Error.hpp>
#include <File.hpp>

// Include local.
#define LOG_MODULE_NAME "exampleFile"
#include <Log.hpp>

#define TEST(cmd) std::cout << #cmd << " <" << (cmd) << ">" << std::endl;

static void exampleFile()
{
#if defined(_MSC_VER)
    std::cout << "<_MSC_VER> is defined" << std::endl;
#else
    std::cout << "<_MSC_VER> is not defined" << std::endl;
#endif

    TEST(File::currentPath());

    TEST(File::isAbsolute("../Documents/file.txt"));
    TEST(File::isAbsolute("/tmp/file.txt"));
    TEST(File::isAbsolute("C:\\tmp\\file.txt"));

    TEST(File::fileName("file.txt"));
    TEST(File::fileName("/tmp/file.txt"));
    TEST(File::fileName("/tmp/directory/"));
    TEST(File::fileName("/"));
    TEST(File::fileName("C:\\tmp\\file.txt"));
    TEST(File::fileName("C:\\tmp\\directory\\"));
    TEST(File::fileName("C:\\"));
    TEST(File::fileName(""));

    TEST(File::fileExtension("file.txt"));
    TEST(File::fileExtension("/tmp/file.txt"));
    TEST(File::fileExtension("/tmp/directory/"));
    TEST(File::fileExtension(""));

    TEST(File::replaceFileName("file.txt", "data.xml"));
    TEST(File::replaceFileName("/tmp/file.txt", "data.xml"));
    TEST(File::replaceFileName("/tmp/directory/", "data.xml"));
    TEST(File::replaceFileName("", "data.xml"));

    TEST(File::replaceExtension("file.txt", ".xml"));
    TEST(File::replaceExtension("/tmp/file.txt", ".xml"));
    TEST(File::replaceExtension("/tmp/directory/", ".xml"));
    TEST(File::replaceExtension("", ".xml"));
}

int main()
{
    try
    {
        exampleFile();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
