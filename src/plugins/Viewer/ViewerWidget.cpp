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

/** @file ViewerWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <HBoxLayout.hpp>
#include <ThemeIcon.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <VBoxLayout.hpp>
#include <Viewer.hpp>
#include <ViewerWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewerWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#include <ViewerResources.hpp> // Generated
#define ICON(name) (ThemeIcon(app_, ":/ViewerResources/", name))

ViewerWidget::ViewerWidget(Application *app) : app_(app)
{
    // Viewer.
    viewer_ = new Viewer;

    // Tool bar.
    app_->createToolButton(&viewOrthographicAction_,
                           tr("Orthographic"),
                           tr("Orthographic projection"),
                           ICON("orthographic-wire"),
                           [this]() { slotViewOrthographic(); });

    app_->createToolButton(&viewPerspectiveAction_,
                           tr("Perspective"),
                           tr("Perspective projection"),
                           ICON("perspective-wire"),
                           [this]() { slotViewPerspective(); });

    app_->createToolButton(&view2dAction_,
                           tr("2D DBH"),
                           tr("2D projection with DBH"),
                           ICON("view-2d"),
                           [this]() { slotView2d(); });

    app_->createToolButton(&view3dAction_,
                           tr("3d view"),
                           tr("3d view"),
                           ICON("portraits-fill"),
                           [this]() { slotView3d(); });

    app_->createToolButton(&viewTopAction_,
                           tr("Top view"),
                           tr("Top view"),
                           ICON("view-top"),
                           [this]() { slotViewTop(); });

    app_->createToolButton(&viewFrontAction_,
                           tr("Front view"),
                           tr("Front view"),
                           ICON("view-front"),
                           [this]() { slotViewFront(); });

    app_->createToolButton(&viewRightAction_,
                           tr("Right view"),
                           tr("Right view"),
                           ICON("view-right"),
                           [this]() { slotViewRight(); });

    app_->createToolButton(&viewResetDistanceAction_,
                           tr("Reset distance"),
                           tr("Reset distance"),
                           ICON("fit-to-page"),
                           [this]() { slotViewResetDistance(); });

    app_->createToolButton(&viewResetCenterAction_,
                           tr("Reset center"),
                           tr("Reset center"),
                           ICON("collect"),
                           [this]() { slotViewResetCenter(); });

    ToolBar *toolBar = new ToolBar;
    // toolBar->setOrientation(Ui::Vertical);
    toolBar->addWidget(viewOrthographicAction_);
    toolBar->addWidget(viewPerspectiveAction_);
    toolBar->addWidget(view2dAction_);
    toolBar->addSeparator();
    toolBar->addWidget(viewTopAction_);
    toolBar->addWidget(viewFrontAction_);
    toolBar->addWidget(viewRightAction_);
    toolBar->addWidget(view3dAction_);
    toolBar->addSeparator();
    toolBar->addWidget(viewResetDistanceAction_);
    toolBar->addWidget(viewResetCenterAction_);

    int size = Application::ICON_SIZE;
    toolBar->setIconSize(Size(size, size));

    // Layout.
#if 0
    VBoxLayout *mainLayout = new VBoxLayout;
    toolBar->setOrientation(Ui::Horizontal);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolBar, 0);
    mainLayout->addWidget(viewer_, 1);
#else
    HBoxLayout *mainLayout = new HBoxLayout;
    toolBar->setOrientation(Ui::Vertical);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolBar, 0);
    mainLayout->addWidget(viewer_, 1);
#endif

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect([this](const Message &msg) { slotUpdate(msg); });

    slotUpdate({});
}

void ViewerWidget::slotUpdate(const Message &msg)
{
    if (msg.sender() == this)
    {
        return;
    }
}

std::vector<Camera> ViewerWidget::camera(size_t viewportId) const
{
    std::vector<Camera> list;
    list.push_back(viewer_->camera());
    return list;
}

std::vector<Camera> ViewerWidget::camera() const
{
    std::vector<Camera> list;
    list.push_back(viewer_->camera());
    return list;
}

void ViewerWidget::updateScene()
{
    viewer_->requestUpdate();
}

void ViewerWidget::resetScene()
{
    viewer_->requestReset();
}

void ViewerWidget::resetSceneView()
{
    viewer_->requestResetView();
}

void ViewerWidget::slotViewOrthographic()
{
    viewer_->command(Viewer::ViewOrthographic);
    updateViewer();
}

void ViewerWidget::slotViewPerspective()
{
    viewer_->command(Viewer::ViewPerspective);
    updateViewer();
}

void ViewerWidget::slotView2d()
{
    viewer_->command(Viewer::View2d);
    updateViewer();
}

void ViewerWidget::slotViewTop()
{
    viewer_->command(Viewer::ViewTop);
    updateViewer();
}

void ViewerWidget::slotViewFront()
{
    viewer_->command(Viewer::ViewFront);
    updateViewer();
}

void ViewerWidget::slotViewRight()
{
    viewer_->command(Viewer::ViewRight);
    updateViewer();
}

void ViewerWidget::slotView3d()
{
    viewer_->command(Viewer::View3d);
    updateViewer();
}

void ViewerWidget::slotViewResetDistance()
{
    viewer_->command(Viewer::ViewResetDistance);
    updateViewer();
}

void ViewerWidget::slotViewResetCenter()
{
    viewer_->command(Viewer::ViewResetCenter);
    updateViewer();
}

void ViewerWidget::updateViewer()
{
    //updateScene();
    app_->slotRenderViewports();
}
