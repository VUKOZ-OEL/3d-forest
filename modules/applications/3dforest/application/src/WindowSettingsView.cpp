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

/** @file WindowSettingsView.cpp */

#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <WindowSettingsView.hpp>

WindowSettingsView::WindowSettingsView(QWidget *parent) : QWidget(parent)
{
    int row;

    // Tab Visualization : color source
    tree_ = new QTreeWidget();

    deselectButton_ = new QPushButton(tr("Disable all"));
    deselectButton_->setToolTip(tr("Disable all sources"));
    connect(deselectButton_, SIGNAL(clicked()), this, SLOT(clearSelection()));

    colorFgButton_ = new QPushButton(tr("Foreground"));
    connect(colorFgButton_, SIGNAL(clicked()), this, SLOT(setColorFg()));

    colorBgButton_ = new QPushButton(tr("Background"));
    connect(colorBgButton_, SIGNAL(clicked()), this, SLOT(setColorBg()));

    QGroupBox *groupBox = new QGroupBox(tr("Color Source"));

    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->addStretch();
    controlLayout->addWidget(deselectButton_);

    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget(colorFgButton_);
    colorLayout->addWidget(colorBgButton_);
    colorLayout->addStretch();

    QVBoxLayout *groupBoxLayout = new QVBoxLayout;
    groupBoxLayout->setContentsMargins(2, 1, 2, 1);
    groupBoxLayout->addWidget(tree_);
    groupBoxLayout->addLayout(controlLayout);

    groupBox->setLayout(groupBoxLayout);

    // Tab Visualization : point size
    pointSizeSlider_ = new QSlider;
    pointSizeSlider_->setMinimum(1);
    pointSizeSlider_->setMaximum(5);
    pointSizeSlider_->setSingleStep(1);
    pointSizeSlider_->setTickInterval(1);
    pointSizeSlider_->setTickPosition(QSlider::TicksAbove);
    pointSizeSlider_->setOrientation(Qt::Horizontal);
    connect(pointSizeSlider_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(setPointSize(int)));

    // Tab Visualization : fog
    fogCheckBox_ = new QCheckBox;
    fogCheckBox_->setChecked(settings_.isFogEnabled());
    fogCheckBox_->setToolTip(tr("Reduce intensity with increasing distance."));
    connect(fogCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(setFogEnabled(int)));

    // Tab Visualization
    QGridLayout *visualizationLayout1 = new QGridLayout;
    row = 0;
    visualizationLayout1->addWidget(groupBox, row, 0, 1, 2);
    row++;
    visualizationLayout1->addWidget(new QLabel(tr("Point Size")), row, 0);
    visualizationLayout1->addWidget(pointSizeSlider_, row, 1);
    row++;
    visualizationLayout1->addWidget(new QLabel(tr("Fog")), row, 0);
    visualizationLayout1->addWidget(fogCheckBox_, row, 1);
    row++;
    visualizationLayout1->addWidget(new QLabel(tr("Color")), row, 0);
    visualizationLayout1->addLayout(colorLayout, row, 1);

    QWidget *visualization = new QWidget;
    QVBoxLayout *visualizationLayout = new QVBoxLayout;
    visualizationLayout->addLayout(visualizationLayout1);
    visualizationLayout->addStretch(1);
    visualization->setLayout(visualizationLayout);

    QWidget *guide = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    guide->setLayout(vbox);

    // Tab Main
    tabWidget_ = new QTabWidget;
    tabWidget_->addTab(visualization, tr("Visual"));
    tabWidget_->addTab(guide, tr("Guide"));

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget_);
    mainLayout->setContentsMargins(1, 1, 1, 1);
    setLayout(mainLayout);

    // Window
    setFixedHeight(290);
}

WindowSettingsView::~WindowSettingsView()
{
}

void WindowSettingsView::clearSelection()
{
    settings_.setColorSourceEnabledAll(false);
    updateTree();
    emit settingsChangedApply();
}

void WindowSettingsView::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        settings_.setColorSourceEnabled(index(item), checked);
        emit settingsChangedApply();
    }
}

size_t WindowSettingsView::index(const QTreeWidgetItem *item)
{
    return item->text(COLUMN_ID).toULong();
}

void WindowSettingsView::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        size_t idx = index(*it);

        if (settings_.isColorSourceEnabled(idx))
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }
        else
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }

        ++it;
    }

    unblock();
}

void WindowSettingsView::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void WindowSettingsView::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(itemChanged(QTreeWidgetItem *, int)));
}

void WindowSettingsView::addItem(size_t i)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    if (settings_.isColorSourceEnabled(i))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(i));

    item->setText(COLUMN_LABEL,
                  QString::fromStdString(settings_.colorSourceString(i)));
    item->setText(COLUMN_OPACITY, "100%");
}

void WindowSettingsView::setColorSource(const EditorSettingsView &settings)
{
    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Enabled") << tr("Id") << tr("Label") << tr("Opacity");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < settings.colorSourceSize(); i++)
    {
        addItem(i);
    }

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    tree_->setColumnHidden(COLUMN_ID, true);
}

void WindowSettingsView::setPointSize(int v)
{
    settings_.setPointSize(static_cast<float>(v));
    emit settingsChanged();
}

void WindowSettingsView::setFogEnabled(int v)
{
    (void)v;
    settings_.setFogEnabled(fogCheckBox_->isChecked());
    emit settingsChanged();
}

void WindowSettingsView::setSettings(const EditorSettingsView &settings)
{
    block();

    settings_ = settings;

    setColorSource(settings_);
    pointSizeSlider_->setValue(static_cast<int>(settings_.pointSize()));
    setColor(colorFgButton_, settings_.pointColor());
    setColor(colorBgButton_, settings_.backgroundColor());

    unblock();
}

void WindowSettingsView::setColorFg()
{
    Vector3<float> rgb = settings_.pointColor();

    if (colorDialog(rgb))
    {
        settings_.setPointColor(rgb);
        setColor(colorFgButton_, rgb);
        emit settingsChangedApply();
    }
}

void WindowSettingsView::setColorBg()
{
    Vector3<float> rgb = settings_.backgroundColor();

    if (colorDialog(rgb))
    {
        settings_.setBackgroundColor(rgb);
        setColor(colorBgButton_, rgb);
        emit settingsChangedApply();
    }
}

bool WindowSettingsView::colorDialog(Vector3<float> &rgb)
{
    QColor color;
    color.setRgbF(rgb[0], rgb[1], rgb[2]);

    QColorDialog dialog(color, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return false;
    }

    color = dialog.selectedColor();
    rgb[0] = static_cast<float>(color.redF());
    rgb[1] = static_cast<float>(color.greenF());
    rgb[2] = static_cast<float>(color.blueF());

    return true;
}

void WindowSettingsView::setColor(QPushButton *button,
                                  const Vector3<float> &rgb)
{
    QColor color;
    color.setRgbF(rgb[0], rgb[1], rgb[2]);

    QPixmap pixmap(25, 25);
    pixmap.fill(color);

    QIcon icon(pixmap);

    button->setIcon(icon);
    button->setIconSize(QSize(10, 10));
}
