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

#include <QCheckBox>
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
      autoStartCheckBox_(nullptr),
      acceptButton_(nullptr),
      rejectButton_(nullptr),
      progressBarTask_(nullptr),
      progressBar_(nullptr)
{
    LOG_DEBUG(<< "Create.");

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
    progressBarTask_ = new QProgressBar;
    progressBarTask_->setRange(0, 100);

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);

    QVBoxLayout *progressBarLayout = new QVBoxLayout;
    progressBarLayout->addWidget(progressBarTask_);
    progressBarLayout->addWidget(progressBar_);
    progressBarLayout->setContentsMargins(0, 0, 0, 0);

    // Add apply and cancel buttons.
    autoStartCheckBox_ = new QCheckBox(tr("Auto Start"));
    connect(autoStartCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(autoRunChanged(int)));

    acceptButton_ = new QPushButton(tr("Start"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Stop"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));
    rejectButton_->setDisabled(true);

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addWidget(autoStartCheckBox_);
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Create main layout.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(menu_);
    dialogLayout->addStretch();
    dialogLayout->addSpacing(5);
    dialogLayout->addLayout(progressBarLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);

    setLayout(dialogLayout);

    // Create our dialog.
    setWindowTitle(tr("Algorithm"));
    setWindowIcon(ICON("algorithm"));
    setModal(true);

    // Connect the worker thread to gui thread.
    connect(this,
            SIGNAL(signalThread(bool, size_t, size_t, double)),
            this,
            SLOT(slotThread(bool, size_t, size_t, double)),
            Qt::QueuedConnection);

    thread_.setCallback(this);
    thread_.create();
}

AlgorithmWindow::~AlgorithmWindow()
{
    LOG_DEBUG(<< "Destroy.");
    thread_.stop();
}

void AlgorithmWindow::autoRunChanged(int index)
{
    LOG_DEBUG(<< "Auto Run.");
    (void)index;

    acceptButton_->setDisabled(autoStartCheckBox_->isChecked());
}

void AlgorithmWindow::slotAccept()
{
    LOG_DEBUG(<< "Accept.");
    // close();
    // setResult(QDialog::Accepted);

    for (size_t i = 0; i < widgets_.size(); i++)
    {
        if (widgets_[i]->isVisible())
        {
            resumeThreads(widgets_[i]);
            break;
        }
    }
}

void AlgorithmWindow::slotReject()
{
    LOG_DEBUG(<< "Reject.");
    // close();
    // setResult(QDialog::Rejected);

    suspendThreads();
}

void AlgorithmWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG(<< "Show.");
    QDialog::showEvent(event);
    mainWindow_->suspendThreads();
    resumeThreads(nullptr);
}

void AlgorithmWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG(<< "Close.");
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
    size_t nTasks = 0;
    size_t iTask = 0;
    double percent = 100.0;

    thread_.progress(nTasks, iTask, percent);

    LOG_DEBUG(<< "In worker thread: notify gui thread through queued signal."
              << " Parameter finished <" << finished << "> nTasks <" << nTasks
              << "> iTask <" << iTask << "> percent <" << percent << ">.");

    emit signalThread(finished, nTasks, iTask, percent);
}

void AlgorithmWindow::slotThread(bool finished,
                                 size_t nTasks,
                                 size_t iTask,
                                 double percent)
{
    LOG_DEBUG(<< "In gui thread: update progress."
              << " Parameters finished <" << finished << "> nTasks <" << nTasks
              << "> iTask <" << iTask << "> percent <" << percent << ">.");

    setProgressBar(nTasks, iTask, percent);

    if (finished)
    {
        LOG_DEBUG(<< "Thread finished.");
        thread_.updateData();
        rejectButton_->setEnabled(false);
    }
    else
    {
        rejectButton_->setEnabled(true);
    }
}

void AlgorithmWindow::setProgressBar(size_t nTasks,
                                     size_t iTask,
                                     double percent)
{
    int progressBarTaskPercent;
    if (nTasks > 0)
    {
        progressBarTaskPercent = static_cast<int>(
            (static_cast<double>(iTask) / static_cast<double>(nTasks)) * 100.0);
    }
    else
    {
        progressBarTaskPercent = 100;
    }
    LOG_DEBUG(<< "progressBarTaskPercent <" << progressBarTaskPercent << ">.");
    progressBarTask_->setValue(progressBarTaskPercent);

    int progressBarPercent = static_cast<int>(percent);
    LOG_DEBUG(<< "progressBarPercent <" << progressBarPercent << ">.");
    progressBar_->setValue(progressBarPercent);
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

void AlgorithmWindow::resetProgressBar()
{
    progressBarTask_->setValue(0);
    progressBar_->setValue(0);
}

void AlgorithmWindow::suspendThreads()
{
    LOG_DEBUG(<< "In gui thread: cancel task in worker thread.");
    thread_.cancel();
    resetProgressBar();
    rejectButton_->setDisabled(true);
}

void AlgorithmWindow::resumeThreads(AlgorithmWidgetInterface *algorithm)
{
    LOG_DEBUG(<< "In gui thread: start new task in worker thread.");
    thread_.cancel();
    resetProgressBar();
    thread_.restart(algorithm, autoStartCheckBox_->isChecked());
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
