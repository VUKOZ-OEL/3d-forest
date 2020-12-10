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
    @file fileinfo.cpp
*/

#include <LasFile.hpp>
#include <cstring>
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[])
{
    const char *filename = nullptr;

    for (int opt = 1; opt < argc; opt++)
    {
        if (strcmp(argv[opt], "-i") == 0)
        {
            opt++;
            if (opt < argc)
            {
                filename = argv[opt];
            }
        }
    }

    try
    {
        LasFile las;
        las.open(filename);

        Json obj;
        std::cout << las.header.serialize(obj).serialize() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
