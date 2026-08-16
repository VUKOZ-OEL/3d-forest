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

/** @file QtSidebar.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <Action.hpp>
#include <QtApplication.hpp>
#include <QtSidebar.hpp>
#include <Widget.hpp>

// Include Qt.
#include <QTreeWidget>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "QtSidebar"
#include <Log.hpp>

QtSidebar::QtSidebar(QtApplication *application, QWidget *parent)
    : QWidget(parent),
      application_(application),
      tree_(new QTreeWidget(this))
{
    tree_->setHeaderHidden(true);
    tree_->setRootIsDecorated(true);
    tree_->setIndentation(14);
    tree_->setUniformRowHeights(false);

    tree_->setItemsExpandable(false);
    tree_->setExpandsOnDoubleClick(false);

    // tree_->setFocusPolicy(Qt::NoFocus);
    // tree_->setFrameShape(QFrame::NoFrame);
    // tree_->setHorizontalScrollBarPolicy(
    //     Qt::ScrollBarAlwaysOff);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    // layout->setContentsMargins(6, 8, 6, 8);
    layout->setSpacing(0);
    layout->addWidget(tree_);

    QObject::connect(tree_,
                     &QTreeWidget::itemClicked,
                     this,
                     [this](QTreeWidgetItem *item, int)
                     { handleItemClick(item); });
}

void QtSidebar::setItemKind(QTreeWidgetItem *item, ItemKind kind)
{
    item->setData(0, ITEM_KIND_ROLE, static_cast<int>(kind));
}

QtSidebar::ItemKind QtSidebar::itemKind(const QTreeWidgetItem *item)
{
    return static_cast<ItemKind>(item->data(0, ITEM_KIND_ROLE).toInt());
}

QTreeWidgetItem *QtSidebar::findChild(QTreeWidgetItem *parent,
                                      const QString &text) const
{
    const int count =
        parent ? parent->childCount() : tree_->topLevelItemCount();

    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem *item =
            parent ? parent->child(i) : tree_->topLevelItem(i);

        if (item->text(0) == text && itemKind(item) == ItemKind::Group)
        {
            return item;
        }
    }

    return nullptr;
}

QTreeWidgetItem *QtSidebar::findOrCreateGroup(QTreeWidgetItem *parent,
                                              const QString &text)
{
    if (QTreeWidgetItem *existing = findChild(parent, text))
    {
        return existing;
    }

    QTreeWidgetItem *item = nullptr;

    if (parent)
    {
        item = new QTreeWidgetItem(parent);
    }
    else
    {
        item = new QTreeWidgetItem(tree_);
    }

    item->setText(0, text);
    setItemKind(item, ItemKind::Group);

    return item;
}

QTreeWidgetItem *QtSidebar::createPathGroups(
    const std::vector<std::string> &path)
{
    QTreeWidgetItem *parent = nullptr;

    for (std::size_t i = 0; i + 1 < path.size(); ++i)
    {
        parent = findOrCreateGroup(parent, QString::fromStdString(path[i]));
    }

    return parent;
}

void QtSidebar::removeEmptyGroups(QTreeWidgetItem *item)
{
    while (item && itemKind(item) == ItemKind::Group && item->childCount() == 0)
    {
        QTreeWidgetItem *parent = item->parent();

        delete item;
        item = parent;
    }
}

void QtSidebar::handleItemClick(QTreeWidgetItem *item)
{
    if (!item)
    {
        return;
    }

    const ItemKind kind = itemKind(item);

    switch (kind)
    {
        case ItemKind::Group:
        case ItemKind::Panel:
        {
            item->setExpanded(!item->isExpanded());

            // updateExpandableTitle(item);

            // An expandable section is not an active selection.
            tree_->clearSelection();
            tree_->setCurrentItem(nullptr);

            break;
        }

        case ItemKind::Action:
        {
            const auto it =
                std::find_if(bindings_.begin(),
                             bindings_.end(),
                             [item](const Binding &binding) {
                                 return binding.kind == ItemKind::Action &&
                                        binding.item == item;
                             });

            if (it != bindings_.end() && it->action)
            {
                it->action->trigger();
            }

            // Remove this if actions represent persistent pages.
            tree_->clearSelection();
            tree_->setCurrentItem(nullptr);

            break;
        }

        case ItemKind::PanelContent:
            break;
    }
}

void QtSidebar::addMenuItem(const std::vector<std::string> &path,
                            Action *action)
{
    if (path.empty() || !action)
    {
        return;
    }

    const auto existing = std::find_if(
        bindings_.begin(),
        bindings_.end(),
        [action](const Binding &binding) {
            return binding.kind == ItemKind::Action && binding.action == action;
        });

    if (existing != bindings_.end())
    {
        return;
    }

    QTreeWidgetItem *parent = createPathGroups(path);

    QTreeWidgetItem *item =
        parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree_);

    item->setText(0, QString::fromStdString(path.back()));

    setItemKind(item, ItemKind::Action);

    Binding binding;
    binding.kind = ItemKind::Action;
    binding.action = action;
    binding.item = item;

    bindings_.push_back(binding);
}

void QtSidebar::removeMenuItem(Action *action)
{
    const auto it = std::find_if(bindings_.begin(),
                                 bindings_.end(),
                                 [action](const Binding &binding) {
                                     return binding.kind == ItemKind::Action &&
                                            binding.action == action;
                                 });

    if (it == bindings_.end())
    {
        return;
    }

    QTreeWidgetItem *parent = it->item->parent();

    // Deleting the item automatically removes it from
    // its parent or from tree_ when it is top-level.
    delete it->item;

    bindings_.erase(it);

    // Remove parent menu groups that no longer contain anything.
    removeEmptyGroups(parent);
}

void QtSidebar::addPanel(const std::vector<std::string> &path, Widget *widget)
{
    if (path.empty() || !widget)
    {
        return;
    }

    const auto existing = std::find_if(
        bindings_.begin(),
        bindings_.end(),
        [widget](const Binding &binding) {
            return binding.kind == ItemKind::Panel && binding.widget == widget;
        });

    if (existing != bindings_.end())
    {
        return;
    }

    QTreeWidgetItem *parent = createPathGroups(path);

    QTreeWidgetItem *panelItem =
        parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree_);

    panelItem->setText(0, QString::fromStdString(path.back()));

    setItemKind(panelItem, ItemKind::Panel);

    QTreeWidgetItem *contentItem = new QTreeWidgetItem(panelItem);

    setItemKind(contentItem, ItemKind::PanelContent);

    QWidget *qtWidget = application_->createWidget(widget, tree_);

    if (!qtWidget)
    {
        delete panelItem;
        removeEmptyGroups(parent);
        return;
    }

    tree_->setItemWidget(contentItem, 0, qtWidget);

    contentItem->setSizeHint(0, qtWidget->sizeHint());

    panelItem->setExpanded(false);

    Binding binding;
    binding.kind = ItemKind::Panel;
    binding.widget = widget;
    binding.item = panelItem;
    binding.contentItem = contentItem;
    binding.qtWidget = qtWidget;

    bindings_.push_back(binding);
}

void QtSidebar::removePanel(Widget *widget)
{
    const auto it = std::find_if(bindings_.begin(),
                                 bindings_.end(),
                                 [widget](const Binding &binding) {
                                     return binding.kind == ItemKind::Panel &&
                                            binding.widget == widget;
                                 });

    if (it == bindings_.end())
    {
        return;
    }

    QTreeWidgetItem *parent = it->item->parent();

    if (it->qtWidget)
    {
        tree_->removeItemWidget(it->contentItem, 0);

        delete it->qtWidget;
    }

    delete it->item;

    bindings_.erase(it);

    removeEmptyGroups(parent);
}
