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
#include <QtCheckBox.hpp>
#include <QtComboBox.hpp>
#include <QtGridLayout.hpp>
#include <QtGroupBox.hpp>
#include <QtLabel.hpp>
#include <QtSlider.hpp>
#include <QtVBoxLayout.hpp>
#include <QtViewer.hpp>
#include <QtWidget.hpp>

// Include Qt.
// #include <QSurfaceFormat>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPalette>
#include <QStyleHints>

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

    connect(
        this,
        &QtApplication::wakeUpRequested,
        this,
        [this] { processRenderRequest(); },
        Qt::QueuedConnection);
}

void QtApplication::wakeUp()
{
    emit wakeUpRequested();
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
    splitter_->setHandleWidth(1);
    splitter_->setStyleSheet("QSplitter::handle {"
                             "    background: #303030;"
                             "}");

    // Left side bar area
    sidebar_ = new QtSidebar(&navigation(), this, splitter_);

    // Right viewer area, initially empty
    viewerContainer_ = new QWidget(splitter_);

    viewerLayout_ = new QVBoxLayout(viewerContainer_);
    viewerLayout_->setContentsMargins(0, 0, 0, 0);
    viewerLayout_->setSpacing(0);

    // Splitter.
    splitter_->addWidget(sidebar_);
    splitter_->addWidget(viewerContainer_);

    // Left side does not stretch as much as the viewer.
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    // Initial widths.
    splitter_->setSizes({300, 900});

    sidebar_->setMinimumWidth(220);
    sidebar_->setMaximumWidth(500);

    mainWindow_.setCentralWidget(splitter_);

    // Theme.
    updateTheme();

    QObject::connect(qapplication_.styleHints(),
                     &QStyleHints::colorSchemeChanged,
                     &mainWindow_,
                     [this](Qt::ColorScheme) { updateTheme(); });
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

        // Deletes Viewer before the generic Viewer.
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

    if (auto *w = dynamic_cast<CheckBox *>(widget))
    {
        LOG_DEBUG(<< "Create checkbox widget.");
        return new QtCheckBox(w, parent);
    }

    if (auto *w = dynamic_cast<ComboBox *>(widget))
    {
        LOG_DEBUG(<< "Create combobox widget.");
        return new QtComboBox(w, parent);
    }

    if (auto *w = dynamic_cast<GroupBox *>(widget))
    {
        LOG_DEBUG(<< "Create groupbox widget.");
        return new QtGroupBox(w, this, parent);
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
        return new QtViewer(w, this, parent);
    }

    LOG_DEBUG(<< "Create default widget.");
    return new QtWidget(widget, this, parent);
}

QLayout *QtApplication::createLayout(Layout *layout, QWidget *parent)
{
    if (auto *gridLayout = dynamic_cast<GridLayout *>(layout))
    {
        LOG_DEBUG(<< "Create GridLayout.");
        return new QtGridLayout(gridLayout, this, parent);
    }

    if (auto *vBoxLayout = dynamic_cast<VBoxLayout *>(layout))
    {
        LOG_DEBUG(<< "Create VBoxLayout.");
        return new QtVBoxLayout(vBoxLayout, this, parent);
    }

    LOG_DEBUG(<< "Create null layout.");
    return nullptr;
}

bool QtApplication::isDarkMode() const
{
    const Qt::ColorScheme scheme = qapplication_.styleHints()->colorScheme();

    if (scheme == Qt::ColorScheme::Dark)
    {
        return true;
    }

    if (scheme == Qt::ColorScheme::Light)
    {
        return false;
    }

    const int lightness =
        qapplication_.palette().color(QPalette::Window).lightness();

    return lightness < 128;
}

void QtApplication::updateTheme()
{
    sidebar_->setDarkMode(isDarkMode());
}

std::string QtApplication::getOpenFileName(const std::string &dialogTitle,
                                           const std::string &filter)
{
    const QString filePath =
        QFileDialog::getOpenFileName(&mainWindow_,
                                     QString::fromStdString(dialogTitle),
                                     QString(),
                                     QString::fromStdString(filter));

    return filePath.toStdString();
}
