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

/** @file GuiWindow.hpp */

#ifndef GUI_WINDOW_HPP
#define GUI_WINDOW_HPP

#include <QDockWidget>
#include <QIcon>

class QLabel;
class QMainWindow;
class QToolButton;
class QDragEnterEvent;

/** Gui Window. */
class GuiWindow : public QDockWidget
{
    Q_OBJECT

public:
    GuiWindow(QMainWindow *parent = nullptr);

    QMainWindow *mainWindow() const;

    void setWindowTitle(const QString &);
    void setWindowIcon(const QIcon &);

    void paintEvent(QPaintEvent *e);

public slots:
    void windowDockFloatEvent(bool topLevel);
    void windowCollapse();
    void windowClose();

protected:
    QMainWindow *mainWindow_;
    QLabel *windowIcon_;
    QLabel *windowTitle_;
    QToolButton *windowButtonCollapse_;
    QToolButton *windowButtonClose_;
};

#endif /* GUI_WINDOW_HPP */
