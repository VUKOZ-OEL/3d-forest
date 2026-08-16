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
#include <string>
#include <vector>

// Include 3D Forest.
class Action;
class Widget;
class QtApplication;

// Include Qt.
#include <QWidget>
class QTreeWidget;
class QTreeWidgetItem;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtSidebar. */
class EXPORT_UI_QT QtSidebar : public QWidget
{
public:
    explicit QtSidebar(QtApplication *application, QWidget *parent = nullptr);

    void addMenuItem(const std::vector<std::string> &path, Action *action);
    void removeMenuItem(Action *action);
    void addPanel(const std::vector<std::string> &path, Widget *widget);
    void removePanel(Widget *widget);

private:
    enum class ItemKind
    {
        Group,
        Action,
        Panel,
        PanelContent
    };

    struct Binding
    {
        ItemKind kind{ItemKind::Group};

        Action *action{nullptr};
        Widget *widget{nullptr};

        QTreeWidgetItem *item{nullptr};
        QTreeWidgetItem *contentItem{nullptr};

        QWidget *qtWidget{nullptr};
    };

    static constexpr int ITEM_KIND_ROLE = Qt::UserRole;

    static void setItemKind(QTreeWidgetItem *item, ItemKind kind);

    static ItemKind itemKind(const QTreeWidgetItem *item);

    QTreeWidgetItem *findChild(QTreeWidgetItem *parent,
                               const QString &text) const;

    QTreeWidgetItem *findOrCreateGroup(QTreeWidgetItem *parent,
                                       const QString &text);

    QTreeWidgetItem *createPathGroups(const std::vector<std::string> &path);

    void removeEmptyGroups(QTreeWidgetItem *item);

    void handleItemClick(QTreeWidgetItem *item);

    QtApplication *application_;

    QTreeWidget *tree_{nullptr};

    std::vector<Binding> bindings_;
};

#include <WarningsEnable.hpp>

#endif /* QT_SIDEBAR_HPP */
