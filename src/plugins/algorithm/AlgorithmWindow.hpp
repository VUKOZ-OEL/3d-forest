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
class AlgorithmMainWidget;
class AlgorithmWidget;
class AlgorithmPluginInterface;

#include <QDialog>
class QPushButton;

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
    void slotThread(bool finished, int progressPercent);

signals:
    void signalThread(bool finished, int progressPercent);

protected slots:
    void slotAccept();
    void slotReject();

private:
    MainWindow *mainWindow_;

    AlgorithmMainWidget *menu_;

    QPushButton *acceptButton_;
    QPushButton *rejectButton_;

    std::vector<AlgorithmPluginInterface *> plugins_;
    std::vector<AlgorithmWidget *> widgets_;

    AlgorithmThread thread_;

    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

    void suspendThreads();
    void resumeThreads(AlgorithmWidget *algorithm);

    void loadPlugins();
    void loadPlugin(QObject *plugin);
};

#endif /* ALGORITHM_WINDOW_HPP */
