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

/** @file ExportFileFormatInterface.hpp */

#ifndef EXPORT_FILE_FORMAT_INTERFACE_HPP
#define EXPORT_FILE_FORMAT_INTERFACE_HPP

// Include 3D Forest.
#include <ExportFileProperties.hpp>
#include <Query.hpp>

/** Export File Format. */
class ExportFileFormatInterface
{
public:
    virtual ~ExportFileFormatInterface() = default;

    virtual bool open() = 0;
    virtual void create(const std::string &path) = 0;
    virtual void write(Query &query) = 0;
    virtual void close() = 0;

    void setProperties(const ExportFileProperties &prop) { properties_ = prop; }
    const ExportFileProperties &properties() const { return properties_; }

private:
    ExportFileProperties properties_;
};

#endif /* EXPORT_FILE_FORMAT_INTERFACE_HPP */
