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

/** @file ExportFileAction.cpp */

#include <cstring>

#include <Editor.hpp>
#include <ExportFileAction.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "ExportFileAction"

ExportFileAction::ExportFileAction(Editor *editor)
    : ProgressActionInterface(),
      editor_(editor),
      query_(editor)
{
    LOG_DEBUG(<< "Called.");
}

ExportFileAction::~ExportFileAction()
{
    LOG_DEBUG(<< "Called.");
}

void ExportFileAction::initialize(std::shared_ptr<ExportFileFormat> writer,
                                  const ExportFileProperties &properties)
{
    LOG_DEBUG(<< "Called.");

    writer_ = writer;
    properties_ = properties;

    nPointsTotal_ = 0;
    regionMin_.clear();
    regionMax_.clear();

    if (properties_.isFilterEnabled())
    {
        query_.setWhere(editor_->viewports().where());
    }

    query_.exec();

    if (query_.next())
    {
        regionMin_.set(query_.x(), query_.y(), query_.z());
        regionMax_ = regionMin_;

        nPointsTotal_++;
    }

    ProgressActionInterface::initialize(ProgressActionInterface::npos, 1000UL);
}

void ExportFileAction::clear()
{
    LOG_DEBUG(<< "Called.");
    query_.clear();
}

void ExportFileAction::step()
{
    if (initializing())
    {
        determineMaximum();
        return;
    }

    uint64_t n = process();
    uint64_t i = 0;

    startTimer();

    if (!writer_->isOpen())
    {
        writer_->create(properties_.fileName());
    }

    while (i < n)
    {
        if (query_.next())
        {
            writer_->write(query_);
        }

        i++;

        if (timedOut())
        {
            break;
        }
    }

    increment(i);

    if (end())
    {
        writer_->close();
    }
}

void ExportFileAction::determineMaximum()
{
    startTimer();

    while (query_.next())
    {
        regionMin_.updateLess(query_.x(), query_.y(), query_.z());
        regionMax_.updateGreater(query_.x(), query_.y(), query_.z());

        nPointsTotal_++;

        if (timedOut())
        {
            return;
        }
    }

    query_.reset();

    properties_.setNumberOfPoints(nPointsTotal_);
    properties_.setRegion(Box<double>(regionMin_, regionMax_));

    writer_->setProperties(properties_);

    ProgressActionInterface::initialize(nPointsTotal_, 1000UL);
}
