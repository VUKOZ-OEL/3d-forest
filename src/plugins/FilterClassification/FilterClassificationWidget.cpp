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

/** @file FilterClassificationWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ColorPalette.hpp>
#include <FilterClassificationWidget.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <ThemeIcon.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <TreeWidgetItem.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterClassificationWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterClassificationResources/", name))

FilterClassificationWidget::FilterClassificationWidget(Application *app)
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
                           tr("Make selected classifications visible"),
                           THEME_ICON("eye"),
                           [this]() { slotShow(); });
    showButton_->setEnabled(false);

    app_->createToolButton(&hideButton_,
                           tr("Hide"),
                           tr("Hide selected classifications"),
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

void FilterClassificationWidget::slotUpdate(const Message &msg)
{
    if (msg.sender() == this)
    {
        return;
    }

    if (msg.empty() || msg.contains(Message::TYPE_CLASSIFICATION))
    {
        LOG_DEBUG_UPDATE(<< "Input classifications.");

        setClassifications(app_->editor().classifications(),
                           app_->editor().classificationsFilter());
    }
}

void FilterClassificationWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output classifications.");

    app_->suspendThreads();
    app_->editor().setClassifications(classifications_);
    app_->editor().setClassificationsFilter(filter_);
    app_->updateData();
}

void FilterClassificationWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output classifications filter.");

    app_->suspendThreads();
    app_->editor().setClassificationsFilter(filter_);
    app_->updateFilter();
}

void FilterClassificationWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set classifications filer enabled <" << toString(b) << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterClassificationWidget::setClassifications(
    const Classifications &classifications,
    const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set classifications n <" << classifications.size() << ">.");

    block();

    classifications_ = classifications;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    std::vector<std::string> labels;
    labels.push_back(tr("Visible"));
    labels.push_back(tr("Class"));
    labels.push_back(tr("Label"));
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < classifications_.size(); i++)
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

void FilterClassificationWidget::slotShow()
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

        filterChanged();
    }
}

void FilterClassificationWidget::slotHide()
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

        filterChanged();
    }
}

void FilterClassificationWidget::slotSelectAll()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(true);
    }

    slotItemSelectionChanged();
}

void FilterClassificationWidget::slotSelectInvert()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(!item.isSelected());
    }

    slotItemSelectionChanged();
}

void FilterClassificationWidget::slotSelectNone()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(false);
    }

    slotItemSelectionChanged();
}

void FilterClassificationWidget::slotItemSelectionChanged()
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

void FilterClassificationWidget::slotItemChanged(TreeWidgetItem *item,
                                                 int column)
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

size_t FilterClassificationWidget::identifier(const TreeWidgetItem *item)
{
    return toSize(item->text(COLUMN_ID));
}

void FilterClassificationWidget::updateTree()
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

void FilterClassificationWidget::block()
{
    (void)tree_->blockSignals(true);
    (void)blockSignals(true);
}

void FilterClassificationWidget::unblock()
{
    (void)blockSignals(false);
    (void)tree_->blockSignals(false);
}

void FilterClassificationWidget::addTreeItem(size_t index)
{
    TreeWidgetItem item;

    if (filter_.enabled(index))
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Checked);
    }
    else
    {
        item.setCheckState(COLUMN_CHECKED, Ui::Unchecked);
    }

    item.setText(COLUMN_ID, toString(index));

    item.setText(COLUMN_LABEL, classifications_.label(index));

    // Color legend.
    if (index < ColorPalette::Classification.size())
    {
        const Vector3<double> &rgb = ColorPalette::Classification[index];

        Color color;
        color.setRedF(static_cast<float>(rgb[0]));
        color.setGreenF(static_cast<float>(rgb[1]));
        color.setBlueF(static_cast<float>(rgb[2]));

        Brush brush(color, Ui::SolidPattern);
        item.setBackground(COLUMN_ID, brush);
        // brush.setColor(Color(0, 0, 0));
        // item.setForeground(COLUMN_ID, brush);
    }

    tree_->push_back(item);
}
