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

/** @file PluginClassify.cpp */

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Log.hpp>
#include <PluginClassify.hpp>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <Time.hpp>

#define PLUGIN_CLASSIFY_NAME "Classify"

PluginClassifyWindow::PluginClassifyWindow(QMainWindow *parent, Editor *editor)
    : WindowDock(parent),
      editor_(editor)
{
    // Widgets apply
    computeButton_ = new QPushButton(tr("&Compute"));
    computeButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(computeButton_, SIGNAL(clicked()), this, SLOT(compute()));

    // Layout
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(computeButton_, 0, Qt::AlignRight);
    hbox->addStretch();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);
    widget_->setFixedHeight(100);
    setWidget(widget_);
}

void PluginClassifyWindow::compute()
{
    editor_->cancelThreads();

    EditorQuery query(editor_);
    query.selectGrid();
    query.exec();

    editor_->restartThreads();
}

PluginClassify::PluginClassify() : window_(nullptr), editor_(nullptr)
{
}

void PluginClassify::initialize(QMainWindow *parent, Editor *editor)
{
    // Do not create GUI when this plugin is loaded
    (void)parent;

    editor_ = editor;
}

void PluginClassify::show(QMainWindow *parent)
{
    // Create GUI only when this plugin is used for the first time
    if (!window_)
    {
        window_ = new PluginClassifyWindow(parent, editor_);
        window_->setWindowTitle(windowTitle());
        window_->setWindowIcon(icon());
        window_->setFloating(true);
        window_->setAllowedAreas(Qt::RightDockWidgetArea);
        parent->addDockWidget(Qt::RightDockWidgetArea, window_);
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

QAction *PluginClassify::toggleViewAction() const
{
    if (window_)
    {
        return window_->toggleViewAction();
    }

    return nullptr;
}

QString PluginClassify::windowTitle() const
{
    return tr(PLUGIN_CLASSIFY_NAME);
}

QString PluginClassify::buttonText() const
{
    return tr(PLUGIN_CLASSIFY_NAME);
}

QString PluginClassify::toolTip() const
{
    return tr("Classify points");
}

QPixmap PluginClassify::icon() const
{
    return QPixmap(":/deviation-ios-50.png");
}
