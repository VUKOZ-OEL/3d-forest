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
#include <NavigationItem.hpp>
#include <NavigationTree.hpp>
#include <QtApplication.hpp>
#include <QtIcon.hpp>
#include <QtSidebar.hpp>
#include <Widget.hpp>

// Include Qt.
#include <QAbstractItemView>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "QtSidebar"
#include <Log.hpp>

namespace
{
// Choose an unused role in your sidebar.
constexpr int SidebarIndentRole = Qt::UserRole + 100;
constexpr int SidebarPanelRole = Qt::UserRole + 101;

class SidebarTreeWidget : public QTreeWidget
{
public:
    explicit SidebarTreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent)
    {
    }

protected:
    void drawRow(QPainter *painter,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override
    {
        if (index.data(SidebarPanelRole).toBool())
        {
            // The embedded panel paints itself.
            return;
        }

        QTreeWidget::drawRow(painter, option, index);
    }
};

class SidebarItemDelegate : public QStyledItemDelegate
{
public:
    explicit SidebarItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem adjusted(option);

        if (index.data(SidebarPanelRole).toBool())
        {
            adjusted.state &= ~QStyle::State_MouseOver;
        }
        else
        {
            adjusted.rect.adjust(index.data(SidebarIndentRole).toInt(),
                                 0,
                                 0,
                                 0);
        }

        QStyledItemDelegate::paint(painter, adjusted, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.rwidth() += index.data(SidebarIndentRole).toInt();
        return size;
    }

protected:
    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

#if 0
        if (!option->icon.isNull() &&
            option->decorationPosition == QStyleOptionViewItem::Left)
        {
            option->decorationSize.rwidth() += 4;
            option->decorationAlignment =
                Qt::AlignLeft | Qt::AlignVCenter;
        }
#else
        option->icon = QIcon();
        option->features &= ~QStyleOptionViewItem::HasDecoration;
        option->decorationSize = QSize(0, 0);
#endif
    }
};
} // namespace

QtSidebar::QtSidebar(NavigationTree *navigation,
                     QtApplication *application,
                     QWidget *parent)
    : QWidget(parent),
      navigation_(navigation),
      app_(application)
{
    tree_ = new SidebarTreeWidget(this);
    tree_->setObjectName("sidebarNavigationTree");

    tree_->setHeaderHidden(true);
    tree_->setRootIsDecorated(true);
    tree_->setUniformRowHeights(false);
    tree_->setItemsExpandable(false);
    tree_->setExpandsOnDoubleClick(false);
    tree_->setSelectionMode(QAbstractItemView::NoSelection);
    tree_->setFocusPolicy(Qt::NoFocus);
    tree_->setFrameShape(QFrame::NoFrame);
    tree_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tree_->setIndentation(0);
    tree_->setItemDelegate(new SidebarItemDelegate(tree_));
    tree_->setSortingEnabled(false);

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

    navigation_->itemAdded.connect([this](NavigationItem *item)
                                   { addItem(item); });

    navigation_->itemAboutToBeRemoved.connect([this](NavigationItem *item)
                                              { removeItem(item); });

    // Render anything added before QtSidebar was created.
    for (NavigationItem *item : navigation_->items())
    {
        addItemRecursive(item);
    }
}

QtSidebar::Binding *QtSidebar::findBinding(NavigationItem *item)
{
    const auto it = std::find_if(bindings_.begin(),
                                 bindings_.end(),
                                 [item](const Binding &binding)
                                 { return binding.commonItem == item; });

    return it == bindings_.end() ? nullptr : &(*it);
}

QtSidebar::Binding *QtSidebar::findBinding(QTreeWidgetItem *item)
{
    const auto it = std::find_if(bindings_.begin(),
                                 bindings_.end(),
                                 [item](const Binding &binding)
                                 { return binding.qtItem == item; });

    return it == bindings_.end() ? nullptr : &(*it);
}

QTreeWidgetItem *QtSidebar::findQtParent(NavigationItem *item)
{
    if (!item || !item->parent())
    {
        return nullptr;
    }

    Binding *binding = findBinding(item->parent());

    return binding ? binding->qtItem : nullptr;
}

void QtSidebar::addItem(NavigationItem *item)
{
    if (!item || findBinding(item))
    {
        return;
    }

    QTreeWidgetItem *qtParent = findQtParent(item);

    // Create new item.
    const int index = itemIndex(item);

    if (index < 0)
    {
        return;
    }

    QTreeWidgetItem *qtItem = new QTreeWidgetItem;

    if (qtParent)
    {
        qtParent->insertChild(index, qtItem);
    }
    else
    {
        tree_->insertTopLevelItem(index, qtItem);
    }

    qtItem->setText(0, QString::fromStdString(item->title()));

    // Indent navigation titles, without indenting embedded panels.
    int depth = 0;

    for (QTreeWidgetItem *parent = qtParent; parent; parent = parent->parent())
    {
        ++depth;
    }

    qtItem->setData(0, SidebarIndentRole, depth * 8);

    // Binding.
    Binding binding;
    binding.commonItem = item;
    binding.qtItem = qtItem;

    if (item->type() == NavigationItem::Type::Group)
    {
        QFont font = qtItem->font(0);
        font.setBold(true);
        qtItem->setFont(0, font);

        qtItem->setExpanded(false);

        bindings_.push_back(binding);
        return;
    }

    Action *action = item->action();

    if (!action)
    {
        delete qtItem;
        return;
    }

    qtItem->setIcon(0, toQIcon(action->icon(), themeColors_.isDarkMode()));
    qtItem->setToolTip(0, QString::fromStdString(action->toolTip()));

    if (action->hasPanel())
    {
        QTreeWidgetItem *contentItem = new QTreeWidgetItem(qtItem);

        contentItem->setData(0, SidebarIndentRole, 0);
        contentItem->setData(0, SidebarPanelRole, true);
        contentItem->setFlags(contentItem->flags() & ~Qt::ItemIsSelectable);

        QWidget *qtWidget = app_->createWidget(action->panel(), tree_);

        if (!qtWidget)
        {
            delete qtItem;
            return;
        }

        qtWidget->setProperty("sidebarPanel", true);
        qtWidget->setAttribute(Qt::WA_StyledBackground, true);

        applyPanelTheme(qtWidget, styleSheet_);

        contentItem->setFlags(contentItem->flags() & ~Qt::ItemIsSelectable);

        qtWidget->setContentsMargins(10, 4, 4, 10);

        tree_->setItemWidget(contentItem, 0, qtWidget);
        contentItem->setSizeHint(0, qtWidget->sizeHint());

        qtItem->setExpanded(false);

        binding.contentItem = contentItem;
        binding.qtWidget = qtWidget;
    }

    bindings_.push_back(binding);
}

void QtSidebar::addItemRecursive(NavigationItem *item)
{
    addItem(item);

    for (NavigationItem *child : item->children())
    {
        addItemRecursive(child);
    }
}

void QtSidebar::handleItemClick(QTreeWidgetItem *qtItem)
{
    if (!qtItem)
    {
        return;
    }

    Binding *binding = findBinding(qtItem);

    if (!binding || !binding->commonItem)
    {
        // A panel content row was clicked.
        return;
    }

    NavigationItem *item = binding->commonItem;

    if (item->type() == NavigationItem::Type::Group)
    {
        qtItem->setExpanded(!qtItem->isExpanded());

        return;
    }

    Action *action = item->action();

    if (!action)
    {
        return;
    }

    if (action->hasPanel())
    {
        qtItem->setExpanded(!qtItem->isExpanded());
    }
    else
    {
        action->trigger();
    }
}

void QtSidebar::removeItem(NavigationItem *item)
{
    const auto it = std::find_if(bindings_.begin(),
                                 bindings_.end(),
                                 [item](const Binding &binding)
                                 { return binding.commonItem == item; });

    if (it == bindings_.end())
    {
        return;
    }

    if (it->qtWidget)
    {
        tree_->removeItemWidget(it->contentItem, 0);

        delete it->qtWidget;
    }

    // Also deletes contentItem.
    delete it->qtItem;

    bindings_.erase(it);
}

int QtSidebar::itemIndex(const NavigationItem *item) const
{
    if (!item)
    {
        return -1;
    }

    const std::vector<NavigationItem *> &siblings =
        item->parent() ? item->parent()->children() : navigation_->items();

    const auto it = std::find(siblings.begin(), siblings.end(), item);

    if (it == siblings.end())
    {
        return -1;
    }

    return static_cast<int>(std::distance(siblings.begin(), it));
}

void QtSidebar::setTheme(const QtThemeColors &themeColors)
{
    themeColors_ = themeColors;

    styleSheet_ = themeColors.getStyleSheet();

    setStyleSheet(styleSheet_);

    for (const Binding &binding : bindings_)
    {
        if (binding.qtWidget)
        {
            applyPanelTheme(binding.qtWidget, styleSheet_);
        }
    }

    updateActionIcons(themeColors_.isDarkMode());
}

void QtSidebar::applyPanelTheme(QWidget *widget, const QString &styleSheet)
{
    if (!widget)
    {
        return;
    }

    widget->setStyleSheet(styleSheet);
}

void QtSidebar::updateActionIcons(bool dark)
{
    for (const Binding &binding : bindings_)
    {
        if (!binding.commonItem || !binding.qtItem)
        {
            continue;
        }

        Action *action = binding.commonItem->action();

        if (!action || action->icon().isNull())
        {
            continue;
        }

        binding.qtItem->setIcon(0, toQIcon(action->icon(), dark));
    }
}
