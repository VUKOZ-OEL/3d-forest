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

/** @file QtApplication.cpp */

// Include std.

// Include 3D Forest.
#include <QtApplication.hpp>
#include <QtLabel.hpp>
#include <QtSlider.hpp>
#include <QtVBoxLayout.hpp>
#include <QtViewer.hpp>
#include <QtWidget.hpp>

// Include Qt.
// #include <QSurfaceFormat>
#include <QHBoxLayout>

// Include local.
#define LOG_MODULE_NAME "QtApplication"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

QtApplication::QtApplication(int &argc, char **argv) : qapplication_(argc, argv)
{
}

QtApplication::~QtApplication()
{
}

void QtApplication::init()
{
    // QSurfaceFormat format;
    // format.setDepthBufferSize(24);
    // format.setAlphaBufferSize(8);
    // format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    // QSurfaceFormat::setDefaultFormat(format);

    initLayout();
    load();
}

void QtApplication::setOrganizationName(const std::string &str)
{
}

void QtApplication::setApplicationName(const std::string &str)
{
}

void QtApplication::setApplicationVersion(const std::string &str)
{
}

int QtApplication::exec()
{
    mainWindow_.resize(800, 600);
    mainWindow_.show();
    return qapplication_.exec();
}

void QtApplication::initLayout()
{
    splitter_ = new QSplitter(Qt::Horizontal, &mainWindow_);

    // Left plugin panel area
    panelScrollArea_ = new QScrollArea(splitter_);
    panelScrollArea_->setWidgetResizable(true);
    panelScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    panelContainer_ = new QWidget(panelScrollArea_);

    panelLayout_ = new QVBoxLayout(panelContainer_);
    panelLayout_->setContentsMargins(4, 4, 4, 4);
    panelLayout_->setSpacing(4);
    panelLayout_->addStretch();

    panelScrollArea_->setWidget(panelContainer_);

    // Right viewer area, initially empty
    viewerContainer_ = new QWidget(splitter_);

    viewerLayout_ = new QVBoxLayout(viewerContainer_);
    viewerLayout_->setContentsMargins(0, 0, 0, 0);
    viewerLayout_->setSpacing(0);

    /*QLabel *placeholder = new QLabel(
        "No viewer loaded",
        viewerContainer_);

    placeholder->setAlignment(Qt::AlignCenter);

    viewerLayout_->addWidget(placeholder);
    viewerWidget_ = placeholder;*/

    splitter_->addWidget(panelScrollArea_);
    splitter_->addWidget(viewerContainer_);

    // Left side does not stretch as much as the viewer.
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    // Initial widths.
    splitter_->setSizes({300, 900});

    panelScrollArea_->setMinimumWidth(200);
    panelScrollArea_->setMaximumWidth(600);

    mainWindow_.setCentralWidget(splitter_);
}

void QtApplication::addPanel(const std::string &title, Widget *widget)
{
    LOG_DEBUG(<< "Start adding panel <" << title << ">.");

    QWidget *qtContent = createWidget(widget);

    if (!qtContent)
    {
        LOG_WARNING(<< "Could not create Qt widget.");
        return;
    }

    LOG_DEBUG(<< "Adding widget.");

    QtExpandableWidget *expandable =
        new QtExpandableWidget(title, qtContent, panelContainer_);

    // Insert before the stretch at the end.
    panelLayout_->insertWidget(panelLayout_->count() - 1, expandable);

    panelBindings_.push_back({widget, expandable});

    LOG_DEBUG(<< "Finished adding panel.");
}

void QtApplication::removePanel(Widget *widget)
{
    auto it = std::find_if(panelBindings_.begin(),
                           panelBindings_.end(),
                           [widget](const PanelBinding &binding)
                           { return binding.commonWidget == widget; });

    if (it == panelBindings_.end())
    {
        return;
    }

    panelLayout_->removeWidget(it->expandableWidget);

    // Also deletes the contained QtWidget and QtSlider objects.
    delete it->expandableWidget;

    panelBindings_.erase(it);
}

void QtApplication::setViewer(Widget *widget)
{
    if (commonViewer_ == widget)
    {
        return;
    }

    if (commonViewer_)
    {
        removeViewer(commonViewer_);
    }

    if (!widget)
    {
        return;
    }

    QWidget *qtWidget = createWidget(widget, viewerContainer_);

    if (!qtWidget)
    {
        return;
    }

    commonViewer_ = widget;
    qtViewer_ = qtWidget;

    viewerLayout_->addWidget(qtViewer_);
}

void QtApplication::removeViewer(Widget *widget)
{
    if (commonViewer_ != widget)
    {
        return;
    }

    if (qtViewer_)
    {
        viewerLayout_->removeWidget(qtViewer_);

        // Deletes QtViewer before the generic Viewer.
        delete qtViewer_;
    }

    qtViewer_ = nullptr;
    commonViewer_ = nullptr;
}

QWidget *QtApplication::createWidget(Widget *widget, QWidget *parent)
{
    if (!widget)
    {
        LOG_WARNING(<< "Could not create Qt widget from null.");
        return nullptr;
    }

    if (auto *w = dynamic_cast<Label *>(widget))
    {
        LOG_DEBUG(<< "Create label widget.");
        return new QtLabel(w, parent);
    }

    if (auto *w = dynamic_cast<Slider *>(widget))
    {
        LOG_DEBUG(<< "Create slider widget.");
        return new QtSlider(w, parent);
    }

    if (auto *w = dynamic_cast<Viewer *>(widget))
    {
        LOG_DEBUG(<< "Create viewer widget.");
        return new QtViewer(w, parent);
    }

    LOG_DEBUG(<< "Create widget.");
    return new QtWidget(widget, this, parent);
}

QLayout *QtApplication::createLayout(Layout *layout, QWidget *parent)
{
    if (auto *vbox = dynamic_cast<VBoxLayout *>(layout))
    {
        LOG_DEBUG(<< "Create VBoxLayout.");
        return new QtVBoxLayout(vbox, this, parent);
    }

    LOG_DEBUG(<< "Create null layout.");
    return nullptr;
}
