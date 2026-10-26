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
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QSignalBlocker>
#include <QSlider>
#include <QStyledItemDelegate>
#include <QTabBar>
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
constexpr int SidebarExpandedRole = Qt::UserRole + 102;

class SidebarTreeWidget : public QTreeWidget
{
public:
    explicit SidebarTreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent)
    {
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Enter)
        {
            Q_EMIT itemClicked(currentItem(), 0);
            event->accept();
            return;
        }
        QTreeWidget::keyPressEvent(event);
    }

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
        if (index.data(SidebarPanelRole).toBool())
        {
            return;
        }
        const bool dark = parent()->property("darkNavigation").toBool();
        const bool open = option.state & QStyle::State_Open;
        const bool hover = option.state & QStyle::State_MouseOver;
        const QColor border(dark ? "#454d52" : "#d4dadd");
        const QColor text(dark ? "#eeeeee" : "#202020");
        const QColor fill(open    ? (dark ? "#3a4145" : "#e2f2e9")
                          : hover ? (dark ? "#3a4348" : "#e4e9ec")
                                  : (dark ? "#30363a" : "#edf0f2"));
        QRect rect = option.rect.adjusted(index.data(SidebarIndentRole).toInt(),
                                          2,
                                          -1,
                                          open ? 0 : -3);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(border);
        painter->setBrush(fill);
        painter->drawRoundedRect(rect, 3, 3);
        if ((option.state & QStyle::State_HasFocus) &&
            (option.state & QStyle::State_KeyboardFocusChange))
        {
            painter->setPen(QPen(QColor(dark ? "#69bd83" : "#21833f"), 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 2, 2);
        }
        painter->setPen(text);
        QFont font = option.font;
        font.setBold(open);
        painter->setFont(font);
        const QRect label = rect.adjusted(12, 0, -32, 0);
        painter->drawText(label,
                          Qt::AlignVCenter | Qt::AlignLeft,
                          option.fontMetrics.elidedText(index.data().toString(),
                                                        Qt::ElideRight,
                                                        label.width()));
        if (index.model()->rowCount(index) > 0)
        {
            const QPoint center(rect.right() - 18, rect.center().y());
            QPolygon arrow;
            if (open)
                arrow << center + QPoint(-5, -2) << center + QPoint(0, 3)
                      << center + QPoint(5, -2);
            else
                arrow << center + QPoint(-2, -5) << center + QPoint(3, 0)
                      << center + QPoint(-2, 5);
            painter->setPen(QPen(text, 1.5));
            painter->setBrush(Qt::NoBrush);
            painter->drawPolyline(arrow);
        }
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        if (!index.data(SidebarPanelRole).toBool())
            size.setHeight(45);
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
    setObjectName("navigationSidebar");
    setAttribute(Qt::WA_StyledBackground, true);
    tree_ = new SidebarTreeWidget(this);
    tree_->setObjectName("sidebarNavigationTree");

    tree_->setHeaderHidden(true);
    tree_->setRootIsDecorated(true);
    tree_->setUniformRowHeights(false);
    tree_->setItemsExpandable(false);
    tree_->setExpandsOnDoubleClick(false);
    tree_->setSelectionMode(QAbstractItemView::NoSelection);
    tree_->setFocusPolicy(Qt::StrongFocus);
    tree_->setFrameShape(QFrame::NoFrame);
    tree_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tree_->setIndentation(0);
    tree_->setItemDelegate(new SidebarItemDelegate(tree_));
    tree_->setSortingEnabled(false);
    categories_ = new QTabBar(this);
    categories_->setObjectName("navigationCategories");
    categories_->setExpanding(false);
    categories_->setDrawBase(false);
    categories_->setAccessibleName(tr("Tool categories"));
    categoryTitle_ = new QLabel(this);
    categoryTitle_->setObjectName("navigationCategoryTitle");
    search_ = new QLineEdit(this);
    search_->setObjectName("navigationSearch");
    search_->setPlaceholderText(tr("Find tool..."));
    search_->setAccessibleName(tr("Find tool in current category"));
    search_->setClearButtonEnabled(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 0);
    // layout->setContentsMargins(6, 8, 6, 8);
    layout->setSpacing(12);
    layout->addWidget(categoryTitle_);
    layout->addWidget(search_);
    search_->hide();
    layout->addWidget(tree_);
    QObject::connect(categories_,
                     &QTabBar::currentChanged,
                     this,
                     [this](int index) { selectCategory(index); });
    QObject::connect(search_,
                     &QLineEdit::textChanged,
                     this,
                     [this]() { filterItems(); });

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

    qtItem->setData(0, SidebarIndentRole, std::max(0, depth - 1) * 8);

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
        if (!item->parent())
            rebuildCategories();
        filterItems();
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

        qtWidget->setContentsMargins(12, 14, 12, 14);

        tree_->setItemWidget(contentItem, 0, qtWidget);
        contentItem->setSizeHint(0, qtWidget->sizeHint());

        qtItem->setExpanded(false);

        binding.contentItem = contentItem;
        binding.qtWidget = qtWidget;
    }

    bindings_.push_back(binding);
    if (!item->parent())
        rebuildCategories();
    filterItems();
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
        qtItem->setData(0, SidebarExpandedRole, qtItem->isExpanded());

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
        qtItem->setData(0, SidebarExpandedRole, qtItem->isExpanded());
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

    const bool category = !item->parent();
    if (category)
        tree_->setRootIndex(QModelIndex());
    // Also deletes contentItem.
    delete it->qtItem;

    bindings_.erase(it);
    if (category)
        rebuildCategories(item);
}

void QtSidebar::rebuildCategories(NavigationItem *excluded)
{
    const QSignalBlocker blocker(categories_);
    while (categories_->count())
        categories_->removeTab(0);
    int selected = -1;
    for (NavigationItem *item : navigation_->items())
    {
        if (item == excluded || !findBinding(item))
            continue;
        const int index =
            categories_->addTab(QString::fromStdString(item->title()));
        categories_->setTabData(
            index,
            QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(item)));
        if (item == activeCategory_)
            selected = index;
    }
    if (selected < 0 && categories_->count())
        selected = 0;
    categories_->setCurrentIndex(selected);
    selectCategory(selected);
}

void QtSidebar::selectCategory(int index)
{
    activeCategory_ = index < 0
                          ? nullptr
                          : reinterpret_cast<NavigationItem *>(
                                categories_->tabData(index).value<quintptr>());
    Binding *binding = findBinding(activeCategory_);
    categoryTitle_->setText(index < 0 ? QString()
                                      : categories_->tabText(index));
    tree_->setRootIndex(binding && activeCategory_->isGroup()
                            ? tree_->indexFromItem(binding->qtItem)
                            : QModelIndex());
    // A root action is a valid plugin entry too.
    for (const Binding &entry : bindings_)
        if (!entry.commonItem->parent())
            entry.qtItem->setHidden(entry.commonItem != activeCategory_);
    filterItems();
}

void QtSidebar::filterItems()
{
    const QString query = search_->text().trimmed();
    std::function<bool(NavigationItem *, bool)> filter =
        [&](NavigationItem *item, bool ancestorMatches)
    {
        Binding *binding = findBinding(item);
        if (!binding)
            return false;
        const bool matches =
            ancestorMatches || QString::fromStdString(item->title())
                                   .contains(query, Qt::CaseInsensitive);
        bool childMatches = false;
        for (NavigationItem *child : item->children())
            childMatches = filter(child, matches) || childMatches;
        const bool visible = query.isEmpty() || matches || childMatches;
        binding->qtItem->setHidden(!visible);
        binding->qtItem->setExpanded(
            query.isEmpty()
                ? binding->qtItem->data(0, SidebarExpandedRole).toBool()
                : visible);
        return visible;
    };
    if (activeCategory_)
    {
        if (activeCategory_->isGroup())
            for (NavigationItem *item : activeCategory_->children())
                filter(item, false);
        else
            filter(activeCategory_, false);
    }
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
    tree_->setProperty("darkNavigation", themeColors_.isDarkMode());
    tree_->viewport()->update();

    for (const Binding &binding : bindings_)
    {
        if (binding.qtWidget)
        {
            applyPanelTheme(binding.qtWidget, styleSheet_);
            binding.qtWidget->ensurePolished();
            if (binding.qtWidget->layout())
            {
                binding.qtWidget->layout()->invalidate();
                binding.qtWidget->layout()->activate();
            }
            binding.contentItem->setSizeHint(0, binding.qtWidget->sizeHint());
        }
    }

    tree_->doItemsLayout();

    updateActionIcons(themeColors_.isDarkMode());
}

void QtSidebar::applyPanelTheme(QWidget *widget, const QString &styleSheet)
{
    if (!widget)
    {
        return;
    }

    widget->setStyleSheet(styleSheet);
    for (QSlider *slider : widget->findChildren<QSlider *>())
    {
        if (!slider->property("singleValueSlider").toBool())
            continue;
        slider->setProperty("sidebarSliderAccent", themeColors_.highlight);
        slider->setProperty("sidebarSliderGroove", themeColors_.groove);
        slider->setProperty("sidebarSliderHandle", themeColors_.handle);
        slider->setProperty("sidebarSliderBorder", themeColors_.border);
        slider->update();
    }
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
