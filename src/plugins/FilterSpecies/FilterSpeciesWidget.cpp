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

/** @file FilterSpeciesWidget.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <FilterSpeciesWidget.hpp>
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <TreeWidgetItem.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterSpeciesWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterSpeciesResources/", name))

FilterSpeciesWidget::FilterSpeciesWidget(Application *app)
    : app_(app)
{
    // Table.
    tree_ = new TreeWidget();
    tree_->setSelectionMode(AbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(AbstractItemView::SelectRows);

    tree_->itemChanged.connect([this](TreeWidgetItem *item, int column)
    {
        slotItemChanged(item, column);
    });

    tree_->itemSelectionChanged.connect([this]()
    {
        slotItemSelectionChanged();
    });

    // Tool bar buttons.
    app_->createToolButton(&showButton_,
                                  tr("Show"),
                                  tr("Make selected species visible"),
                                  THEME_ICON("eye"),
                                  [this](){ slotShow(); });
    showButton_->setEnabled(false);

    app_->createToolButton(&hideButton_,
                                  tr("Hide"),
                                  tr("Hide selected species"),
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
    app_->signalUpdate.connect([this](void *sender, const std::set<Editor::Type> &target)
    {
        slotUpdate(sender, target);
    });

    slotUpdate(nullptr, std::set<Editor::Type>());
}

void FilterSpeciesWidget::slotUpdate(void *sender,
                                     const std::set<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_SPECIES) ||
        target.count(Editor::TYPE_SEGMENT))
    {
        LOG_DEBUG_UPDATE(<< "Input species.");

        setSpeciesList(app_->editor().segments(),
                       app_->editor().speciesList(),
                       app_->editor().speciesFilter());
    }
}

void FilterSpeciesWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output species.");

    app_->suspendThreads();
    app_->editor().setSpeciesList(species_);
    app_->editor().setSpeciesFilter(filter_);
    app_->updateData();
}

void FilterSpeciesWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output species filter.");

    app_->suspendThreads();
    app_->editor().setSpeciesFilter(filter_);
    app_->updateFilter();
}

void FilterSpeciesWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set species filer enabled <" << toString(b) << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterSpeciesWidget::setSpeciesList(const Segments &segments,
                                         const SpeciesList &species,
                                         const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set species n <" << species.size() << ">.");

    block();

    segments_ = segments;
    species_ = species;
    filter_ = filter;

    updateUsedSpecies();

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    std::vector<std::string> labels;
    labels.push_back(tr("Visible"));
    labels.push_back(tr("Id"));
    labels.push_back(tr("Name"));
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < species_.size(); i++)
    {
        if (usedSpeciesIds_.count(species_[i].id) > 0)
        {
            addTreeItem(i);
        }
    }

    // Resize Columns to the minimum space.
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    unblock();
}

void FilterSpeciesWidget::updateUsedSpecies()
{
    usedSpeciesIds_.clear();

    for (size_t i = 0; i < segments_.size(); i++)
    {
        usedSpeciesIds_.insert(segments_[i].speciesId);
    }
}

void FilterSpeciesWidget::slotShow()
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

void FilterSpeciesWidget::slotHide()
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

void FilterSpeciesWidget::slotSelectAll()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(true);
    }

    slotItemSelectionChanged();
}

void FilterSpeciesWidget::slotSelectInvert()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(!item.isSelected());
    }

    slotItemSelectionChanged();
}

void FilterSpeciesWidget::slotSelectNone()
{
    for (auto &item : tree_->items())
    {
        item.setSelected(false);
    }

    slotItemSelectionChanged();
}

void FilterSpeciesWidget::slotItemSelectionChanged()
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

void FilterSpeciesWidget::slotItemChanged(TreeWidgetItem *item, int column)
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

size_t FilterSpeciesWidget::identifier(const TreeWidgetItem *item)
{
    return toSize(item->text(COLUMN_ID));
}

void FilterSpeciesWidget::updateTree()
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

void FilterSpeciesWidget::block()
{
    (void)tree_->blockSignals(true);
    (void)blockSignals(true);
}

void FilterSpeciesWidget::unblock()
{
    (void)blockSignals(false);
    (void)tree_->blockSignals(false);
}

void FilterSpeciesWidget::addTreeItem(size_t index)
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
    const Species &species = species_[index];

    // Id.
    item.setText(COLUMN_ID, toString(species.id));

    // Label.
    item.setText(COLUMN_LABEL, species.latin);

    // Color legend.
    Color color;
    color.setRedF(static_cast<float>(species.color[0]));
    color.setGreenF(static_cast<float>(species.color[1]));
    color.setBlueF(static_cast<float>(species.color[2]));

    Brush brush(color, Ui::SolidPattern);
    item.setBackground(COLUMN_ID, brush);

    tree_->push_back(item);
}
