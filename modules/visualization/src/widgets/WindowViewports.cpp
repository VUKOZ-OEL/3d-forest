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

/**
    @file WindowViewports.cpp
*/

#include <Editor.hpp>
#include <GLWidget.hpp>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>
#include <WindowViewports.hpp>
#include <iostream>

#define WINDOW_VIEWPORTS_3D 0
#define WINDOW_VIEWPORTS_TOP 1
#define WINDOW_VIEWPORTS_FRONT 2
#define WINDOW_VIEWPORTS_RIGHT 3

WindowViewports::WindowViewports(QWidget *parent) : QWidget(parent)
{
    initializeViewer();
}

WindowViewports::~WindowViewports()
{
}

void WindowViewports::initializeViewer()
{
    setLayout(ViewLayout::VIEW_LAYOUT_SINGLE);
}

GLWidget *WindowViewports::createViewport(size_t viewportId)
{
    GLWidget *viewport = new GLWidget(this);
    viewport->setWindowViewports(this, viewportId);
    viewport->setSelected(false);

    return viewport;
}

GLWidget *WindowViewports::selectedViewport()
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

const GLWidget *WindowViewports::selectedViewport() const
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

void WindowViewports::setViewOrthographic()
{
    selectedViewport()->setViewOrthographic();
}

void WindowViewports::setViewPerspective()
{
    selectedViewport()->setViewPerspective();
}

void WindowViewports::setViewTop()
{
    selectedViewport()->setViewTop();
}

void WindowViewports::setViewFront()
{
    selectedViewport()->setViewFront();
}

void WindowViewports::setViewRight()
{
    selectedViewport()->setViewRight();
}

void WindowViewports::setView3d()
{
    selectedViewport()->setView3d();
}

void WindowViewports::setViewResetDistance()
{
    selectedViewport()->setViewResetDistance();
}

void WindowViewports::setViewResetCenter()
{
    selectedViewport()->setViewResetCenter();
}

void WindowViewports::selectViewport(GLWidget *viewport)
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

size_t WindowViewports::selectedViewportId() const
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

void WindowViewports::updateScene(Editor *editor)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->updateScene(editor);
        viewports_[i]->update();
    }
}

void WindowViewports::resetScene(Editor *editor)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->resetScene(editor);
    }
}

void WindowViewports::resetScene(Editor *editor, size_t viewportId)
{
    if (viewportId == WINDOW_VIEWPORTS_TOP)
    {
        viewports_[WINDOW_VIEWPORTS_TOP]->resetScene(editor);
        viewports_[WINDOW_VIEWPORTS_TOP]->setViewOrthographic();
        viewports_[WINDOW_VIEWPORTS_TOP]->setViewTop();
    }
    else if (viewportId == WINDOW_VIEWPORTS_FRONT)
    {
        viewports_[WINDOW_VIEWPORTS_FRONT]->resetScene(editor);
        viewports_[WINDOW_VIEWPORTS_FRONT]->setViewOrthographic();
        viewports_[WINDOW_VIEWPORTS_FRONT]->setViewFront();
    }
    else if (viewportId == WINDOW_VIEWPORTS_RIGHT)
    {
        viewports_[WINDOW_VIEWPORTS_RIGHT]->resetScene(editor);
        viewports_[WINDOW_VIEWPORTS_RIGHT]->setViewOrthographic();
        viewports_[WINDOW_VIEWPORTS_RIGHT]->setViewRight();
    }
}

Camera WindowViewports::camera(size_t viewportId) const
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

void WindowViewports::setLayout(ViewLayout viewLayout)
{
    // Remove the current layout
    QLayout *oldLayout = layout();
    if (oldLayout)
    {
        Q_ASSERT(oldLayout->count() == 1);
        QLayoutItem *item = oldLayout->itemAt(0);
        GLWidget *viewport = dynamic_cast<GLWidget *>(item->widget());
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
