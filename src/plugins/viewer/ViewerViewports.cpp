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

/** @file ViewerViewports.cpp */

#include <iostream>

#include <Editor.hpp>

#include <ViewerOpenGLViewport.hpp>
#include <ViewerViewports.hpp>

#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

#define WINDOW_VIEWPORTS_3D 0
#define WINDOW_VIEWPORTS_TOP 1
#define WINDOW_VIEWPORTS_FRONT 2
#define WINDOW_VIEWPORTS_RIGHT 3

ViewerViewports::ViewerViewports(QWidget *parent) : QWidget(parent)
{
    initializeViewer();
}

ViewerViewports::~ViewerViewports()
{
}

void ViewerViewports::initializeViewer()
{
    setLayout(ViewLayout::VIEW_LAYOUT_SINGLE);
}

ViewerOpenGLViewport *ViewerViewports::createViewport(size_t viewportId)
{
    ViewerOpenGLViewport *viewport = new ViewerOpenGLViewport(this);
    viewport->setWindowViewports(this, viewportId);
    viewport->setSelected(false);

    return viewport;
}

ViewerOpenGLViewport *ViewerViewports::selectedViewport()
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->isSelected())
        {
            return viewports_[i];
        }
    }

    return nullptr;
}

const ViewerOpenGLViewport *ViewerViewports::selectedViewport() const
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->isSelected())
        {
            return viewports_[i];
        }
    }

    return nullptr;
}

void ViewerViewports::setViewOrthographic()
{
    selectedViewport()->setViewOrthographic();
}

void ViewerViewports::setViewPerspective()
{
    selectedViewport()->setViewPerspective();
}

void ViewerViewports::setViewTop()
{
    selectedViewport()->setViewTop();
}

void ViewerViewports::setViewFront()
{
    selectedViewport()->setViewFront();
}

void ViewerViewports::setViewRight()
{
    selectedViewport()->setViewRight();
}

void ViewerViewports::setView3d()
{
    selectedViewport()->setView3d();
}

void ViewerViewports::setViewResetDistance()
{
    selectedViewport()->setViewResetDistance();
}

void ViewerViewports::setViewResetCenter()
{
    selectedViewport()->setViewResetCenter();
}

void ViewerViewports::selectViewport(ViewerOpenGLViewport *viewport)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i] == viewport)
        {
            viewports_[i]->setSelected(true);
        }
        else
        {
            viewports_[i]->setSelected(false);
        }
        viewports_[i]->update();
    }
}

size_t ViewerViewports::selectedViewportId() const
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->isSelected())
        {
            return i;
        }
    }
    return 0;
}

void ViewerViewports::updateScene(Editor *editor)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->updateScene(editor);
        viewports_[i]->update();
    }
}

void ViewerViewports::resetScene(Editor *editor, bool resetView)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->resetScene(editor, resetView);
    }
}

void ViewerViewports::resetScene(Editor *editor,
                                 size_t viewportId,
                                 bool resetView)
{
    if (viewportId == WINDOW_VIEWPORTS_TOP)
    {
        viewports_[WINDOW_VIEWPORTS_TOP]->resetScene(editor, resetView);
        viewports_[WINDOW_VIEWPORTS_TOP]->setViewOrthographic();
        viewports_[WINDOW_VIEWPORTS_TOP]->setViewTop();
    }
    else if (viewportId == WINDOW_VIEWPORTS_FRONT)
    {
        viewports_[WINDOW_VIEWPORTS_FRONT]->resetScene(editor, resetView);
        viewports_[WINDOW_VIEWPORTS_FRONT]->setViewOrthographic();
        viewports_[WINDOW_VIEWPORTS_FRONT]->setViewFront();
    }
    else if (viewportId == WINDOW_VIEWPORTS_RIGHT)
    {
        viewports_[WINDOW_VIEWPORTS_RIGHT]->resetScene(editor, resetView);
        viewports_[WINDOW_VIEWPORTS_RIGHT]->setViewOrthographic();
        viewports_[WINDOW_VIEWPORTS_RIGHT]->setViewRight();
    }
}

Camera ViewerViewports::camera(size_t viewportId) const
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->viewportId() == viewportId)
        {
            return viewports_[i]->camera();
        }
    }

    return Camera();
}

void ViewerViewports::setLayout(ViewLayout viewLayout)
{
    // Remove the current layout
    QLayout *oldLayout = layout();
    if (oldLayout)
    {
        Q_ASSERT(oldLayout->count() == 1);
        QLayoutItem *item = oldLayout->itemAt(0);
        ViewerOpenGLViewport *viewport =
            dynamic_cast<ViewerOpenGLViewport *>(item->widget());
        if (viewport)
        {
            oldLayout->removeWidget(viewport);
        }
        else
        {
            QSplitter *splitter = dynamic_cast<QSplitter *>(item->widget());
            if (splitter)
            {
                // Delete extra viewports
                for (size_t i = 1; i < viewports_.size(); i++)
                {
                    viewports_[i]->hide();
                    viewports_[i]->deleteLater();
                }

                // Delete layout content
                viewports_[0]->setParent(this);
                delete splitter;
                viewports_.resize(1);
            }
            else
            {
                Q_UNREACHABLE();
            }
        }

        delete oldLayout;
    }

    // Create the first viewport
    if (viewports_.size() == 0)
    {
        viewports_.resize(1);
        viewports_[0] = createViewport(WINDOW_VIEWPORTS_3D);
        viewports_[0]->setSelected(true);
    }

    // Create new layout
    QHBoxLayout *newLayout = new QHBoxLayout;
    newLayout->setContentsMargins(1, 1, 1, 1);

    // Set layout
    if (viewLayout == ViewLayout::VIEW_LAYOUT_SINGLE)
    {
        newLayout->addWidget(viewports_[0]);
        viewports_[0]->setSelected(true);
    }
    else if (viewLayout == ViewLayout::VIEW_LAYOUT_TWO_COLUMNS)
    {
        // Create viewports
        viewports_.resize(2);
        viewports_[1] = createViewport(WINDOW_VIEWPORTS_TOP);

        // Create new layout
        QSplitter *splitter = new QSplitter;
        splitter->addWidget(viewports_[0]);
        splitter->addWidget(viewports_[1]);
        int w = width() / 2;
        splitter->setSizes(QList<int>({w, w}));

        newLayout->addWidget(splitter);
    }
    else if (viewLayout == ViewLayout::VIEW_LAYOUT_GRID)
    {
        // Create viewports
        viewports_.resize(4);
        viewports_[1] = createViewport(WINDOW_VIEWPORTS_TOP);
        viewports_[2] = createViewport(WINDOW_VIEWPORTS_FRONT);
        viewports_[3] = createViewport(WINDOW_VIEWPORTS_RIGHT);

        // Create new layout
        int w = width() / 2;
        int h = height() / 2;

        QSplitter *splitterLeft = new QSplitter;
        splitterLeft->addWidget(viewports_[WINDOW_VIEWPORTS_TOP]);
        splitterLeft->addWidget(viewports_[WINDOW_VIEWPORTS_FRONT]);
        splitterLeft->setOrientation(Qt::Vertical);
        splitterLeft->setSizes(QList<int>({h, h}));

        QSplitter *splitterRight = new QSplitter;
        splitterRight->addWidget(viewports_[WINDOW_VIEWPORTS_3D]);
        splitterRight->addWidget(viewports_[WINDOW_VIEWPORTS_RIGHT]);
        splitterRight->setOrientation(Qt::Vertical);
        splitterRight->setSizes(QList<int>({h, h}));

        QSplitter *splitter = new QSplitter;
        splitter->addWidget(splitterLeft);
        splitter->addWidget(splitterRight);
        splitter->setSizes(QList<int>({w, w}));

        newLayout->addWidget(splitter);
    }
    else if (viewLayout == ViewLayout::VIEW_LAYOUT_THREE_ROWS_RIGHT)
    {
        // Create viewports
        viewports_.resize(4);
        viewports_[1] = createViewport(WINDOW_VIEWPORTS_TOP);
        viewports_[2] = createViewport(WINDOW_VIEWPORTS_FRONT);
        viewports_[3] = createViewport(WINDOW_VIEWPORTS_RIGHT);

        // Create new layout
        int w = width() / 3;
        int h = height() / 3;

        QSplitter *splitterRight = new QSplitter;
        splitterRight->addWidget(viewports_[1]);
        splitterRight->addWidget(viewports_[2]);
        splitterRight->addWidget(viewports_[3]);
        splitterRight->setOrientation(Qt::Vertical);
        splitterRight->setSizes(QList<int>({h, h, h}));

        QSplitter *splitter = new QSplitter;
        splitter->addWidget(viewports_[0]);
        splitter->addWidget(splitterRight);
        splitter->setSizes(QList<int>({w + w, w}));

        newLayout->addWidget(splitter);
    }
    else
    {
        Q_UNREACHABLE();
    }

    // Set new layout
    QWidget::setLayout(newLayout);
}
