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
    @file GLViewer.cpp
*/

#include <GLViewer.hpp>
#include <GLWidget.hpp>
#include <QDebug>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

GLViewer::GLViewer(QWidget *parent) : QWidget(parent)
{
    initializeViewer();
}

GLViewer::~GLViewer()
{
}

void GLViewer::initializeViewer()
{
    setViewLayout(ViewLayout::VIEW_LAYOUT_SINGLE);
}

GLWidget *GLViewer::createViewport()
{
    GLWidget *viewport = new GLWidget(this);
    viewport->setViewer(this);
    viewport->setSelected(false);

    return viewport;
}

void GLViewer::setViewLayout(ViewLayout viewLayout)
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
                // Reparent viewports from splitter and find selected viewport
                size_t selectedViewport = 0;
                for (size_t i = 0; i < viewports_.size(); i++)
                {
                    if (viewports_[i]->isSelected())
                    {
                        selectedViewport = i;
                    }
                    viewports_[i]->setParent(this);
                }

                // Delete layout content
                delete splitter;

                // Delete extra viewports
                GLWidget *tmpViewport = viewports_[0];
                viewports_[0] = viewports_[selectedViewport];
                viewports_[selectedViewport] = tmpViewport;
                for (size_t i = 1; i < viewports_.size(); i++)
                {
                    delete viewports_[i];
                }
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
        viewports_[0] = createViewport();
        viewports_[0]->setSelected(true);
    }

    // Set layout
    if (viewLayout == ViewLayout::VIEW_LAYOUT_SINGLE)
    {
        // Create new layout
        QHBoxLayout *newLayout = new QHBoxLayout;
        newLayout->setContentsMargins(1, 1, 1, 1);
        newLayout->addWidget(viewports_[0]);
        setLayout(newLayout);
    }
    else if (viewLayout == ViewLayout::VIEW_LAYOUT_TWO_COLUMNS)
    {
        // Create the second viewport
        viewports_.resize(2);
        viewports_[1] = createViewport();

        // Create new layout
        QSplitter *splitter = new QSplitter;
        splitter->addWidget(viewports_[0]);
        splitter->addWidget(viewports_[1]);
        int w = width() / 2;
        splitter->setSizes(QList<int>({w, w}));

        QHBoxLayout *newLayout = new QHBoxLayout;
        newLayout->setContentsMargins(1, 1, 1, 1);
        newLayout->addWidget(splitter);
        setLayout(newLayout);
    }
    else
    {
        Q_UNREACHABLE();
    }
}

void GLViewer::selectViewport(GLWidget *viewport)
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

void GLViewer::updateScene(const Scene &scene)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->updateScene(scene);
        viewports_[i]->update();
    }
}

void GLViewer::updateScene(Editor *editor)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->updateScene(editor);
        viewports_[i]->update();
    }
}
