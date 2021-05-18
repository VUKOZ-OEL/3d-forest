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

/**
    @file Forest3dPluginToolExampleWindow.hpp
*/

#ifndef FOREST_3D_PLUGIN_TOOL_EXAMPLE_WINDOW_HPP
#define FOREST_3D_PLUGIN_TOOL_EXAMPLE_WINDOW_HPP

#include <QDialog>

class Forest3dEditor;
class QLabel;
class QPushButton;

/** Forest 3d Plugin Tool Example Window. */
class Forest3dPluginToolExampleWindow : public QDialog
{
    Q_OBJECT

public:
    Forest3dPluginToolExampleWindow(QWidget *parent, Forest3dEditor *editor);
    ~Forest3dPluginToolExampleWindow();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private slots:
    void run();

protected:
    Forest3dEditor *editor_;
    QLabel *label_;
    QPushButton *button_;
};

#endif /* FOREST_3D_PLUGIN_TOOL_EXAMPLE_WINDOW_HPP */
