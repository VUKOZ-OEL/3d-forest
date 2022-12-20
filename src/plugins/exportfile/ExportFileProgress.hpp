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

/** @file ExportFileProgress.hpp */

#ifndef EXPORT_FILE_PROGRESS_HPP
#define EXPORT_FILE_PROGRESS_HPP

#include <ExportFileFormat.hpp>
#include <ExportFileProperties.hpp>
#include <LasFile.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>

class Editor;

/** Export File Progress. */
class ExportFileProgress : public ProgressActionInterface
{
public:
    ExportFileProgress(Editor *editor);
    virtual ~ExportFileProgress();

    void initialize(std::shared_ptr<ExportFileFormat> writer,
                    const ExportFileProperties &properties);
    void clear();

    virtual void step();

private:
    Editor *editor_;
    Query query_;

    uint64_t nPointsTotal_;
    Vector3<double> regionMin_;
    Vector3<double> regionMax_;

    std::shared_ptr<ExportFileFormat> writer_;
    ExportFileProperties properties_;

    void determineMaximum();
};

#endif /* EXPORT_FILE_PROGRESS_HPP */
