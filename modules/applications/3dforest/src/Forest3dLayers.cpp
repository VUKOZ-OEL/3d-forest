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
    @file Forest3dLayers.cpp
*/

#include <Forest3dLayers.hpp>
#include <Project.hpp>
#include <ProjectLayer.hpp>
#include <QTreeWidget>
#include <QVBoxLayout>

Forest3dLayers::Forest3dLayers(QWidget *parent) : QWidget(parent)
{
    // Table
    layers_ = new QTreeWidget();

    connect(layers_,
            &QTreeWidget::itemChanged,
            this,
            &Forest3dLayers::itemChanged);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addWidget(layers_);
    setLayout(mainLayout);
}

Forest3dLayers::~Forest3dLayers()
{
}

void Forest3dLayers::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        // #id is now checked or unchecked
        size_t id = item->text(COLUMN_ID).toULong();
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);
        emit itemChangedCheckState(id, checked);
    }
}

void Forest3dLayers::updateEditor(const Editor &editor)
{
    (void)blockSignals(true);
    layers_->clear();

    // Header
    layers_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Index") << tr("Select") << tr("Label");
    layers_->setHeaderLabels(labels);

    // Content
    const Project &project = editor.project();
    QList<QTreeWidgetItem *> items;
    for (size_t i = 0; i < project.layerSize(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(layers_);
        const ProjectLayer &layer = project.layer(i);

        item->setText(COLUMN_ID, QString::number(i));
        if (layer.visible)
        {
            item->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }
        else
        {
            item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }
        item->setText(COLUMN_LABEL, QString::fromStdString(layer.label));
    }

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        layers_->resizeColumnToContents(i);
    }

    // Sort Content
    layers_->setSortingEnabled(true);
    layers_->sortItems(COLUMN_ID, Qt::AscendingOrder);
    (void)blockSignals(false);
}
