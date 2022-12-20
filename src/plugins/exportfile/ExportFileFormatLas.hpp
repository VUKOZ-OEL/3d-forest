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

/** @file ExportFileFormatLas.hpp */

#ifndef EXPORT_FILE_FORMAT_LAS_HPP
#define EXPORT_FILE_FORMAT_LAS_HPP

#include <ExportFileFormat.hpp>
#include <LasFile.hpp>
#include <Query.hpp>

/** Export File in LAS (LASer) File Format. */
class ExportFileFormatLas : public ExportFileFormat
{
public:
    ExportFileFormatLas();
    virtual ~ExportFileFormatLas();

    virtual bool isOpen() { return file_.file().isOpen(); }
    virtual void create(const std::string &path);
    virtual void write(Query &query);
    virtual void close();

private:
    LasFile file_;
};

#endif /* EXPORT_FILE_FORMAT_LAS_HPP */
