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
    @file Forest3dLayers.hpp
*/

#ifndef FOREST_3D_LAYERS_HPP
#define FOREST_3D_LAYERS_HPP

#include <Editor.hpp>
#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

/** Forest 3d Layers. */
class Forest3dLayers : public QWidget
{
    Q_OBJECT

public:
    /** Forest 3d Layers Column. */
    enum Column
    {
        COLUMN_ID,
        COLUMN_CHECKED,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    explicit Forest3dLayers(QWidget *parent = nullptr);
    ~Forest3dLayers();

    void updateEditor(const Editor &editor);

public slots:
    void itemChanged(QTreeWidgetItem *item, int column);

signals:
    void itemChangedCheckState(size_t id, bool checked);

protected:
    QTreeWidget *layers_;
};

#endif /* FOREST_3D_LAYERS_HPP */
