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

/** @file FileFormatCsv.cpp */

// Include 3D Forest.
#include <File.hpp>
#include <FileFormatCsv.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "FileFormatCsv"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FileFormatCsv::FileFormatCsv()
{
    LOG_DEBUG(<< "Create.");
}

FileFormatCsv::~FileFormatCsv()
{
    LOG_DEBUG(<< "Destroy.");
}

void FileFormatCsv::create(const FileFormatTable &table)
{
    // Create/overwrite new text file which is open for writing.
    File file;
    file.open(fileName(), "w+t");

    // Big buffer.
    char text[4096];

    // Table size.
    size_t colCount = table.columns.size();
    size_t rowCount = 0;
    if (colCount > 0)
    {
        rowCount = table.columns[0].cells.size();
    }

    // Write CSV header.
    text[0] = 0;

    for (size_t col = 0; col < colCount; ++col)
    {
        if (col > 0)
        {
            (void)ustrcat(text, ";");
        }

        (void)ustrcat(text, table.columns[col].header.c_str());
    }

    (void)ustrcat(text, "\n");

    file.write(text);

    // Write CSV data.
    for (size_t row = 0; row < rowCount; ++row)
    {
        text[0] = 0;

        for (size_t col = 0; col < colCount; ++col)
        {
            if (col > 0)
            {
                (void)ustrcat(text, ";");
            }

            (void)ustrcat(text, table.columns[col].cells[row].text.c_str());
        }

        (void)ustrcat(text, "\n");

        file.write(text);
    }

    // Close the file.
    file.close();
}
