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

// Include std.
#include <iostream>

// Include 3D Forest.
#include <Editor.hpp>
#include <ViewerOpenGLViewport.hpp>
#include <ViewerViewports.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ViewerViewports"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define VIEWER_VIEWPORTS_3D 0
#define VIEWER_VIEWPORTS_TOP 1
#define VIEWER_VIEWPORTS_FRONT 2
#define VIEWER_VIEWPORTS_RIGHT 3

ViewerViewports::ViewerViewports(QWidget *parent) : QWidget(parent)
{
    LOG_DEBUG(<< "The viewports are being created.");
    initializeViewports();
    LOG_DEBUG(<< "Finished creating the viewports.");
}

ViewerViewports::~ViewerViewports()
{
    LOG_DEBUG(<< "The viewports are being destroyed.");
}

void ViewerViewports::paintEvent(QPaintEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Paint event.");
    QWidget::paintEvent(event);
}

void ViewerViewports::resizeEvent(QResizeEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Resize event.");
    QWidget::resizeEvent(event);
}

void ViewerViewports::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    QWidget::showEvent(event);
}

void ViewerViewports::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide.");
    QWidget::hideEvent(event);
}

void ViewerViewports::initializeViewports()
{
    LOG_DEBUG(<< "Initialize viewports.");
    setLayout(ViewLayout::VIEW_LAYOUT_SINGLE);
}

ViewerOpenGLViewport *ViewerViewports::createViewport(size_t viewportId)
{
    LOG_DEBUG(<< "Create viewport <" << viewportId << ">.");
    ViewerOpenGLViewport *viewport = new ViewerOpenGLViewport(this);
    viewport->setViewports(this, viewportId);
    viewport->setSelected(false);

    return viewport;
}

ViewerOpenGLViewport *ViewerViewports::selectedViewport()
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->selected())
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
        if (viewports_[i]->selected())
        {
            return viewports_[i];
        }
    }

    return nullptr;
}

void ViewerViewports::setViewOrthographic()
{
    LOG_DEBUG(<< "Set orthographic view to the active viewport.");
    selectedViewport()->setViewOrthographic();
}

void ViewerViewports::setViewPerspective()
{
    LOG_DEBUG(<< "Set perspective view to the active viewport.");
    selectedViewport()->setViewPerspective();
}

void ViewerViewports::setViewTop()
{
    LOG_DEBUG(<< "Set top view to the active viewport.");
    selectedViewport()->setViewTop();
}

void ViewerViewports::setViewFront()
{
    LOG_DEBUG(<< "Set front view to the active viewport.");
    selectedViewport()->setViewFront();
}

void ViewerViewports::setViewRight()
{
    LOG_DEBUG(<< "Set right view to the active viewport.");
    selectedViewport()->setViewRight();
}

void ViewerViewports::setView3d()
{
    LOG_DEBUG(<< "Set 3d view to the active viewport.");
    selectedViewport()->setView3d();
}

void ViewerViewports::setViewResetDistance()
{
    LOG_DEBUG(<< "Reset distance in the active viewport.");
    selectedViewport()->setViewResetDistance();
}

void ViewerViewports::setViewResetCenter()
{
    LOG_DEBUG(<< "Reset center in the active viewport.");
    selectedViewport()->setViewResetCenter();
}

void ViewerViewports::selectViewport(ViewerOpenGLViewport *viewport)
{
    LOG_DEBUG(<< "Start selecting viewport <" << viewport->viewportId() << ".");
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
    LOG_DEBUG(<< "Finished selecting viewport.");
}

size_t ViewerViewports::selectedViewportId() const
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->selected())
        {
            return i;
        }
    }
    return 0;
}

void ViewerViewports::updateScene(Editor *editor)
{
    LOG_DEBUG_RENDER(<< "Start updating all viewports.");
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->updateScene(editor);
        viewports_[i]->update();
    }
    LOG_DEBUG_RENDER(<< "Finished updating all viewports.");
}

void ViewerViewports::resetScene(Editor *editor, bool resetView)
{
    LOG_DEBUG_RENDER(<< "Start reseting all viewports, reset view <"
                     << static_cast<int>(resetView) << ">.");

    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->resetScene(editor, resetView);
    }

    LOG_DEBUG_RENDER(<< "Finished reseting all viewports.");
}

void ViewerViewports::resetViewport(Editor *editor,
                                    size_t viewportId,
                                    bool resetView)
{
    LOG_DEBUG_RENDER(<< "Start reseting viewport <" << viewportId
                     << "> reset view <" << static_cast<int>(resetView)
                     << ">.");

    if (viewportId == VIEWER_VIEWPORTS_3D)
    {
        viewports_[VIEWER_VIEWPORTS_3D]->resetScene(editor, resetView);
        viewports_[VIEWER_VIEWPORTS_3D]->setViewOrthographic();
        viewports_[VIEWER_VIEWPORTS_3D]->setView3d();
    }
    else if (viewportId == VIEWER_VIEWPORTS_TOP)
    {
        viewports_[VIEWER_VIEWPORTS_TOP]->resetScene(editor, resetView);
        viewports_[VIEWER_VIEWPORTS_TOP]->setViewOrthographic();
        viewports_[VIEWER_VIEWPORTS_TOP]->setViewTop();
    }
    else if (viewportId == VIEWER_VIEWPORTS_FRONT)
    {
        viewports_[VIEWER_VIEWPORTS_FRONT]->resetScene(editor, resetView);
        viewports_[VIEWER_VIEWPORTS_FRONT]->setViewOrthographic();
        viewports_[VIEWER_VIEWPORTS_FRONT]->setViewFront();
    }
    else if (viewportId == VIEWER_VIEWPORTS_RIGHT)
    {
        viewports_[VIEWER_VIEWPORTS_RIGHT]->resetScene(editor, resetView);
        viewports_[VIEWER_VIEWPORTS_RIGHT]->setViewOrthographic();
        viewports_[VIEWER_VIEWPORTS_RIGHT]->setViewRight();
    }

    LOG_DEBUG_RENDER(<< "Finished reseting viewport.");
}

std::vector<Camera> ViewerViewports::camera(size_t viewportId) const
{
    std::vector<Camera> cameraList;

    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i]->viewportId() == viewportId)
        {
            Camera c = viewports_[i]->camera();
            LOG_DEBUG_RENDER(<< "Append camera <" << c << "> by viewportId <"
                             << viewportId << "> from viewport index <" << i
                             << ">.");
            cameraList.push_back(std::move(c));
            break;
        }
    }

    if (cameraList.empty())
    {
        Camera c;
        LOG_DEBUG_RENDER(<< "Append default camera <" << c
                         << "> by viewportId <" << viewportId << ">.");
        cameraList.push_back(std::move(c));
    }

    return cameraList;
}

std::vector<Camera> ViewerViewports::camera() const
{
    std::vector<Camera> cameraList;

    for (size_t i = 0; i < viewports_.size(); i++)
    {
        Camera c = viewports_[i]->camera();
        LOG_DEBUG_RENDER(<< "Append camera <" << c << "> from viewport index <"
                         << i << ">.");
        cameraList.push_back(std::move(c));
    }

    if (cameraList.empty())
    {
        Camera c;
        LOG_DEBUG_RENDER(<< "Append default camera <" << c << ">.");
        cameraList.push_back(std::move(c));
    }

    return cameraList;
}

void ViewerViewports::setLayout(ViewLayout viewLayout)
{
    LOG_DEBUG_RENDER(<< "Start setting layout <" << viewLayout << ">.");

    // Remove the current layout.
    QLayout *oldLayout = layout();
    if (oldLayout)
    {
        LOG_DEBUG_RENDER(<< "Remove old layout.");

        Q_ASSERT(oldLayout->count() == 1);
        QLayoutItem *item = oldLayout->itemAt(0);
        ViewerOpenGLViewport *viewport =
            dynamic_cast<ViewerOpenGLViewport *>(item->widget());

        if (viewport)
        {
            LOG_DEBUG_RENDER(<< "Remove widget from old layout.");
            oldLayout->removeWidget(viewport);
        }
        else
        {
            QSplitter *splitter = dynamic_cast<QSplitter *>(item->widget());

            if (splitter)
            {
                LOG_DEBUG_RENDER(<< "Remove n viewports from old layout.");

                // Delete extra viewports.
                for (size_t i = 1; i < viewports_.size(); i++)
                {
                    LOG_DEBUG_RENDER(<< "Remove viewport <" << i << ">.");
                    viewports_[i]->hide();
                    viewports_[i]->deleteLater();
                }

                // Delete layout content.
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

    // Create the first viewport.
    if (viewports_.size() == 0)
    {
        LOG_DEBUG_RENDER(<< "Create the first viewport.");
        viewports_.resize(1);
        viewports_[0] = createViewport(VIEWER_VIEWPORTS_3D);
        viewports_[0]->setSelected(true);
    }

    // Create new layout.
    QHBoxLayout *newLayout = new QHBoxLayout;
    newLayout->setContentsMargins(1, 1, 1, 1);

    // Set layout.
    if (viewLayout == ViewLayout::VIEW_LAYOUT_SINGLE)
    {
        newLayout->addWidget(viewports_[0]);
        viewports_[0]->setSelected(true);
    }
    else if (viewLayout == ViewLayout::VIEW_LAYOUT_TWO_COLUMNS)
    {
        // Create viewports.
        viewports_.resize(2);
        viewports_[1] = createViewport(VIEWER_VIEWPORTS_TOP);

        // Create new layout.
        QSplitter *splitter = new QSplitter;
        splitter->addWidget(viewports_[0]);
        splitter->addWidget(viewports_[1]);
        int w = width() / 2;
        splitter->setSizes(QList<int>({w, w}));

        newLayout->addWidget(splitter);
    }
    else if (viewLayout == ViewLayout::VIEW_LAYOUT_GRID)
    {
        // Create viewports.
        viewports_.resize(4);
        viewports_[1] = createViewport(VIEWER_VIEWPORTS_TOP);
        viewports_[2] = createViewport(VIEWER_VIEWPORTS_FRONT);
        viewports_[3] = createViewport(VIEWER_VIEWPORTS_RIGHT);

        // Create new layout.
        int w = width() / 2;
        int h = height() / 2;

        QSplitter *splitterLeft = new QSplitter;
        splitterLeft->addWidget(viewports_[VIEWER_VIEWPORTS_TOP]);
        splitterLeft->addWidget(viewports_[VIEWER_VIEWPORTS_FRONT]);
        splitterLeft->setOrientation(Qt::Vertical);
        splitterLeft->setSizes(QList<int>({h, h}));

        QSplitter *splitterRight = new QSplitter;
        splitterRight->addWidget(viewports_[VIEWER_VIEWPORTS_3D]);
        splitterRight->addWidget(viewports_[VIEWER_VIEWPORTS_RIGHT]);
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
        // Create viewports.
        viewports_.resize(4);
        viewports_[1] = createViewport(VIEWER_VIEWPORTS_TOP);
        viewports_[2] = createViewport(VIEWER_VIEWPORTS_FRONT);
        viewports_[3] = createViewport(VIEWER_VIEWPORTS_RIGHT);

        // Create new layout.
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

    // Set new layout.
    QWidget::setLayout(newLayout);

    LOG_DEBUG_RENDER(<< "Finished setting layout.");
}
