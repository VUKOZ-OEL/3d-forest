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
#include <Log.hpp>

ExportFileAction::ExportFileAction(Editor *editor)
    : ProgressActionInterface(),
      editor_(editor),
      query_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ExportFileAction::~ExportFileAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ExportFileAction::initialize(
    std::shared_ptr<ExportFileFormatInterface> writer,
    const ExportFileProperties &properties)
{
    LOG_DEBUG(<< "Initialize.");

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

    progress_.setMaximumStep(ProgressCounter::npos, 1000UL);
}

void ExportFileAction::clear()
{
    LOG_DEBUG(<< "Clear.");
    query_.clear();
}

void ExportFileAction::next()
{
    if (progress_.initializing())
    {
        determineMaximum();
        return;
    }

    progress_.startTimer();

    if (!writer_->isOpen())
    {
        writer_->create(properties_.fileName());
    }

    while (query_.next())
    {
        writer_->write(query_);

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    writer_->close();

    progress_.setValueStep(progress_.maximumStep());
}

void ExportFileAction::determineMaximum()
{
    progress_.startTimer();

    while (query_.next())
    {
        regionMin_.updateLess(query_.x(), query_.y(), query_.z());
        regionMax_.updateGreater(query_.x(), query_.y(), query_.z());

        nPointsTotal_++;

        if (progress_.timedOut())
        {
            return;
        }
    }

    query_.reset();

    properties_.setNumberOfPoints(nPointsTotal_);
    properties_.setRegion(Box<double>(regionMin_, regionMax_));

    writer_->setProperties(properties_);

    progress_.setMaximumStep(nPointsTotal_, 1000UL);
}
