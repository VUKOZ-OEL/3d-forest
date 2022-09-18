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

/** @file ToolTabWidget.hpp */

#ifndef TOOL_TAB_WIDGET_HPP
#define TOOL_TAB_WIDGET_HPP

#include <vector>

#include <ExportGui.hpp>

#include <QWidget>

class QToolButton;
class QVBoxLayout;

/** Tool Tab Widget. */
class EXPORT_GUI ToolTabWidget : public QWidget
{
    Q_OBJECT

public:
    ToolTabWidget();

    void addTab(QWidget *widget, const QIcon &icon, const QString &label);

public slots:
    void slotToolButton();

private:
    QToolBar *toolBar_;
    QVBoxLayout *mainLayout_;
    std::vector<QWidget *> tabList_;
    std::vector<QToolButton *> toolButtonList_;
};

#endif /* TOOL_TAB_WIDGET_HPP */
