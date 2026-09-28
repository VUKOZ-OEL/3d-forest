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

/** @file QtSplitter.cpp */

// Include 3D Forest.
#include <QtApplication.hpp>
#include <QtSplitter.hpp>

// Include Qt.
#include <QList>
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtSplitter"
#include <Log.hpp>

QtSplitter::QtSplitter(Splitter *splitter, QtApplication *app, QWidget *parent)
    : QSplitter(parent),
      splitter_(splitter),
      app_(app)
{
    updateSettings();

    for (Widget *widget : splitter_->widgets())
    {
        addCommonWidget(widget);
    }

    applySizes(splitter_->requestedSizes());

    const QPointer<QtSplitter> guard(this);

    splitter_->widgetAdded.connect(
        [guard](Widget *widget)
        {
            if (guard)
            {
                guard->addCommonWidget(widget);
                guard->applySizes(guard->splitter_->requestedSizes());
            }
        });

    splitter_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    splitter_->sizesUpdated.connect(
        [guard](const std::vector<int> &sizes)
        {
            if (guard)
            {
                guard->applySizes(sizes);
            }
        });

    QObject::connect(this,
                     &QSplitter::splitterMoved,
                     this,
                     [this](int position, int index)
                     {
                         if (!splitter_->signalsBlocked())
                         {
                             splitter_->splitterMoved(position, index);
                         }
                     });
}

QtSplitter::~QtSplitter() = default;

void QtSplitter::addCommonWidget(Widget *widget)
{
    QWidget *qtWidget = app_->createWidget(widget, this);

    QSplitter::addWidget(qtWidget);
}

void QtSplitter::updateSettings()
{
    const QSignalBlocker blocker(this);

    QSplitter::setOrientation(splitter_->orientation() == Ui::Vertical
                                  ? Qt::Vertical
                                  : Qt::Horizontal);

    QSplitter::setChildrenCollapsible(splitter_->childrenCollapsible());
}

void QtSplitter::applySizes(const std::vector<int> &sizes)
{
    if (sizes.empty() || static_cast<int>(sizes.size()) != count())
    {
        return;
    }

    QList<int> qtSizes;

    for (int size : sizes)
    {
        qtSizes.append(size);
    }

    const QSignalBlocker blocker(this);
    QSplitter::setSizes(qtSizes);
}
