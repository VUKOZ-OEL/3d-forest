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
#include <QProgressDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#define PLUGIN_DATABASE_STATISTICS_NAME "Statistics"

PluginDatabaseStatisticsWindow::PluginDatabaseStatisticsWindow(QWidget *parent,
                                                               Editor *editor)
    : QDialog(parent),
      editor_(editor)
{
    // Widgets
    textEdit_ = new QTextEdit;

    computeButton_ = new QPushButton(tr("&Compute"));
    connect(computeButton_, SIGNAL(clicked()), this, SLOT(compute()));

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit_);
    layout->addWidget(computeButton_);

    // Window
    setWindowTitle(tr(PLUGIN_DATABASE_STATISTICS_NAME));
    setMinimumWidth(300);
    setMinimumHeight(300);
}

void PluginDatabaseStatisticsWindow::compute()
{
    editor_->cancelThreads();
    editor_->lock();

    std::vector<FileIndex::Selection> tiles;
    editor_->select(tiles);

    int maximum = static_cast<int>(tiles.size());

    QProgressDialog progressDialog(parentWidget());
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, static_cast<int>(maximum));
    progressDialog.setWindowTitle(QObject::tr(PLUGIN_DATABASE_STATISTICS_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setLabelText(tr("Processing..."));
    progressDialog.setMinimumDuration(100);

    uint32_t classificationMaximum = 0;

    for (int i = 0; i < maximum; i++)
    {
        // Update progress
        progressDialog.setValue(i + 1);
        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
        {
            break;
        }

        // Process step i
        size_t idx = static_cast<size_t>(i);
        EditorTile *tile = editor_->tile(tiles[idx].id, tiles[idx].idx);
        if (!tile)
        {
            continue;
        }

        const std::vector<EditorTile::Attributes> &attrib = tile->attrib;
        const std::vector<unsigned int> &indices = tile->indices;

        for (size_t j = 0; j < indices.size(); j++)
        {
            size_t row = indices[j];
            if (attrib[row].classification > classificationMaximum)
            {
                classificationMaximum = attrib[row].classification;
            }
        }
    }
    progressDialog.setValue(progressDialog.maximum());

    editor_->unlock();
    editor_->restartThreads();

    QString msg =
        QString("Classification Maximum = %1").arg(classificationMaximum);
    textEdit_->append(msg);
}

// -----------------------------------------------------------------------------
PluginDatabaseStatistics::PluginDatabaseStatistics()
    : window_(nullptr),
      editor_(nullptr)
{
}

void PluginDatabaseStatistics::initialize(QWidget *parent, Editor *editor)
{
    // Do not create GUI when this plugin is loaded
    (void)parent;

    editor_ = editor;
}

void PluginDatabaseStatistics::show(QWidget *parent)
{
    // Create GUI only when this plugin is used for the first time
    if (!window_)
    {
        window_ = new PluginDatabaseStatisticsWindow(parent, editor_);
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

QString PluginDatabaseStatistics::windowTitle() const
{
    return tr(PLUGIN_DATABASE_STATISTICS_NAME);
}
