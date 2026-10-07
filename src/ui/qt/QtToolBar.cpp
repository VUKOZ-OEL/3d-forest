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

/** @file QtToolBar.cpp */

// Include std.
#include <stdexcept>

// Include 3D Forest.
#include <QtApplication.hpp>
#include <QtToolBar.hpp>

// Include Qt.
#include <QAction>
#include <QPointer>
#include <QSize>
#include <QToolButton>

// Include local.
#define LOG_MODULE_NAME "QtToolBar"
#include <Log.hpp>

QtToolBar::QtToolBar(ToolBar *toolBar, QtApplication *app, QWidget *parent)
    : QToolBar(parent),
      toolBar_(toolBar),
      app_(app)
{
    if (!toolBar_ || !app_)
    {
        throw std::invalid_argument(
            "QtToolBar requires a ToolBar and QtApplication.");
    }

    setMovable(false);
    setFloatable(false);

    // Explicitly apply icon sizes to buttons inserted using addWidget().
    QObject::connect(this,
                     &QToolBar::iconSizeChanged,
                     this,
                     [this](const QSize &) { updateButtonIconSizes(); });

    updateSettings();

    for (const ToolBar::Item &item : toolBar_->items())
    {
        appendItem(item);
    }

    const QPointer<QtToolBar> guard(this);

    toolBar_->itemAdded.connect(
        [guard](ToolBar::Item item)
        {
            if (guard)
            {
                guard->appendItem(item);
            }
        });

    toolBar_->clearing.connect(
        [guard]
        {
            if (guard)
            {
                guard->clearItems();
            }
        });

    toolBar_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    toolBar_->destroying.connect(
        [guard]
        {
            if (guard)
            {
                delete guard.data();
            }
        });
}

QtToolBar::~QtToolBar()
{
}

void QtToolBar::appendItem(const ToolBar::Item &item)
{
    if (item.isSeparator())
    {
        QToolBar::addSeparator();
        return;
    }

    QWidget *widget = app_->createWidget(item.widget, this);

    if (!widget)
    {
        throw std::runtime_error(
            "QtApplication::createWidget returned nullptr.");
    }

    QToolBar::addWidget(widget);

    if (auto *button = qobject_cast<QToolButton *>(widget))
    {
        button->setIconSize(QToolBar::iconSize());
    }
}

void QtToolBar::clearItems()
{
    // Take a snapshot because removing actions changes actions().
    const auto toolbarActions = actions();

    for (QAction *action : toolbarActions)
    {
        QToolBar::removeAction(action);

        // addWidget() creates a QWidgetAction which owns its widget.
        // Deleting the action also deletes that native widget.
        delete action;
    }
}

void QtToolBar::updateSettings()
{
    QToolBar::setOrientation(toolBar_->orientation() == Ui::Vertical
                                 ? Qt::Vertical
                                 : Qt::Horizontal);

    if (orientation() == Qt::Vertical)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    else
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    const Size &size = toolBar_->iconSize();

    QToolBar::setIconSize(QSize(size.width(), size.height()));

    // Also applies when the toolbar size itself did not change.
    updateButtonIconSizes();

    // After applying orientation and icon size:
    if (QLayout *toolbarLayout = QToolBar::layout())
    {
    const int margin = orientation() == Qt::Horizontal ? 2 : 0;
    toolbarLayout->setContentsMargins(margin, margin, margin, margin);
    }

    updateGeometry();
}

void QtToolBar::updateButtonIconSizes()
{
    const QSize size = QToolBar::iconSize();

    for (QAction *action : actions())
    {
        QWidget *widget = widgetForAction(action);

        if (auto *button = qobject_cast<QToolButton *>(widget))
        {
            button->setIconSize(size);
        }
    }
}
