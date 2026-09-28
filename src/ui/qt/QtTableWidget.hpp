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

/** @file QtTableWidget.hpp */

#ifndef QT_TABLE_WIDGET_HPP
#define QT_TABLE_WIDGET_HPP

// Include 3D Forest.
#include <TableWidget.hpp>

// Include Qt.
#include <QTableWidget>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtTableWidget. */
class EXPORT_UI_QT QtTableWidget : public QTableWidget
{
public:
    explicit QtTableWidget(TableWidget *table, QWidget *parent = nullptr);
    ~QtTableWidget() override;

private:
    TableWidget *table_;

    void rebuild();
    void updateCell(int row, int column);
    void updateHeaders();
    void updateSettings();
    void updateHeaderSettings();
    void updateSelection();
    void updateColumnSize(int column);

    void readSelection();
    void readCell(int row, int column);

    static void copyItem(QTableWidgetItem *target,
                         const TableWidgetItem &source);
};

#include <WarningsEnable.hpp>

#endif /* QT_TABLE_WIDGET_HPP */
