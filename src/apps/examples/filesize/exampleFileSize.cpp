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

/** @file exampleFileSize.cpp @brief File size example. */

#include <ArgumentParser.hpp>
#include <Error.hpp>
#include <File.hpp>

#define LOG_MODULE_NAME "exampleFileSize"
#include <Log.hpp>

static void exampleFileSize(const std::string &path)
{
    // Open file.
    File f;
    f.open(path);

    // Print file size.
    uint64_t size = f.size();
    std::cout << "file <" << path << "> size <" << size << ">" << std::endl;

    // Print some values above 32-bit file size.
    if (size >= 5007881695ULL)
    {
        f.seek(5007881680ULL);

        uint8_t buffer[16];
        f.read(buffer, 16);

        for (size_t i = 0; i < 16; i++)
        {
            int value = static_cast<int>(buffer[i]);
            std::cout << "byte <" << std::hex << value << ">" << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    try
    {
        ArgumentParser arg;
        arg.add("--input", "");
        arg.parse(argc, argv);

        exampleFileSize(arg.toString("--input"));
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
