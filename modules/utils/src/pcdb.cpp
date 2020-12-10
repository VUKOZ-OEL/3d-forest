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
    @file pcdb.cpp
*/

#include <Aabb.hpp>
#include <Error.hpp>
#include <SpatialIndex.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>

enum Command
{
    COMMAND_NONE,
    COMMAND_CREATE_INDEX,
    COMMAND_PRINT,
    COMMAND_SELECT
};

void getarg(size_t *v, int &opt, int argc, char *argv[])
{
    opt++;
    if (opt < argc)
    {
        *v = std::stoul(argv[opt]);
    }
}

void getarg(double *v, int &opt, int argc, char *argv[])
{
    opt++;
    if (opt < argc)
    {
        *v = std::stod(argv[opt]);
    }
}

void getarg(const char **v, int &opt, int argc, char *argv[])
{
    opt++;
    if (opt < argc)
    {
        *v = argv[opt];
    }
}

void cmd_create_index(const char *filename_out,
                      const char *filename_in,
                      size_t maxlevel)
{
    if ((!filename_out) || (!filename_in))
    {
        THROW("Invalid arguments");
    }

    SpatialIndex::create(filename_out, filename_in, maxlevel);
}

void cmd_print(const char *filename_in)
{
    if (!filename_in)
    {
        THROW("Invalid arguments");
    }
}

void cmd_select(const char *filename_in, const Aabbd &window)
{
    if (!filename_in)
    {
        THROW("Invalid arguments");
    }

    (void)window;
}

int main(int argc, char *argv[])
{
    int command = COMMAND_NONE;
    size_t maxlevel = 2;
    double wx1 = 0, wy1 = 0, wz1 = 0, wx2 = 0, wy2 = 0, wz2 = 0;
    Aabbd window;
    const char *filename_out = nullptr;
    const char *filename_in = nullptr;

    for (int opt = 1; opt < argc; opt++)
    {
        if (strcmp(argv[opt], "-x") == 0)
        {
            command = COMMAND_CREATE_INDEX;
        }
        else if (strcmp(argv[opt], "-p") == 0)
        {
            command = COMMAND_PRINT;
        }
        else if (strcmp(argv[opt], "-s") == 0)
        {
            command = COMMAND_SELECT;
        }
        else if (strcmp(argv[opt], "-l") == 0)
        {
            getarg(&maxlevel, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-i") == 0)
        {
            getarg(&filename_in, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-o") == 0)
        {
            getarg(&filename_out, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-x1") == 0)
        {
            getarg(&wx1, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-y1") == 0)
        {
            getarg(&wy1, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-z1") == 0)
        {
            getarg(&wz1, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-x2") == 0)
        {
            getarg(&wx2, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-y2") == 0)
        {
            getarg(&wy2, opt, argc, argv);
        }
        else if (strcmp(argv[opt], "-z2") == 0)
        {
            getarg(&wz2, opt, argc, argv);
        }
    }

    try
    {
        switch (command)
        {
            case COMMAND_CREATE_INDEX:
                cmd_create_index(filename_out, filename_in, maxlevel);
                break;
            case COMMAND_PRINT:
                cmd_print(filename_in);
                break;
            case COMMAND_SELECT:
                window.set(wx1, wy1, wz1, wx2, wy2, wz2);
                cmd_select(filename_in, window);
                break;
            case COMMAND_NONE:
            default:
                THROW("Unknown command");
                break;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
