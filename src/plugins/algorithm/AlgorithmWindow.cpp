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

#include <AlgorithmPluginInterface.hpp>
#include <AlgorithmTabWidget.hpp>
#include <AlgorithmWidgetInterface.hpp>
#include <AlgorithmWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QPluginLoader>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "AlgorithmWindow"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

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
    LOG_DEBUG(<< "Create algorithm window.");

    // Load algorithm plugins.
    loadPlugins();

    // Create menu for algorithm plugins.
    menu_ = new AlgorithmTabWidget(mainWindow_);
    for (size_t i = 0; i < widgets_.size(); i++)
    {
        menu_->addItem(widgets_[i]);
        connect(widgets_[i],
                SIGNAL(signalParametersChanged()),
                this,
                SLOT(slotParametersChanged()));
    }

    // Create progress bar.
    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);

    QHBoxLayout *progressBarLayout = new QHBoxLayout;
    progressBarLayout->addWidget(progressBar_);
    progressBarLayout->setContentsMargins(0, 0, 0, 0);

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
    dialogLayout->addSpacing(5);
    dialogLayout->addLayout(progressBarLayout);
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
    LOG_DEBUG(<< "Destroy algorithm window.");
    thread_.stop();
}

void AlgorithmWindow::slotAccept()
{
    LOG_DEBUG(<< "Accept.");
    close();
    setResult(QDialog::Accepted);
}

void AlgorithmWindow::slotReject()
{
    LOG_DEBUG(<< "Reject.");
    close();
    setResult(QDialog::Rejected);
}

void AlgorithmWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG(<< "Show algorithm window.");
    QDialog::showEvent(event);
    mainWindow_->suspendThreads();
    resumeThreads(nullptr);
}

void AlgorithmWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG(<< "Close algorithm window.");
    suspendThreads();
    mainWindow_->resumeThreads();
    QDialog::closeEvent(event);
}

void AlgorithmWindow::slotParametersChanged()
{
    LOG_DEBUG(<< "Plugin widget parameters have been changed.");
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
    LOG_DEBUG(<< "In worker thread: notify gui thread through queued signal."
              << " Parameter finished <" << finished << ">.");
    emit signalThread(finished, thread_.progressPercent());
}

void AlgorithmWindow::slotThread(bool finished, int progressPercent)
{
    LOG_DEBUG(<< "In gui thread: update progress."
              << " Parameters finished <" << finished << "> progress <"
              << progressPercent << ">.");

    setProgressBarPercent(progressPercent);

    if (finished)
    {
        LOG_DEBUG(<< "Thread finished.");
        thread_.updateData();
    }
}

void AlgorithmWindow::suspendThreads()
{
    LOG_DEBUG(<< "In gui thread: cancel task in worker thread.");
    thread_.cancel();
    setProgressBarPercent(0);
}

void AlgorithmWindow::resumeThreads(AlgorithmWidgetInterface *algorithm)
{
    LOG_DEBUG(<< "In gui thread: start new task in worker thread.");
    thread_.cancel();
    setProgressBarPercent(0);
    thread_.restart(algorithm);
}

void AlgorithmWindow::loadPlugins()
{
    LOG_DEBUG(<< "Get list of files in the application plugin directory.");
    QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins/");
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    LOG_DEBUG(<< "Try to load <" << entries.count() << "> files as plugins.");

    for (const QString &fileName : entries)
    {
        LOG_DEBUG(<< "Try to load <" << fileName.toStdString()
                  << "> as a plugin.");
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();

        loadPlugin(fileName, plugin);
    }
}

void AlgorithmWindow::loadPlugin(const QString &fileName, QObject *plugin)
{
    if (!plugin)
    {
        return;
    }

    // Use qobject_cast to detect our algorithm plugins by interface type.

    AlgorithmPluginInterface *pluginInterface;
    pluginInterface = qobject_cast<AlgorithmPluginInterface *>(plugin);
    if (pluginInterface)
    {
        pluginInterface->initialize(mainWindow_);
        plugins_.push_back(pluginInterface);
        widgets_.push_back(pluginInterface->widget());
        LOG_DEBUG(<< "Loaded plugin <" << fileName.toStdString()
                  << "> with name <" << pluginInterface->name().toStdString()
                  << ">.");
        (void)fileName;
    }
}

void AlgorithmWindow::setProgressBarPercent(int percent)
{
    progressBar_->setValue(percent);
#if 0
    if (percent > 0 && percent < 100)
    {
        progressBar_->setVisible(true);
    }
    else
    {
        progressBar_->setVisible(false);
    }
#endif
}
