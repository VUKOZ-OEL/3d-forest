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

/** @file AlgorithmWindow.hpp */

#ifndef ALGORITHM_WINDOW_HPP
#define ALGORITHM_WINDOW_HPP

#include <AlgorithmThread.hpp>
#include <ThreadCallbackInterface.hpp>
class MainWindow;
class AlgorithmTabWidget;
class AlgorithmWidgetInterface;
class AlgorithmPluginInterface;

#include <QDialog>
class QCheckBox;
class QPushButton;
class QProgressBar;

/** Algorithm Window. */
class AlgorithmWindow : public QDialog, public ThreadCallbackInterface
{
    Q_OBJECT

public:
    AlgorithmWindow(MainWindow *mainWindow);
    virtual ~AlgorithmWindow();

    virtual void threadProgress(bool finished);

public slots:
    void slotParametersChanged();
    void slotThread(bool finished, size_t nTasks, size_t iTask, double percent);

signals:
    void signalThread(bool finished,
                      size_t nTasks,
                      size_t iTask,
                      double percent);

protected slots:
    void autoRunChanged(int index);
    void slotAccept();
    void slotReject();

private:
    MainWindow *mainWindow_;

    AlgorithmTabWidget *menu_;

    QCheckBox *autoStartCheckBox_;
    QPushButton *acceptButton_;
    QPushButton *rejectButton_;

    QProgressBar *progressBarTask_;
    QProgressBar *progressBar_;

    std::vector<AlgorithmPluginInterface *> plugins_;
    std::vector<AlgorithmWidgetInterface *> widgets_;

    AlgorithmThread thread_;

    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

    void suspendThreads();
    void resumeThreads(AlgorithmWidgetInterface *algorithm);

    void loadPlugins();
    void loadPlugin(const QString &fileName, QObject *plugin);

    void setProgressBar(size_t nTasks, size_t iTask, double percent);
    void resetProgressBar();
};

#endif /* ALGORITHM_WINDOW_HPP */
