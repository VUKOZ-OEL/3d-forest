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

/** @file ExportFileFormatCsv.hpp */

#ifndef EXPORT_FILE_FORMAT_CSV_HPP
#define EXPORT_FILE_FORMAT_CSV_HPP

// Include 3D Forest.
#include <ExportFileFormatInterface.hpp>
#include <File.hpp>
#include <Query.hpp>

/** Export File in Comma Separated Values File Format. */
class ExportFileFormatCsv : public ExportFileFormatInterface
{
public:
    ExportFileFormatCsv();
    virtual ~ExportFileFormatCsv();

    virtual bool open() { return file_.open(); }
    virtual void create(const std::string &path);
    virtual void write(Query &query);
    virtual void close();

private:
    File file_;
};

#endif /* EXPORT_FILE_FORMAT_CSV_HPP */
