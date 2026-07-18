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

/** @file FilterTreesWidget.cpp */

// Include 3D Forest.
#include <FilterTreesTreeWidget.hpp>
#include <FilterTreesWidget.hpp>
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <PushButton.hpp>
#include <Splitter.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <TreeWidgetItem.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterTreesWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterTreesResources/", name))

FilterTreesWidget::FilterTreesWidget(Application *app)
    : app_(app)
{
    // Table.
    tree_ = new TreeWidget();
    tree_->setSelectionMode(AbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(AbstractItemView::SelectRows);

    tree_->itemClicked.connect([this](TreeWidgetItem *item, int column)
    {
        slotItemClicked(item, column);
    });

    tree_->itemChanged.connect([this](TreeWidgetItem *item, int column)
    {
        slotItemChanged(item, column);
    });

    tree_->itemSelectionChanged.connect([this]()
    {
        slotItemSelectionChanged();
    });

    // Tool bar buttons.
    app_->createToolButton(&addButton_,
                                  tr("Add"),
                                  tr("Add new segments"),
                                  THEME_ICON("add"),
                                  [this](){ slotAdd(); });
    addButton_->setEnabled(false);

    app_->createToolButton(&deleteButton_,
                                  tr("Remove"),
                                  tr("Remove selected segments"),
                                  THEME_ICON("remove"),
                                  [this](){ slotDelete(); });
    deleteButton_->setEnabled(false);

    app_->createToolButton(&showButton_,
                                  tr("Show"),
                                  tr("Make selected segments visible"),
                                  THEME_ICON("eye"),
                                  [this](){ slotShow(); });
    showButton_->setEnabled(false);

    app_->createToolButton(&hideButton_,
                                  tr("Hide"),
                                  tr("Hide selected segments"),
                                  THEME_ICON("hide"),
                                  [this](){ slotHide(); });
    hideButton_->setEnabled(false);

    app_->createToolButton(&selectAllButton_,
                                  tr("Select all"),
                                  tr("Select all"),
                                  THEME_ICON("select-all"),
                                  [this](){ slotSelectAll(); });

    app_->createToolButton(&selectInvertButton_,
                                  tr("Invert"),
                                  tr("Invert selection"),
                                  THEME_ICON("select-invert"),
                                  [this](){ slotSelectInvert(); });

    app_->createToolButton(&selectNoneButton_,
                                  tr("Select none"),
                                  tr("Select none"),
                                  THEME_ICON("select-none"),
                                  [this](){ slotSelectNone(); });

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

#if defined(FILTER_TREES_SHOW_DETAIL)
    // Segment.
    treeWidget_ = new FilterTreesTreeWidget(app_);

    // Splitter.
    splitter_ = new Splitter;
    splitter_->addWidget(tree_);
    splitter_->addWidget(treeWidget_);
    splitter_->setOrientation(Ui::Vertical);
    splitter_->setSizes(std::vector<int>({1, 1}));
#endif

    // Layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
#if defined(FILTER_TREES_SHOW_DETAIL)
    mainLayout->addWidget(splitter_);
#else
    mainLayout->addWidget(tree_);
#endif

    setLayout(mainLayout);

    // Data.
    updatesEnabled_ = true;
    app_->signalUpdate.connect([this](void *sender, const std::set<Editor::Type> &target)
    {
        slotUpdate(sender, target);
    });

    slotUpdate(nullptr, std::set<Editor::Type>());
}

void FilterTreesWidget::slotUpdate(void *sender,
                                   const std::set<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_SEGMENT) ||
        target.count(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input segments.");

        setSegments(app_->editor().segments(),
                    app_->editor().segmentsFilter());
    }
}

void FilterTreesWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output segments.");

    app_->suspendThreads();
    app_->editor().setSegments(segments_);
    app_->editor().setSegmentsFilter(filter_);
    app_->updateData();
    app_->update(this, {Editor::TYPE_SEGMENT});
}

void FilterTreesWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output segments filter.");

    app_->suspendThreads();
    app_->editor().setSegmentsFilter(filter_);
    app_->updateFilter();
    app_->update(this, {Editor::TYPE_SEGMENT});
}

void FilterTreesWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set segments filer enabled <" << toString(b) << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterTreesWidget::setSegments(const Segments &segments,
                                    const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set segments n <" << segments.size() << ">.");

    block();

    segments_ = segments;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    std::vector<std::string> labels;
    labels.push_back(tr("Visible"));
    labels.push_back(tr("Id"));
    labels.push_back(tr("Label"));
    tree_->setHeaderLabels(labels);

#if defined(FILTER_TREES_SHOW_DETAIL)
    if (segments_.size() > 0)
    {
        treeWidget_->setSegment(segments_[0]);
    }
    else
    {
        treeWidget_->clear();
    }
#endif

    // Content.
    for (size_t i = 0; i < segments_.size(); i++)
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

void FilterTreesWidget::slotAdd()
{
}

void FilterTreesWidget::slotDelete()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        slotSelectNone();

        for (auto &item : items)
        {
            segments_.erase(index(item));
            filter_.erase(identifier(item));

            delete item;
        }

        dataChanged();
    }
}

void FilterTreesWidget::slotShow()
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

void FilterTreesWidget::slotHide()
{
    LOG_DEBUG(<< "Hide.");

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

void FilterTreesWidget::slotSelectAll()
{
    LOG_DEBUG(<< "Select all.");

    for (auto &item : tree_->items())
    {
        item.setSelected(true);
    }

    slotItemSelectionChanged();
}

void FilterTreesWidget::slotSelectInvert()
{
    LOG_DEBUG(<< "Invert selection.");

    for (auto &item : tree_->items())
    {
        item.setSelected(!item.isSelected());
    }

    slotItemSelectionChanged();
}

void FilterTreesWidget::slotSelectNone()
{
    LOG_DEBUG(<< "Select none.");

    for (auto &item : tree_->items())
    {
        item.setSelected(false);
    }

    slotItemSelectionChanged();
}

void FilterTreesWidget::slotItemSelectionChanged()
{
    LOG_DEBUG(<< "Selection changed.");

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

    // Unselect all trees.
    for (size_t i = 0; i < segments_.size(); i++)
    {
        segments_[i].selected = false;
    }

    if (updatesEnabled_)
    {
        dataChanged();
    }
}

void FilterTreesWidget::slotItemClicked(TreeWidgetItem *item, int column)
{
    LOG_DEBUG(<< "Start item clicked in column <" << column << ">.");

    if (!item)
    {
        return;
    }

    size_t id = identifier(item);
    size_t index = segments_.index(id);
    LOG_DEBUG(<< "Item ID <" << id << "> index <" << index << "> column <"
              << column << ">.");

    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Ui::Checked);

        LOG_DEBUG(<< "Set filter ID <" << id << "> enabled <"
                  << toString(checked) << ">.");
        filter_.setEnabled(id, checked);

        if (updatesEnabled_)
        {
            filterChanged();
        }

        return;
    }

#if defined(FILTER_TREES_SHOW_DETAIL)
    treeWidget_->setSegment(segments_[index]);
#endif

    for (size_t i = 0; i < segments_.size(); i++)
    {
        segments_[i].selected = false;
    }
    segments_[index].selected = true;

    if (updatesEnabled_)
    {
        dataChanged();
    }

    LOG_DEBUG(<< "Finished item clicked.");
}

void FilterTreesWidget::slotItemChanged(TreeWidgetItem *item, int column)
{
    if (!item)
    {
        return;
    }

    (void)column;
}

size_t FilterTreesWidget::identifier(const TreeWidgetItem *item)
{
    return toSize(item->text(COLUMN_ID));
}

size_t FilterTreesWidget::index(const TreeWidgetItem *item)
{
    return segments_.index(identifier(item));
}

void FilterTreesWidget::updateTree()
{
    LOG_DEBUG(<< "Update list.");

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

void FilterTreesWidget::block()
{
    (void)tree_->blockSignals(true);
    (void)blockSignals(true);
}

void FilterTreesWidget::unblock()
{
    (void)blockSignals(false);
    (void)tree_->blockSignals(false);
}

void FilterTreesWidget::addTreeItem(size_t index)
{
    LOG_DEBUG(<< "Add tree item to index <" << index << ">.");

    TreeWidgetItem item;

    size_t id = segments_.id(index);

    if (filter_.enabled(id))
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Checked);
    }
    else
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Unchecked);
    }

    item.setText(COLUMN_ID, toString(id));

    item.setText(COLUMN_LABEL, segments_[index].label);

    // Color legend.
    const Vector3<double> &rgb = segments_[index].color;

    Color color;
    color.setRedF(static_cast<float>(rgb[0]));
    color.setGreenF(static_cast<float>(rgb[1]));
    color.setBlueF(static_cast<float>(rgb[2]));

    Brush brush(color, Ui::SolidPattern);
    item.setBackground(COLUMN_ID, brush);

    tree_->push_back(item);
}
