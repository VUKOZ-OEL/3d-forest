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

/** @file FilterManagementStatusWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ColorPalette.hpp>
#include <Core.hpp>
#include <FilterManagementStatusTreeWidget.hpp>
#include <FilterManagementStatusWidget.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <Splitter.hpp>
#include <ThemeIcon.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <TreeWidgetItem.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterManagementStatusWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterManagementStatusResources/", name))

FilterManagementStatusWidget::FilterManagementStatusWidget(Application *app)
    : app_(app)
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
    app_->createToolButton(&showButton_,
                           tr("Show"),
                           tr("Make selected management status visible"),
                           THEME_ICON("eye"),
                           [this]() { slotShow(); });
    showButton_->setEnabled(false);

    app_->createToolButton(&hideButton_,
                           tr("Hide"),
                           tr("Hide selected management status"),
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
    toolBar->addWidget(showButton_);
    toolBar->addWidget(hideButton_);
    toolBar->addSeparator();
    toolBar->addWidget(selectAllButton_);
    toolBar->addWidget(selectInvertButton_);
    toolBar->addWidget(selectNoneButton_);
    toolBar->setIconSize(Size(Application::ICON_SIZE, Application::ICON_SIZE));

    // Detail.
    treeWidget_ = new FilterManagementStatusTreeWidget(app_);

    // Splitter.
    splitter_ = new Splitter;
    splitter_->addWidget(tree_);
    splitter_->addWidget(treeWidget_);
    splitter_->setOrientation(Ui::Vertical);
    splitter_->setSizes(std::vector<int>({1, 1}));

    // Layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(splitter_);

    setLayout(mainLayout);

    // Data.
    updatesEnabled_ = true;
    app_->signalUpdate.connect(
        [this](void *sender, const std::set<Editor::Type> &target)
        { slotUpdate(sender, target); });

    slotUpdate(nullptr, std::set<Editor::Type>());
}

void FilterManagementStatusWidget::slotUpdate(
    void *sender,
    const std::set<Editor::Type> &target)
{
    if (sender == this || sender == treeWidget_)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_MANAGEMENT_STATUS))
    {
        LOG_DEBUG_UPDATE(<< "Input management status.");
        receivedManagementStatusList();
    }

    if (target.empty() || target.count(Editor::TYPE_SEGMENT))
    {
        LOG_DEBUG_UPDATE(<< "Input segments.");
        receivedSegments();
    }
}

void FilterManagementStatusWidget::sendFilter()
{
    LOG_DEBUG_UPDATE(<< "Output management status filter.");

    app_->suspendThreads();
    app_->editor().setManagementStatusFilter(filter_);
    app_->updateFilter();
}

void FilterManagementStatusWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set management status filer enabled <" << toString(b)
              << ">.");

    filter_.setEnabled(b);
    sendFilter();
}

void FilterManagementStatusWidget::receivedSegments()
{
    bool found = false;
    const Segments &segments = app_->editor().segments();
    for (size_t i = 0; i < segments.size(); i++)
    {
        if (segments[i].selected)
        {
            treeWidget_->setSegment(segments[i]);
            found = true;
            break;
        }
    }
    if (!found)
    {
        treeWidget_->clear();
    }
}

void FilterManagementStatusWidget::receivedManagementStatusList()
{
    LOG_DEBUG(<< "Set management status n <" << managementStatus.size()
              << ">.");

    block();

    managementStatus_ = app_->editor().managementStatusList();
    filter_ = app_->editor().managementStatusFilter();

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    std::vector<std::string> labels;
    labels.push_back(tr("Visible"));
    labels.push_back(tr("Id"));
    labels.push_back(tr("Name"));
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < managementStatus_.size(); i++)
    {
        addTreeItem(i);
    }

    // Resize Columns to the minimum space.
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    unblock();
}

void FilterManagementStatusWidget::slotShow()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        updatesEnabled_ = false;
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Ui::Checked);
        }
        updatesEnabled_ = true;

        sendFilter();
    }
}

void FilterManagementStatusWidget::slotHide()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        updatesEnabled_ = false;
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Ui::Unchecked);
        }
        updatesEnabled_ = true;

        sendFilter();
    }
}

void FilterManagementStatusWidget::slotSelectAll()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(true);
    }

    slotItemSelectionChanged();
}

void FilterManagementStatusWidget::slotSelectInvert()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(!item.isSelected());
    }

    slotItemSelectionChanged();
}

void FilterManagementStatusWidget::slotSelectNone()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(false);
    }

    slotItemSelectionChanged();
}

void FilterManagementStatusWidget::slotItemSelectionChanged()
{
    std::vector<TreeWidgetItem *> items = tree_->selectedItems();

    if (items.size() > 0)
    {
        showButton_->setEnabled(true);
        hideButton_->setEnabled(true);
    }
    else
    {
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
    }
}

void FilterManagementStatusWidget::slotItemChanged(TreeWidgetItem *item,
                                                   int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = identifier(item);
        bool checked = (item->checkState(COLUMN_CHECKED) == Ui::Checked);

        filter_.setEnabled(id, checked);

        if (updatesEnabled_)
        {
            sendFilter();
        }
    }
}

size_t FilterManagementStatusWidget::identifier(const TreeWidgetItem *item)
{
    return toSize(item->text(COLUMN_ID));
}

void FilterManagementStatusWidget::updateTree()
{
    block();

    size_t i = 0;

    for (auto &item : tree_->items())
    {
        if (filter_.enabled(i))
        {
            item.setCheckState(COLUMN_CHECKED, Ui::Checked);
        }
        else
        {
            item.setCheckState(COLUMN_CHECKED, Ui::Unchecked);
        }

        ++i;
    }

    unblock();
}

void FilterManagementStatusWidget::block()
{
    (void)tree_->blockSignals(true);
    (void)blockSignals(true);
}

void FilterManagementStatusWidget::unblock()
{
    (void)blockSignals(false);
    (void)tree_->blockSignals(false);
}

void FilterManagementStatusWidget::addTreeItem(size_t index)
{
    TreeWidgetItem item;

    // Checked.
    if (filter_.enabled(index))
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Checked);
    }
    else
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Unchecked);
    }

    // Data.
    const ManagementStatus &managementStatus = managementStatus_[index];

    // Id.
    item.setText(COLUMN_ID, toString(managementStatus.id));

    // Label.
    // auto label = core().translate(managementStatus.label);
    item.setText(COLUMN_LABEL, managementStatus.label);

    // Color legend.
    Color color;
    color.setRedF(static_cast<float>(managementStatus.color[0]));
    color.setGreenF(static_cast<float>(managementStatus.color[1]));
    color.setBlueF(static_cast<float>(managementStatus.color[2]));

    Brush brush(color, Ui::SolidPattern);
    item.setBackground(COLUMN_ID, brush);

    tree_->push_back(item);
}
