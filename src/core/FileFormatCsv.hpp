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

/** @file FileFormatCsv.hpp */

#ifndef FILE_FORMAT_CSV_HPP
#define FILE_FORMAT_CSV_HPP

// Include 3D Forest.
#include <FileFormatInterface.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** File Format CSV. */
class EXPORT_CORE FileFormatCsv : public FileFormatInterface
{
public:
    FileFormatCsv();
    virtual ~FileFormatCsv();

    virtual void create(const FileFormatTable &table);

    void setFileName(const std::string &fileName) { fileName_ = fileName; }
    virtual const std::string &fileName() const { return fileName_; }

private:
    std::string fileName_;
};

#include <WarningsEnable.hpp>

#endif /* FILE_FORMAT_CSV_HPP */
