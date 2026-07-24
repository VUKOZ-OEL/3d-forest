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

/** @file FilterFilesWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <FilterFilesWidget.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <PushButton.hpp>
#include <ThemeIcon.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <TreeWidgetItem.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterFilesWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterFilesResources/", name))

FilterFilesWidget::FilterFilesWidget(Application *app) : app_(app)
{
    // Table.
    tree_ = new TreeWidget();
    tree_->setSelectionMode(AbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(AbstractItemView::SelectRows);

    tree_->itemChanged.connect([this](TreeWidgetItem *item, int column)
                               { slotItemChanged(item, column); });

    tree_->itemSelectionChanged.connect([this]()
                                        { slotItemSelectionChanged(); });

    // Tool bar buttons.
    app_->createToolButton(&addButton_,
                           tr("Add"),
                           tr("Add new data set"),
                           THEME_ICON("add"),
                           [this]() { slotAdd(); });

    app_->createToolButton(&deleteButton_,
                           tr("Remove"),
                           tr("Remove selected data set"),
                           THEME_ICON("remove"),
                           [this]() { slotDelete(); });
    deleteButton_->setEnabled(false);

    app_->createToolButton(&showButton_,
                           tr("Show"),
                           tr("Make selected data sets visible"),
                           THEME_ICON("eye"),
                           [this]() { slotShow(); });
    showButton_->setEnabled(false);

    app_->createToolButton(&hideButton_,
                           tr("Hide"),
                           tr("Hide selected data sets"),
                           THEME_ICON("hide"),
                           [this]() { slotHide(); });
    hideButton_->setEnabled(false);

    app_->createToolButton(&selectAllButton_,
                           tr("Select all"),
                           tr("Select all"),
                           THEME_ICON("select-all"),
                           [this]() { slotSelectAll(); });

    app_->createToolButton(&selectInvertButton_,
                           tr("Invert"),
                           tr("Invert selection"),
                           THEME_ICON("select-invert"),
                           [this]() { slotSelectInvert(); });

    app_->createToolButton(&selectNoneButton_,
                           tr("Select none"),
                           tr("Select none"),
                           THEME_ICON("select-none"),
                           [this]() { slotSelectNone(); });

    // Tool bar.
    ToolBar *toolBar = new ToolBar;
    toolBar->addWidget(addButton_);
    toolBar->addWidget(deleteButton_);
    toolBar->addWidget(showButton_);
    toolBar->addWidget(hideButton_);
    toolBar->addSeparator();
    toolBar->addWidget(selectAllButton_);
    toolBar->addWidget(selectInvertButton_);
    toolBar->addWidget(selectNoneButton_);
    toolBar->setIconSize(Size(Application::ICON_SIZE, Application::ICON_SIZE));

    // Layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tree_);

    setLayout(mainLayout);

    // Data.
    updatesEnabled_ = true;
    app_->signalUpdate.connect([this](const Message &msg) { slotUpdate(msg); });

    slotUpdate({});
}

void FilterFilesWidget::slotUpdate(const Message &msg)
{
    if (msg.sender() == this)
    {
        return;
    }

    if (msg.empty() || msg.contains(Message::TYPE_DATA_SET))
    {
        LOG_DEBUG_UPDATE(<< "Input datasets.");

        setDatasets(app_->editor().datasets(), app_->editor().datasetsFilter());
    }
}

void FilterFilesWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output datasets.");

    app_->suspendThreads();
    app_->editor().setDatasets(datasets_);
    app_->editor().setDatasetsFilter(filter_);
    app_->updateData();
    app_->update(this, Message::TYPE_DATA_SET, Page::STATE_READ);
}

void FilterFilesWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output datasets filter <" << filter_.enabled()
                     << ">.");

    app_->suspendThreads();
    app_->editor().setDatasetsFilter(filter_);
    app_->updateFilter();
}

void FilterFilesWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set datasets filer enabled <" << toString(b) << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterFilesWidget::setDatasets(const Datasets &datasets,
                                    const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set datasets n <" << datasets.size() << ">.");

    block();

    datasets_ = datasets;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    std::vector<std::string> labels;
    labels.push_back(tr("Visible"));
    labels.push_back(tr("Id"));
    labels.push_back(tr("Label"));
    labels.push_back(tr("Date"));
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < datasets_.size(); i++)
    {
        addTreeItem(i);
    }

    // Resize Columns to the minimum space.
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    // Sort Content.
    tree_->setSortingEnabled(true);
    tree_->sortItems(COLUMN_ID, Ui::AscendingOrder);

    unblock();
}

void FilterFilesWidget::slotAdd()
{
    app_->importFile();
}

void FilterFilesWidget::slotDelete()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        slotSelectNone();

        for (auto &item : items)
        {
            datasets_.erase(index(item));
            filter_.erase(identifier(item));

            delete item;
        }

        dataChanged();
    }
}

void FilterFilesWidget::slotShow()
{
    LOG_DEBUG(<< "Show.");
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        updatesEnabled_ = false;
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Ui::Checked);
            filter_.setEnabled(index(item), true);
        }
        updatesEnabled_ = true;

        filterChanged();
    }
}

void FilterFilesWidget::slotHide()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        updatesEnabled_ = false;
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Ui::Unchecked);
            filter_.setEnabled(index(item), false);
        }
        updatesEnabled_ = true;

        filterChanged();
    }
}

void FilterFilesWidget::slotSelectAll()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(true);
    }

    slotItemSelectionChanged();
}

void FilterFilesWidget::slotSelectInvert()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(!item.isSelected());
    }

    slotItemSelectionChanged();
}

void FilterFilesWidget::slotSelectNone()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(false);
    }

    slotItemSelectionChanged();
}

void FilterFilesWidget::slotItemSelectionChanged()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        deleteButton_->setEnabled(true);
        showButton_->setEnabled(true);
        hideButton_->setEnabled(true);
    }
    else
    {
        deleteButton_->setEnabled(false);
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
    }
}

void FilterFilesWidget::slotItemChanged(TreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = identifier(item);
        bool checked = (item->checkState(COLUMN_CHECKED) == Ui::Checked);

        filter_.setEnabled(id, checked);

        if (updatesEnabled_)
        {
            filterChanged();
        }
    }
}

size_t FilterFilesWidget::identifier(const TreeWidgetItem *item)
{
    return toSize(item->text(COLUMN_ID));
}

size_t FilterFilesWidget::index(const TreeWidgetItem *item)
{
    return datasets_.index(identifier(item));
}

void FilterFilesWidget::updateTree()
{
    block();

    for (auto &item : tree_->items())
    {
        size_t id = identifier(&item);

        if (filter_.enabled(id))
        {
            item.setCheckState(COLUMN_CHECKED, Ui::Checked);
        }
        else
        {
            item.setCheckState(COLUMN_CHECKED, Ui::Unchecked);
        }
    }

    unblock();
}

void FilterFilesWidget::block()
{
    (void)tree_->blockSignals(true);
    (void)blockSignals(true);
}

void FilterFilesWidget::unblock()
{
    (void)blockSignals(false);
    (void)tree_->blockSignals(false);
}

void FilterFilesWidget::addTreeItem(size_t index)
{
    TreeWidgetItem item;

    size_t id = datasets_.id(index);

    if (filter_.enabled(id))
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Checked);
    }
    else
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Unchecked);
    }

    item.setText(COLUMN_ID, toString(id));
    item.setText(COLUMN_LABEL, datasets_.label(index));
    item.setText(COLUMN_DATE_CREATED, datasets_.dateCreated(index));

    // Color legend.
    const Vector3<double> &rgb = datasets_.color(index);

    Color color;
    color.setRedF(static_cast<float>(rgb[0]));
    color.setGreenF(static_cast<float>(rgb[1]));
    color.setBlueF(static_cast<float>(rgb[2]));

    Brush brush(color, Ui::SolidPattern);
    item.setBackground(COLUMN_ID, brush);

    tree_->push_back(item);
}
