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

/** @file PluginDatabaseStatistics.cpp */

#include <Editor.hpp>
#include <PluginDatabaseStatistics.hpp>
#include <QCoreApplication>
#include <QDebug>
#include <QMainWindow>
#include <QProgressDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#define PLUGIN_DATABASE_STATISTICS_NAME "Statistics"

PluginDatabaseStatisticsWindow::PluginDatabaseStatisticsWindow(
    QMainWindow *parent,
    Editor *editor)
    : WindowDock(parent),
      editor_(editor)
{
    // Widgets
    textEdit_ = new QTextEdit;

    computeButton_ = new QPushButton(tr("&Compute"));
    computeButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(computeButton_, SIGNAL(clicked()), this, SLOT(compute()));

    // Layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(textEdit_);
    layout->addWidget(computeButton_, 0, Qt::AlignRight);

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(layout);
    setWidget(widget_);
}

void PluginDatabaseStatisticsWindow::compute()
{
    // 1. Clear previous result
    computeReset();

    // 2. Collect new result
    editor_->attach();

    std::vector<FileIndex::Selection> selection;
    editor_->select(selection);

    int maximum = static_cast<int>(selection.size());

    QProgressDialog progressDialog(mainWindow());
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(PLUGIN_DATABASE_STATISTICS_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setLabelText(tr("Processing..."));
    progressDialog.setMinimumDuration(100);

    for (int i = 0; i < maximum; i++)
    {
        // Update progress i
        progressDialog.setValue(i + 1);
        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled())
        {
            break;
        }

        // Process step i
        editor_->lock();
        FileIndex::Selection &selected = selection[static_cast<size_t>(i)];
        EditorTile *tile = editor_->tile(selected.id, selected.idx);
        if (tile)
        {
            computeStep(tile);
        }
        editor_->unlock();
    }
    progressDialog.setValue(progressDialog.maximum());

    editor_->detach();

    // 3. Output new result
    computeOutput();
}

void PluginDatabaseStatisticsWindow::computeReset()
{
    numberOfPoints_ = 0;
    classificationPoints_ = 0;
    classificationMaximum_ = 0;
}

void PluginDatabaseStatisticsWindow::computeStep(EditorTile *tile)
{
    const std::vector<EditorTile::Attributes> &attrib = tile->attrib;

    for (size_t j = 0; j < tile->indices.size(); j++)
    {
        numberOfPoints_++;

        size_t row = tile->indices[j];
        if (attrib[row].classification > 0)
        {
            classificationPoints_++;

            if (attrib[row].classification > classificationMaximum_)
            {
                classificationMaximum_ = attrib[row].classification;
            }
        }
    }
}

void PluginDatabaseStatisticsWindow::computeOutput()
{
    textEdit_->clear();

    QString msg;

    msg = QString("Number of points = %1").arg(numberOfPoints_);
    textEdit_->append(msg);

    msg = QString("Classified points = %1").arg(classificationPoints_);
    textEdit_->append(msg);

    msg = QString("Classification Maximum = %1").arg(classificationMaximum_);
    textEdit_->append(msg);
}

// -----------------------------------------------------------------------------
PluginDatabaseStatistics::PluginDatabaseStatistics()
    : window_(nullptr),
      editor_(nullptr)
{
}

void PluginDatabaseStatistics::initialize(QMainWindow *parent, Editor *editor)
{
    // Do not create GUI when this plugin is loaded
    (void)parent;

    editor_ = editor;
}

void PluginDatabaseStatistics::show(QMainWindow *parent)
{
    // Create GUI only when this plugin is used for the first time
    if (!window_)
    {
        window_ = new PluginDatabaseStatisticsWindow(parent, editor_);
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

QAction *PluginDatabaseStatistics::toggleViewAction() const
{
    if (window_)
    {
        return window_->toggleViewAction();
    }

    return nullptr;
}

QString PluginDatabaseStatistics::windowTitle() const
{
    return tr(PLUGIN_DATABASE_STATISTICS_NAME);
}

QString PluginDatabaseStatistics::buttonText() const
{
    return tr(PLUGIN_DATABASE_STATISTICS_NAME);
}

QString PluginDatabaseStatistics::toolTip() const
{
    return tr("Compute database statistics");
}

QIcon PluginDatabaseStatistics::icon() const
{
    return QIcon(":/icons8-graph-40.png");
}
