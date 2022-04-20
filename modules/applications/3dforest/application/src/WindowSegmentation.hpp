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

/** @file WindowSegmentation.hpp */

#ifndef WINDOW_SEGMENTATION_HPP
#define WINDOW_SEGMENTATION_HPP

#include <QWidget>

class Editor;
class QDoubleSpinBox;
class QPushButton;

/** Window Segmentation. */
class WindowSegmentation : public QWidget
{
    Q_OBJECT

public:
    explicit WindowSegmentation(QWidget *parent = nullptr);
    ~WindowSegmentation();

    void setEditor(Editor *editor);

public slots:

signals:
    void finished();

protected:
    Editor *editor_;

    QDoubleSpinBox *radiusSpinBox_;
    QDoubleSpinBox *deltaSpinBox_;
    QPushButton *runButton_;

    void run();
    void segmentation(Editor *editor, double radius, double delta);
};

#endif /* WINDOW_SEGMENTATION_HPP */
