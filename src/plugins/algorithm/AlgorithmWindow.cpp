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

/** @file AlgorithmWindow.cpp */

#include <AlgorithmMainWidget.hpp>
#include <AlgorithmPluginInterface.hpp>
#include <AlgorithmWidget.hpp>
#include <AlgorithmWindow.hpp>
#include <Log.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QPluginLoader>
#include <QProgressDialog>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "AlgorithmWindow"
#define ICON(name) (ThemeIcon(":/algorithm/", name))

AlgorithmWindow::AlgorithmWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      menu_(nullptr),
      acceptButton_(nullptr),
      rejectButton_(nullptr),
      plugins_(),
      widgets_(),
      thread_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Called.");

    // Load algorithm plugins.
    loadPlugins();

    // Create menu for algorithm plugins.
    menu_ = new AlgorithmMainWidget(mainWindow_);
    for (size_t i = 0; i < widgets_.size(); i++)
    {
        menu_->addItem(widgets_[i]);
        connect(widgets_[i],
                SIGNAL(signalParametersChanged()),
                this,
                SLOT(slotParametersChanged()));
    }

    // Add apply and cancel buttons.
    acceptButton_ = new QPushButton(tr("Apply"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Create main layout.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(menu_);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Create our dialog.
    setWindowTitle(tr("Algorithm"));
    setWindowIcon(ICON("algorithm"));
    setModal(true);

    // Connect the worker thread to gui thread.
    connect(this,
            SIGNAL(signalThread(bool, int)),
            this,
            SLOT(slotThread(bool, int)),
            Qt::QueuedConnection);

    thread_.setCallback(this);
    thread_.create();
}

AlgorithmWindow::~AlgorithmWindow()
{
    LOG_DEBUG(<< "Called.");
    thread_.stop();
}

void AlgorithmWindow::slotAccept()
{
    LOG_DEBUG(<< "Called.");
    close();
    setResult(QDialog::Accepted);
}

void AlgorithmWindow::slotReject()
{
    LOG_DEBUG(<< "Called.");
    close();
    setResult(QDialog::Rejected);
}

void AlgorithmWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG(<< "Called.");
    QDialog::showEvent(event);
    mainWindow_->suspendThreads();
    resumeThreads(nullptr);
}

void AlgorithmWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG(<< "Called.");
    suspendThreads();
    mainWindow_->resumeThreads();
    QDialog::closeEvent(event);
}

void AlgorithmWindow::slotParametersChanged()
{
    QObject *obj = sender();

    for (size_t i = 0; i < widgets_.size(); i++)
    {
        if (widgets_[i] == obj)
        {
            resumeThreads(widgets_[i]);
            break;
        }
    }
}

void AlgorithmWindow::threadProgress(bool finished)
{
    // In worker thread: notify gui thread through queued connection.
    LOG_DEBUG(<< "Called with parameter finished <" << finished << ">.");
    emit signalThread(finished, thread_.progressPercent());
}

void AlgorithmWindow::slotThread(bool finished, int progressPercent)
{
    // In gui thread: update visualization.
    LOG_DEBUG(<< "Called with parameter finished <" << finished
              << "> progress <" << progressPercent << ">.");

    mainWindow_->setStatusProgressBarPercent(progressPercent);

    if (finished)
    {
        LOG_TRACE_UNKNOWN(<< "Thread finished.");
        mainWindow_->update({Editor::TYPE_LAYER});
    }
}

void AlgorithmWindow::suspendThreads()
{
    // In gui thread: cancel task in worker thread.
    LOG_DEBUG(<< "Called.");
    thread_.cancel();
    mainWindow_->setStatusProgressBarPercent(0);
}

void AlgorithmWindow::resumeThreads(AlgorithmWidget *algorithm)
{
    // In gui thread: start new task in worker thread.
    if (!algorithm)
    {
        return;
    }

    thread_.cancel();
    algorithm->applyParameters();
    thread_.restart(algorithm);
    mainWindow_->setStatusProgressBarPercent(0);
}

void AlgorithmWindow::loadPlugins()
{
    // Process all files in the application "plugin" directory.
    QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins/");
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entries)
    {
        // Try to load each file as a plugin.
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();

        loadPlugin(plugin);
    }
}

void AlgorithmWindow::loadPlugin(QObject *plugin)
{
    if (!plugin)
    {
        return;
    }

    // Detect and register algorithm plugins by using plugin type.

    AlgorithmPluginInterface *pluginInterface;
    pluginInterface = qobject_cast<AlgorithmPluginInterface *>(plugin);
    if (pluginInterface)
    {
        pluginInterface->initialize(mainWindow_);
        plugins_.push_back(pluginInterface);
        widgets_.push_back(pluginInterface->widget());
    }
}
