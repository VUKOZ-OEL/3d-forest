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

/** @file WindowDock.hpp */

#ifndef WINDOW_DOCK_HPP
#define WINDOW_DOCK_HPP

#include <QDockWidget>
#include <QIcon>

class QLabel;
class QMainWindow;
class QToolButton;

/** Dock Widget. */
class WindowDock : public QDockWidget
{
    Q_OBJECT

public:
    WindowDock(QMainWindow *parent = nullptr);

    QMainWindow *mainWindow() const;

    void setWindowTitle(const QString &);
    void setWindowIcon(const QIcon &);

public slots:
    void windowCollapse();
    void windowClose();

protected:
    QMainWindow *mainWindow_;
    QLabel *windowIcon_;
    QLabel *windowTitle_;
    QToolButton *windowButtonCollapse_;
    QToolButton *windowButtonClose_;
};

#endif /* WINDOW_DOCK_HPP */
