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
    @file Explorer.cpp
*/

#include <Explorer.hpp>

Explorer::Explorer(QWidget *parent) : QTreeWidget(parent)
{
}

Explorer::~Explorer()
{
}

void Explorer::updateProject(const Project &project)
{
    clear();

    // Header
    setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("File name") << tr("Timestamp") << tr("Path");
    setHeaderLabels(labels);

    // Content
    QList<QTreeWidgetItem *> items;
    for (size_t i = 0; i < project.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        const ProjectFile &file = project.getSnapshot(i);
        item->setText(COLUMN_FILE_NAME, QString::fromStdString(file.fileName));
        item->setText(COLUMN_TIMESTAMP, QString::fromStdString(file.timestamp));
        item->setText(COLUMN_PATH, QString::fromStdString(file.path));
    }

    setColumnHidden(COLUMN_PATH, true);

    setSortingEnabled(true);
    sortItems(COLUMN_FILE_NAME, Qt::AscendingOrder);
}
