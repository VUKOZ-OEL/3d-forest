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

/** @file TreeWidget.hpp */

#ifndef TREE_WIDGET_HPP
#define TREE_WIDGET_HPP

// Include 3D Forest.
#include <Segment.hpp>
class MainWindow;

// Include Qt.
#include <QWidget>
class QTableWidget;

/** Tree Widget. */
class TreeWidget : public QWidget
{
    Q_OBJECT

public:
    TreeWidget(MainWindow *mainWindow);

    void setSegment(const Segment &segment);
    void clear();

protected:
    MainWindow *mainWindow_;
    QTableWidget *table_;
    Segment segment_;

    void setRow(int row, const std::string &key, const std::string &value);
    void setRow(int row, const std::string &key, const Vector3<double> &value);
    void setRow(int row,
                const std::string &key,
                double value,
                const std::string &comment = "");
};

#endif /* TREE_WIDGET_HPP */
