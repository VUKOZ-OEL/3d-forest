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

/** @file QtSidebar.hpp */

#ifndef QT_SIDEBAR_HPP
#define QT_SIDEBAR_HPP

// Include std.
#include <memory>
#include <string>
#include <vector>

// Include 3D Forest.
#include <QtThemeColors.hpp>
class Action;
class Widget;
class NavigationTree;
class NavigationItem;
class QtApplication;

// Include Qt.
#include <QWidget>
class QTreeWidget;
class QTreeWidgetItem;
class QToolButton;
class QVBoxLayout;
class QLabel;
class QLineEdit;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtSidebar. */
class EXPORT_UI_QT QtSidebar : public QWidget
{
public:
    explicit QtSidebar(NavigationTree *navigation,
                       QtApplication *application,
                       QWidget *parent = nullptr);
    ~QtSidebar() override;

    void setTheme(const QtThemeColors &themeColors);

private:
    struct Binding
    {
        NavigationItem *commonItem{nullptr};

        QTreeWidgetItem *qtItem{nullptr};
        QTreeWidgetItem *contentItem{nullptr};

        QWidget *qtWidget{nullptr};
    };

    void addItem(NavigationItem *item);
    void addItemRecursive(NavigationItem *item);
    void removeItem(NavigationItem *item);

    void handleItemClick(QTreeWidgetItem *qtItem);

    Binding *findBinding(NavigationItem *item);

    Binding *findBinding(QTreeWidgetItem *item);

    QTreeWidgetItem *findQtParent(NavigationItem *item);

    NavigationTree *navigation_;
    QtApplication *app_;
    std::shared_ptr<void> lifetime_{std::make_shared<int>(0)};

    QTreeWidget *tree_{nullptr};
    QWidget *categoryRail_{nullptr};
    QVBoxLayout *categoryLayout_{nullptr};
    std::vector<QToolButton *> categoryButtons_;
    QLabel *categoryTitle_{nullptr};
    QLineEdit *search_{nullptr};
    NavigationItem *activeCategory_{nullptr};
    void rebuildCategories(NavigationItem *excluded = nullptr);
    void selectCategory(NavigationItem *item);
    void filterItems();
    void updateCategoryIcons(bool dark);

    std::vector<Binding> bindings_;

    // Ordering.
    int itemIndex(const NavigationItem *item) const;

    // Theme.
    QtThemeColors themeColors_;
    QString styleSheet_;
    void applyPanelTheme(QWidget *widget, const QString &styleSheet);
    void updateActionIcons(bool dark);
};

#include <WarningsEnable.hpp>

#endif /* QT_SIDEBAR_HPP */
